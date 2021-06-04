//===============================================================================
// Microsoft patterns & practices
// Parallel Programming Guide
//===============================================================================
// Copyright © Microsoft Corporation.  All rights reserved.
// This code released under the terms of the 
// Microsoft patterns & practices license (http://parallelpatterns.codeplex.com/license).
//===============================================================================

#include <random>
#include <algorithm>
#include <vector>
#include <string>
#include <ppl.h>

#include "SampleUtilities.h"
#include "Account.h"
#include "AccountRepository.h"
#include "Trend.h"

using namespace ::Concurrency;
using namespace ::std;

using namespace ::SampleUtilities;
using namespace ::CreditReview;

const size_t g_predictionWindow = 3;

void UpdatePredictionsSequential(AccountRepository& accounts)
{
    for_each(accounts.begin(), accounts.end(), [](AccountRepository::value_type& record)
    {
        Account& account = record.second;
        Trend trend = Fit(account.Balances());
        double prediction = PredictIntercept(trend, (account.Balances().size() + g_predictionWindow)); 
        account.SeqPrediction() = prediction;
        account.SeqWarning() = prediction < account.GetOverdraft();
    });
}

void UpdatePredictionsParallel(AccountRepository& accounts)
{
    parallel_for_each(accounts.begin(), accounts.end(), [](AccountRepository::value_type& record)
    {
        Account& account = record.second;
        Trend trend = Fit(account.Balances());
        double prediction = PredictIntercept(trend, (account.Balances().size() + g_predictionWindow));
        account.ParPrediction() = prediction;
        account.ParWarning() = prediction < account.GetOverdraft();
    });
}

// Usage: CreditReview n, optional n is number of customers, use 100,000+ for meaningful timings

int main(int argc, char* argv[])
{
    printf("Credit Review Sample\n\n");
#if _DEBUG
    printf("For most accurate timing results, use Release build.\n\n");
#endif

    default_random_engine engine;
    engine.seed(42);
    uniform_real_distribution<double> distribution(0.0, 1.0);
    auto random = std::bind(distribution, engine);

    // Defaults for data generation, may override some on command line
    int months = 36;
    int customerCount = 2000000; 

    Trend goodBalance (0.0, 0.0);
    Trend badBalance (-150.0, 0.0);

    const double variation = 100.0;
    const double overdraft = -1000.0; // Default overdraft limit

    // Printed table of results
    const int rows = 8;
    const int cols = 4;

    if (argc > 1)
        customerCount = atoi(argv[1]);
    if (argc > 2)
        months = atoi(argv[2]);
    months = max(4, months);

    int fewCustomers = 10;
    int fewMonths = 3;

    AccountRepository smallAccounts(fewCustomers, fewMonths, overdraft);

    AssignRandomTrends(smallAccounts, goodBalance, badBalance, variation, random);

    UpdatePredictionsSequential(smallAccounts);
    UpdatePredictionsParallel(smallAccounts);

    // Create accounts for timing tests
    AccountRepository accounts(customerCount, months, overdraft);
    AssignRandomTrends(accounts, goodBalance, badBalance, variation, random);

    // Print summary of accounts  
    printf("\n%d customers, %d months in each account\n\n", customerCount, months);

    // Execute sequential and parallel versions, print timings

    TimedRun([&accounts]() { UpdatePredictionsSequential(accounts); }, "Sequential");
    TimedRun([&accounts]() { UpdatePredictionsParallel(accounts); }, "  Parallel");

    // Print a few accounts including predictions and warnings
    PrintAccounts(accounts, rows, months - cols, cols); // print the last few months

    printf("\nRun complete... press enter to finish.");
    getchar();
}
