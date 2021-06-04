//===============================================================================
// Microsoft patterns & practices
// Parallel Programming Guide
//===============================================================================
// Copyright © Microsoft Corporation.  All rights reserved.
// This code released under the terms of the 
// Microsoft patterns & practices license (http://parallelpatterns.codeplex.com/license).
//===============================================================================

#pragma once

#include <vector>

namespace CreditReview
{
    using namespace ::std;

    // One customer's account data: array of monthly balances, also predictions and warnings

    class Account
    {
    public:
        typedef vector<double> BalanceHistory;

    private:
        BalanceHistory m_balances;
        double m_overdraft;
        double m_seqPrediction;
        double m_parPrediction;
        bool m_seqWarning;
        bool m_parWarning;

    public:
        Account(int months, double overdraft) : 
            m_balances(months),
            m_overdraft(overdraft),
            m_seqPrediction(0),
            m_parPrediction(0),
            m_seqWarning(false),
            m_parWarning(false)
        {
        }
          
        // Move constructor

        Account(Account&& other) : 
            m_balances(other.m_balances),
            m_overdraft(other.m_overdraft),
            m_seqPrediction(other.m_seqPrediction),
            m_parPrediction(other.m_parPrediction),
            m_seqWarning(other.m_seqWarning),
            m_parWarning(other.m_parWarning)
        {
        }

        BalanceHistory& Balances()           { return m_balances; }
        double GetOverdraft() const          { return m_overdraft; }
        double& SeqPrediction()              { return m_seqPrediction; }
        double& ParPrediction()              { return m_parPrediction; }
        bool& SeqWarning()                   { return m_seqWarning; }
        bool& ParWarning()                   { return m_parWarning; }

    private:
        // Disable copy construction
        Account(const Account&);
    };

    // Assign balance history to vary randomly around randomly assigned trend

    template <typename Trend, typename Random>
    inline void AssignRandomTrend(Account& account, const Trend& goodBalance, const Trend& badBalance, double variation, Random& random)
    {
        const double rateScale = 100.0;
        const double balanceScale = 100.0;
        const double rateMean = (goodBalance.Slope() + badBalance.Slope()) / 2;
        const double initialBalanceMean = (goodBalance.Intercept() + badBalance.Intercept()) / 2;
        const double rate = rateMean + rateScale * random();
        const double initialBalance = initialBalanceMean + balanceScale * random();
        const Trend trend(rate, initialBalance);

        // Balance history is trend plus noise
        for (size_t i = 0; i < account.Balances().size(); i++) 
        {
            account.Balances()[i] = PredictIntercept(trend, static_cast<double>(i)) + variation * random(); 
        }
    }

    // Print balances for months starting at firstMonth
    inline void PrintBalance(Account& account, size_t firstMonth, size_t months)
    {
        for (size_t month = firstMonth; month < (firstMonth + months); month++)
        {
            if (month < account.Balances().size())
                printf("%*.2f ", 9, account.Balances()[month]);
            else
                printf("       "); // line up columns even if data missing
        }
    }
}
