#include <astakingdapp/astakingdapp.hpp>

using namespace eosio;
using namespace std;

namespace eosio
{
    void astakingdapp::deposit(name depositer, name to, asset quantity, string memo)
    {
        eosio::print("new deposit ", depositer);
        // run simple tests
        if (depositer != get_self())
        {
            // check(to == get_self(), "Invalid opponent.");
            check(quantity.amount > 0, "Betting amount cannot be 0.");
            check(quantity.symbol == staking_token_symbol, "Invalid tokens.");
            // create the game
            // create_game(host, opponent, bet.amount);
            deposits_table _balances(get_self(), get_self().value);
            auto balance_itr = _balances.find(depositer.value);

            if(balance_itr == _balances.end()){
                _balances.emplace(get_self(), [&](auto &new_balance){
                new_balance.depositer = depositer;
                new_balance.stake_amount = quantity.amount;
                });
            }
            else{
                _balances.modify(balance_itr, get_self(), [&](auto &balance_to_modify){
                balance_to_modify.stake_amount += quantity.amount;
                });
            }
        }
    }

    void astakingdapp::withdraw(name withdrawer)
    {
        require_auth(withdrawer);
        deposits_table _balances(get_self(), get_self().value);

        auto itr = _balances.find(withdrawer.value);

        check(itr != _balances.end(), "deposit does not exist");
                
        double unstake_time = get_unstake_time();

        print("unstake time: ", unstake_time);

        double balance = itr->stake_amount;
        asset payout_asset(balance, staking_token_symbol);

        action payout_action(
            permission_level{get_self(), "active"_n},
            "eosio.token"_n,
            "transfer"_n,
            std::make_tuple(get_self(), withdrawer, payout_asset, string(""))
        );

        payout_action.send();
        _balances.erase(itr);
    }

    void astakingdapp::modsettings(uint64_t unstaketime)
    {
        require_auth(get_self());
        
        auto settings_stored = settings_instance.get_or_create(get_self(), default_settings);

        settings_stored.unstaketime = unstaketime;

        settings_instance.set(settings_stored, get_self());
    }

    void astakingdapp::execquery()
    {
        print("Sending query to Provable....");
        provable_query("URL", "json(https://min-api.cryptocompare.com/data/price?fsym=EOS&tsyms=USD).USD",\
           (proofType_Android | proofStorage_IPFS));
    }
    
    void astakingdapp::callback(
          const eosio::checksum256 queryId,
          const std::vector<uint8_t> result,
          const std::vector<uint8_t> proof
    )
    {
        require_auth(provable_cbAddress());
        const std::string result_str = vector_to_string(result);
        print(" Result: ", result_str);
        print(" Proof length: ", proof.size());
    }

    void astakingdapp::deleteall()
    {
        require_auth(get_self());
        deposits_table _dpeosits(get_self(), get_self().value);

        for (auto itr = _dpeosits.begin();
             itr != _dpeosits.end();)
        {
            itr = _dpeosits.erase(itr);
        }
    }

    double astakingdapp::get_unstake_time()
    {
        if(settings_instance.exists())
            return settings_instance.get().unstaketime;
        else
            return 0;
    }

} /// namespace eosio