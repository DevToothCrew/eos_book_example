#include "play.hpp"
namespace eosio {

    void token::create(account_name issuer, asset maximum_supply)
    {
        require_auth(_self);

        auto sym = maximum_supply.symbol;
        eosio_assert(sym.is_valid(), "Invalid symbol name");
        eosio_assert(maximum_supply.is_valid(), "Invalid Supply");
        eosio_assert(maximum_supply.amount > 0, "Max-supply must be positive");

        stats statstable(_self, sym.name());
        auto existing = statstable.find(sym.name());
        eosio_assert(existing == statstable.end(), "Tokenwith symbol already exists");

        statstable.emplace(_self, [&](auto& s){
            s.supply.symbol = maximum_supply.symbol;
            s.max_supply    = maximum_supply;
            s.issuer        = issuer;
        });
    }

    void token::issue(account_name to, asset quantity, string memo)
    {
        auto sym = quantity.symbol;
        eosio_assert(sym.is_valid(), "Invalid symbol name");
        eosio_assert(memo.size() <= 256, "Memo has more than 256 bytes");

        auto sym_name = sym.name();
        stats statstable(_self, sym_name);
        auto existing = statstable.find(sym_name);
        eosio_assert(existing != statstable.end(), "Token with symbol does now exist, Create token before issue");
        const auto& st = *existing;

        require_auth(st.issuer);
        eosio_assert(quantity.is_valid(), "Invalid quantity");
        eosio_assert(quantity.amount > 0, "Must issue positive quantity");

        eosio_assert(quantity.symbol == st.supply.symbol, "Symbol precision mismatch");
        eosio_assert(quantity.amount <= st.max_supply.amount - st.supply.amount, "Quantity exceeds available supply");

        statstable.modify(st, 0, [&](auto& s){
            s.supply += quantity;
        });

        add_balance(st.issuer, quantity, st.issuer);

        if(to != st.issuer){
            SEND_INLINE_ACTION(*this, transfer, {st.issuer, N(active)}, {st.issuer, to, quantity, memo});
        }
    }

    void token::transfer(account_name from, account_name to, asset quantity, string memo)
    {
        eosio_assert(from != to, "Cannot transfer to self");
        require_auth(from);
        eosio_assert(is_account(to), "To account does not exist");
        auto sym = quantity.symbol.name();
        stats statstable(_self, sym);
        const auto& st = statstable.get(sym, "Not exist symbol");

        require_recipient(from);
        require_recipient(to);

        eosio_assert(quantity.is_valid(), "Invalid quantity");
        eosio_assert(quantity.amount > 0, "Must transfer positive quantity");
        eosio_assert(quantity.symbol == st.supply.symbol, "Symbol precision mismatch");
        eosio_assert(memo.size() <= 250, "Memo has more than 256 bytes");

        sub_balance(from, quantity);
        add_balance(to, quantity, from);
    }

    void token::sub_balance(account_name owner, asset value)
    {
        accounts from_acnts(_self, owner);

        const auto& from = from_acnts.get(value.symbol.name(), "No balance object found");
        eosio_assert(from.balance.amount >= value.amount, "Overdrawn balance");

        if(from.balance.amount == value.amount){
            from_acnts.erase(from);
        }
        else{
            from_acnts.modify(from, owner, [&](auto& a){
                a.balance -= value;
            });
        }
    }

    void token::add_balance(account_name owner, asset value, account_name ram_payer)
    {
        accounts to_acnts(_self, owner);
        auto to = to_acnts.find(value.symbol.name());
        if(to == to_acnts.end()){
            to_acnts.emplace( ram_payer, [&](auto& a){
                a.balance = value;
            });
        }
        else{
            to_acnts.modify(to, 0, [&](auto& a){
                a.balance += value;
            });
        }
    }

    void token::win(account_name winner, asset value)
    {
        asset result = (value * 3);

        scores print_result(_self, winner);
        auto existing = print_result.find(winner);
        if(existing == print_result.end()){
            print_result.emplace(winner, [&](auto& s){
                s.user = winner;
                s.text = "Win! You get ";
                s.amount = result;
            });        
        }
        else{
            print_result.modify(existing, 0, [&](auto& s){
                s.text = "Win! You get ";
                s.amount = result;
            });
        }        

        // Add winner's balance
        accounts winner_acnts(_self, winner);
        const auto& to = winner_acnts.get(value.symbol.name(), "No balance object found");
        winner_acnts.modify(to, 0, [&](auto& a){
            print("     * Win! You get ", result);
            a.balance += result;
            print(" Now your balance ", a.balance);
        });

        // Sub issuer's balance
        accounts owner_acnts(_self, _owner);
        const auto& owner = owner_acnts.get(value.symbol.name(), "No balance object found");
        eosio_assert(owner.balance.amount >= result.amount, "error");
        owner_acnts.modify(owner, 0, [&](auto& a){
            a.balance -= result;
        });
    }

    void token::lose( account_name looser, asset value)
    {
        scores print_result(_self, looser);
        auto existing = print_result.find(looser);
        if(existing == print_result.end()){
            print_result.emplace(looser, [&](auto& s){
                s.user = looser;
                s.text = "You lost ";
                s.amount = value;
            });        
        }
        else{
            print_result.modify(existing, 0, [&](auto& s){
                s.text = "You lost ";
                s.amount = value;
            });
        }
        // Sub looser's balance
        accounts looser_acnts(_self, looser);
        const auto& to = looser_acnts.get(value.symbol.name(), "No balance object found");
        looser_acnts.modify(to, 0, [&](auto& a){
            print("     * You Lost ", value);
            a.balance -= value;
            print(" Now your balance ", a.balance);
        });    

        //Add issuers's balance
        accounts owner_acnts(_self, _owner);
        const auto& owner = owner_acnts.get(value.symbol.name(), "No balance object found");
        owner_acnts.modify(owner, 0, [&](auto& a){
            a.balance += value;
        });
    }

    void token::bet(account_name payer, asset quantity, string betType)
    {
        accounts from_acnts(_self, payer);

        const auto& from = from_acnts.get(quantity.symbol.name(), "No balance object found");

        eosio_assert(from.balance.amount >= quantity.amount, "Overdrawn balance");

        print("You has ", from.balance); 

        checksum256 result;
        uint64_t source = tapos_block_num() * tapos_block_prefix();
        sha256((char *)&source, sizeof(source), &result);
        uint64_t* p = reinterpret_cast<uint64_t*>(&result.hash);
        uint64_t randNum = *p % 3;
        if(randNum==0)
        {
        print("     * Result : Rock ");
        }
        if(randNum==1)
        {
        print("     * Result : Paper ");
        }
        if(randNum==2)
        {
        print("     * Result : Scissors ");
        }
        if(betType == "rock"){
            if( 2 == randNum){
                win(payer, quantity);
            }
            else{
                lose(payer, quantity);
            }
        }
        if(betType == "paper"){
            if( 0 == randNum){
                win(payer, quantity);
            }
            else{
                lose(payer, quantity);
            }
        }
        if(betType == "scissors"){
            if( 1 == randNum){
                win(payer, quantity);
                }
            
            else{
                lose(payer, quantity);
            }
        }
        else
        {
            print("Parameter Only 'rock', 'paper', 'scissors'.");
        }
    }
}

EOSIO_ABI( eosio::token, (create)(issue)(transfer)(bet) )