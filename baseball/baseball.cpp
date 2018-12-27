#include "baseball.hpp"
namespace eosio {

    void baseball::start(account_name player)
    {
        print("Baseball game start!!!");
        print("\n create goal number");

        baseball::createnum(player);        
    }

    void  baseball::throwball(account_name player,uint64_t value )
    {
        scores goalnum(_self,player);
        const auto& data = goalnum.get(player,"No Goal Num");
        uint64_t num = data.num;
        uint64_t strike = 0, ball = 0;
        uint64_t goalarray[3];
        uint64_t playervalue[3];

       matchnum(num, goalarray);
       matchnum(value, playervalue);

        for(int i=0; i<3;i++)
        {
            for(int j=0; j<3; j++)
            {
                if(goalarray[i]==playervalue[j]&& i==j)
                {
                    strike++;
                }
                if(goalarray[i]==playervalue[j]&& i!= j)
                {
                    ball++;
                }
            }
        }
    
        if(strike==3)
        {
            print("Homerun!! You Win.\n");
        }
        else
        {
            print("  Strike : ", strike, "   Ball : ", ball);
        }
    }

    void baseball::createnum(account_name player)
    {
        checksum256 result;
        uint64_t source = tapos_block_num() * tapos_block_prefix();
        sha256((char *)&source, sizeof(source), &result);
        uint64_t* p = reinterpret_cast<uint64_t*>(&result.hash);
        uint64_t randNum = (*p % 504);

        scores num(_self, player);
        auto iter=num.find(player);
        if(iter==num.end())
        {
            num.emplace(_self,[&](auto& s){
                s.user = player;
                s.num = num_table[randNum];
            });
        }
        else{
            num.modify(iter,_self,[&](auto& s){
                s.num = num_table[randNum];
            });
        }
    }

    void baseball::matchnum(uint64_t num, uint64_t array[])
    {
        array[0] = num/100;
        array[1] = num/10;
        array[1] = array[1]%10;
        array[2] = num%10;
    }
}

EOSIO_ABI( eosio::baseball, (start)(throwball) )