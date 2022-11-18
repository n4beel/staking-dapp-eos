#pragma once

#include <eosio/asset.hpp>
#include <eosio/eosio.hpp>
#include <eosio/singleton.hpp>
#include <provable/eos_api.hpp>

#include <string>

namespace eosio
{

    using std::string;

    /**
     * @defgroup astakingdapp
     *
     * astakingdapp contract
     *
     * @details astakingdapp contract for staking and unstaking SYS tokens
     * 
     */
    CONTRACT astakingdapp : public contract
    {
    public:
        using contract::contract;

        astakingdapp(
            name receiver,
            name code,
            datastream<const char *> ds) : contract(receiver,
                                                    code,
                                                    ds),
                                           staking_token_symbol("SYS", 4),
                                           settings_instance(receiver, receiver.value){};

        /**
         * Deposit function.
         *
         * @details Allows `depositer` account to deposit `quantity` tokens,
         * @param depositer - the account that deposits tokens,
         * @param to - contract account,
         * @param quantity - tokens deposited by host,
         * @param memo - optional string memo,
         *
         * @pre to account can not be the depositer,
         * @pre quantity needs to be greater than 0,
         * @pre staking asset's symbol needs to be SYS,
         *
         * If validation is successful depositer's balance is updated.
         */
        [[eosio::on_notify("eosio.token::transfer")]] void
        deposit(name depositer, name to, asset quantity, string memo);

        /**
         * Withdraw action.
         *
         * @details Allows `withdrawer` account to withdraw deposits.
         * @param withdrawer - the account that wants to withdraw their funds,
         *
         * @pre action is called by withdrawer,
         * @pre entry for the deposit exists in the deposits table
         *
         * If validation is successful the deposit entry in host's scope gets erased,
         * user's funds are transferred to the via calling inline transfer action.
         */
        ACTION withdraw(name withdrawer);

        /**
         * Update Settings action.
         *
         * @details Allows astakingdapp account to update the settings.
         * @param unstaketime - number of seconds in which the amount can be unstaked,
         *
         * @pre action is called by astakingdapp account,
         *
         * If validation is successful then the settings are updated
         */
        ACTION modsettings(uint64_t unstaketime);

        ACTION execquery();

        ACTION callback(
          const eosio::checksum256 queryId,
          const std::vector<uint8_t> result,
          const std::vector<uint8_t> proof
        );

        ACTION deleteall();

        using withdraw_action = eosio::action_wrapper<"withdraw"_n, &astakingdapp::withdraw>;
        using update_settings_action = eosio::action_wrapper<"modsettings"_n, &astakingdapp::modsettings>;
        using execute_query_action = eosio::action_wrapper<"execquery"_n, &astakingdapp::execquery>;
        using callback_action = eosio::action_wrapper<"callback"_n, &astakingdapp::callback>;
        using delete_action = eosio::action_wrapper<"deleteall"_n, &astakingdapp::deleteall>;

    private:
        const symbol staking_token_symbol;
        double get_unstake_time();

        TABLE deposits
        {
            name depositer;
            int64_t stake_amount;
            auto primary_key() const { return depositer.value; };
        };
        typedef multi_index<"deposits"_n, deposits> deposits_table;

        TABLE settings
        {
            uint64_t unstaketime;
        } default_settings;

        typedef singleton<"settings"_n, settings> settings_singleton;
        settings_singleton settings_instance;
    };

    /** @}*/ // end of @defgroup astakingdapp
} /// namespace eosio

