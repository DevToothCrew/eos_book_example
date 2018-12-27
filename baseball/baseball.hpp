#pragma once

#include <eosiolib/asset.hpp>
#include <eosiolib/eosio.hpp>
#include <eosiolib/print.hpp>
#include <eosiolib/transaction.hpp>
#include <eosiolib/crypto.h>

#include <string>

namespace eosio {
    
    using std::string;

    class baseball : public contract {
        public:
        account_name _owner;
        uint64_t num_table[504];

        baseball(account_name self):contract(self){
            // Init _owner
            _owner = self;
        
            // Init Number Table
            uint64_t temp = 0;
            for(uint64_t i=100 ; i < 1000; i++){
                if( (i/10)%11 != 0 ){
                    if( i/100 != i%10 ){
                        if( (i%100)%11 != 0 ){
                            if( i%10 != 0 ){
                                if( (i%100)/10 != 0 ){
                                    num_table[temp] = i;
                                    temp++;
                                }
                            }
                        }
                    }
                }
            }
        }

        //@abi action 
        void start(account_name payer);
        
        //@abi action
        void throwball(account_name player,uint64_t value);

        void matchnum(uint64_t num, uint64_t array[]);
        void createnum(account_name player);


        private:
            //@abi table scores i64
            struct score {
                account_name user;
                uint64_t num;
                uint64_t primary_key()const { return user; }
            };

            typedef eosio::multi_index<N(scores), score> scores;
    };
}