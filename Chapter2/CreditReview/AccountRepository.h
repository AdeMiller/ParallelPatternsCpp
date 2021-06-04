//===============================================================================
// Microsoft patterns & practices
// Parallel Programming Guide
//===============================================================================
// Copyright © Microsoft Corporation.  All rights reserved.
// This code released under the terms of the 
// Microsoft patterns & practices license (http://parallelpatterns.codeplex.com/license).
//===============================================================================

#pragma once

#include <unordered_map>
#include <algorithm>

#include "account.h"

namespace CreditReview
{
    using namespace ::std;

    typedef int Customer;

    // Repository of customer accounts
    class AccountRepository
    {
    public:
        typedef unordered_map<Customer, Account> map_type;
        typedef map_type::iterator iterator;
        typedef map_type::value_type value_type;

    private:
        // Repository is implemented by an unoredered_map from customer account numbers to account data,
        // an array of monthly balances etc.
        map_type m_accounts;

    public:
        AccountRepository(int customerCount, int months, double overdraft)
        {
            for (Customer customer = 0; customer < customerCount; customer++)
            {
                m_accounts.insert(map_type::value_type(customer,Account(months, overdraft)));
            }
        }

        inline iterator begin() { return m_accounts.begin(); }

        inline iterator end() { return m_accounts.end(); }

        inline iterator find(Customer& c) { return m_accounts.find(c); }

    private:
        // Disable copy constructor
        AccountRepository(const AccountRepository&);
    };

    // Assign every account with monthly balances that fit randomly assigned trend

    template<typename Trend, typename Random>
    inline void AssignRandomTrends(AccountRepository& accounts,  const Trend& goodBalance,  const Trend& badBalance, double variation, Random& random)
    {
        for_each(accounts.begin(), accounts.end(), [&random, goodBalance, badBalance, variation](AccountRepository::value_type& record)
        {
            AssignRandomTrend(record.second, goodBalance, badBalance, variation, random);
        });
    }

    // Print first rows accounts from firstMonth for months, including predictions and warnings

    void PrintAccounts(AccountRepository& accounts, int rows, int firstMonth, int months)
    {
        printf("\nCustomer  Recent balances for month number        Predicted balances & warnings\n         ");
        for (int month = firstMonth; month < (firstMonth + months); month++) 
            printf("%9d ", month); 
        printf(" Sequential  Parallel\n");

        // Print results for first nRows customers
        Customer firstCustomer = 0;
        for (int customer = firstCustomer; customer < (firstCustomer + rows); customer++)
        {
            if (accounts.find(customer) != accounts.end())
            {
                printf("%09d",customer);
                Account& acc = (*accounts.find(customer)).second;
                PrintBalance(acc, firstMonth, months);

                printf("%*.2f %c %*.2f %c\n",
                    9,acc.SeqPrediction(), acc.SeqWarning() ? 'W' : ' ',  // sequential
                    9,acc.ParPrediction(), acc.ParWarning() ? 'W' : ' ');  // parallel
            }
        }
    }
}
