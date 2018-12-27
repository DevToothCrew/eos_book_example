#pragma once

#include <eosiolib/asset.hpp>
#include <eosiolib/eosio.hpp>
#include <eosiolib/print.hpp>

#include <string>

namespace devtooth {
    
    using std::string;
    using namespace eosio;

    class token : public contract {
        public:
        account_name _owner;

        token(account_name self):contract(self){
            _owner = self;
        }

        //@abi action
        void create(account_name issuer, asset maximum_supply);
        //@abi action    
        void issue(account_name to, asset quantity, string memo);
        //@abi action
        void transfer(account_name from, account_name to, asset quantity, string memo);

        private:
            //@abi table accounts i64
            struct account {
                asset balance;

                uint64_t primary_key()const { return balance.symbol.name(); }
            };
            //@abi table stat i64
            struct currencies {
                asset        supply;
                asset        max_supply;
                account_name issuer;

                uint64_t primary_key()const { return supply.symbol.name(); }
            };

            typedef eosio::multi_index<N(accounts), account> accounts;
            typedef eosio::multi_index<N(stat), currencies> stats;

            void sub_balance( account_name owner, asset value );
            void add_balance( account_name owner, asset value, account_name ram_payer );
    };
}
