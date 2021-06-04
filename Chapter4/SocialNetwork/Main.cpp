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
#include <ppl.h>
#include <ppl_extras.h>

#include "SampleUtilities.h"
#include "FriendMultiSet.h"

using namespace ::std;
using namespace ::Concurrency;
using namespace ::Concurrency::samples;
using namespace ::SampleUtilities;

void Print(const FriendOrderedMultiSet& friends)
{
    printf("Subscriber   # friends\n");
    for_each(friends.cbegin(), friends.cend(), [](FriendsSetPair val)
    {
        printf("%10d %5d\n", val.first, val.second);
    });
}

void Print(const SubscriberMap& subscribers, int maxFriends)
{
    printf("%5d", subscribers.size());

    SubscriberMap::const_iterator itr = subscribers.cbegin();
    for (int i = 0; i < min<int>(maxFriends, subscribers.size()); i++)
    {
        printf("%8d", *(itr++));
    }
    printf("\n");
}

bool AssignRandomFriends(SubscriberMap& m_subscribers, int maxSubscribers, int maxFriends, unsigned long seed)
{
    for (SubscriberID i = 0; i < maxSubscribers; i++)
        m_subscribers.insert(SubscriberMapPair(i, make_shared<FriendsSet>()));

    default_random_engine engine;
    engine.seed(seed);
    uniform_int_distribution<int> distribution_subscribers(0, maxSubscribers - 1);
    uniform_int_distribution<int> distribution_maxfriends(0, maxFriends);
    auto random_subscriber = bind(distribution_subscribers, engine);
    auto random_maxfriends = bind(distribution_maxfriends, engine);

    SubscriberMap::iterator itr;

    try
    {
        for(itr = m_subscribers.begin(); itr != m_subscribers.end(); ++itr)
        {
            auto nFriends = random_maxfriends();
            for (int i = 0; i < nFriends; ++i)
            {
                int friendID = random_subscriber();
                if (friendID == itr->first)                                             // Self is never in friends
                    continue;

                itr->second->insert(friendID);                                          // Add new friend, set ensures no duplicates
                SubscriberMap::iterator myFriend = m_subscribers.find(friendID);        // Add this subscriber to new friend's friends
                myFriend->second->insert(itr->first); 
            }
        }
    }
    catch (bad_alloc e)
    {
        return false;
    }
    return true;
}

// Sequential implementation using for_each

FriendOrderedMultiSet PotentialFriendsSequential(const SubscriberMap& subscribers, SubscriberID id, int maxCandidates)
{
    // Map:

    // First get all the subscriber's friends
    FriendsSetPtr friends = subscribers.find(id)->second;
    vector<FriendMultiSetPtr> friendsOfFriends;

    // For each of the subscriber's friends search their friends (friends of friends) for 
    // people the subscriber is not already a friend of
    for_each(friends->cbegin(), friends->cend(), 
        [&subscribers, &friendsOfFriends, id, friends](int friendID) 
    { 
        FriendsSetPtr theirFriends = subscribers.find(friendID)->second;
        FriendsSetPtr friendsOfFriend = make_shared<FriendsSet>();

        // Find all their friends who are not already the subscriber's friends
        set_difference(theirFriends->cbegin(), theirFriends->cend(), 
            friends->begin(), friends->end(), 
            inserter(*friendsOfFriend, friendsOfFriend->end()));

        // Remove the subscriber
        friendsOfFriend->erase(id);

        // Store the new set of possible friends
        friendsOfFriends.push_back(FriendMultiSetPtr(new FriendMultiSet(friendsOfFriend)));
    });

    // Reduce:

    // For each set of possible friends merge the sets and maintain a count of the occurrences
    FriendMultiSet candidates;
    for_each(friendsOfFriends.cbegin(), friendsOfFriends.cend(), [&candidates](FriendMultiSetPtr set) {
        candidates.Union(set);
    });

    // Postprocess:

    // Sort the candidates in order of their frequency of occurrence
    return candidates.MostNumerous(maxCandidates);
}

// Sequential implementation using transform and for_each

FriendOrderedMultiSet PotentialFriendsSequentialTransform(const SubscriberMap& subscribers, SubscriberID id, int maxCandidates)
{
    // Map:

    FriendsSetPtr friends = subscribers.find(id)->second;
    vector<FriendMultiSetPtr> friendsOfFriends(friends->size());

    transform(friends->cbegin(), friends->cend(), 
        friendsOfFriends.begin(), 
        [&subscribers, &friends, &id](int friendID)->FriendMultiSetPtr 
    {
        FriendsSetPtr theirFriends = subscribers.find(friendID)->second;
        FriendsSetPtr friendsOfFriend = make_shared<FriendsSet>();

        set_difference(theirFriends->cbegin(), theirFriends->cend(),
            friends->cbegin(), friends->cend(), 
            inserter(*friendsOfFriend, friendsOfFriend->end()));
        friendsOfFriend->erase(id);

        return FriendMultiSetPtr(new FriendMultiSet(friendsOfFriend));
    });

    // Reduce:

    // The reduction does not use std:accumulate because this results in too much copying of intermediate FriendCountMap
    FriendMultiSet candidates;
    for_each(friendsOfFriends.cbegin(), friendsOfFriends.cend(), [&candidates](FriendMultiSetPtr set)
    {
        candidates.Union(set);
    });

    // Postprocess:

    return candidates.MostNumerous(maxCandidates);
}

// Parallel implementation using parallel_transform and parallel_reduce

FriendOrderedMultiSet PotentialFriendsParallel(const SubscriberMap& subscribers, SubscriberID id, int maxCandidates)
{
    // Map:

    FriendsSetPtr friends = subscribers.find(id)->second;
    vector<FriendMultiSetPtr> friendsOfFriends(friends->size());

    parallel_transform(friends->cbegin(), friends->cend(), 
        friendsOfFriends.begin(), 
        [&subscribers, &friends, &id](int friendID)->FriendMultiSetPtr 
        {
            FriendsSetPtr theirFriends = subscribers.find(friendID)->second;
            FriendsSetPtr friendsOfFriend = make_shared<FriendsSet>();

            set_difference(theirFriends->cbegin(), theirFriends->cend(), 
                friends->cbegin(), friends->cend(), 
                inserter(*friendsOfFriend, friendsOfFriend->end()));
            friendsOfFriend->erase(id);

            return FriendMultiSetPtr(new FriendMultiSet(friendsOfFriend));
        }
    );

    // Reduce:

    FriendMultiSet candidates;
    candidates = parallel_reduce(friendsOfFriends.cbegin(), friendsOfFriends.cend(), FriendMultiSet(), 
        [](vector<FriendMultiSetPtr>::const_iterator cbegin, vector<FriendMultiSetPtr>::const_iterator cend, const FriendMultiSet& right)
        { 
            return right.Union(cbegin, cend); 
        },
        [](const FriendMultiSet& left, const FriendMultiSet& right)
        { 
            return left.Union(right);
        }
    );

    // Postprocess:

    return candidates.MostNumerous(maxCandidates);
}

/// Usage: CreditReview n m, optional. n is number of customers, use 25,000+ for meaningful timings, m is the number of friends.

int main(int argc, char* argv[])
{
    printf("Social Network Sample\n\n");
#if _DEBUG
    printf("For most accurate timing results, use Release build.\n\n");
#endif

    // Defaults for data generation, may override on command line
#if _DEBUG
    int subscriberCount = 25;
    int maxFriends = 10;
#else
    int subscriberCount = 25000;
    int maxFriends = 2000;
#endif
    if (argc > 1)
        subscriberCount = atoi(argv[1]);
    if (argc > 2)
        maxFriends = atoi(argv[2]);

    // Defaults for printed table of results
    const int maxRows = 16;
    const int maxCols = 8;
    const int maxCandidates = maxRows; 
    int rows;
    const int id = 0;
    SubscriberMap subscribers;
    FriendOrderedMultiSet potentialFriends;

    printf("Creating data: %d subscribers with up to %d friends...\n", subscriberCount, maxFriends);

    // Allocate subscribers, assign friends for timing tests
    if (!AssignRandomFriends(subscribers, subscriberCount, maxFriends, 42))
    {
        printf("Unable to create data, insufficient memory.\n Try again with a smaller data set size.\n");
        getchar();
        exit(1);
    }

    printf("\nFind potential friends for subscriber %d, with these friends and more:\n", id);
    Print(subscribers, 100);
    printf("\n");

    // Sequential
    TimedRun([&potentialFriends, &subscribers, id, maxCandidates]() { potentialFriends = PotentialFriendsSequential(subscribers, id, maxCandidates); }, 
        "  Sequential");
    printf("\n");

    rows = min<int>(maxRows, (int)potentialFriends.size());
    printf("%d potential friends for this subscriber, and the number of mutual friends\n", rows);            
    Print(potentialFriends);
    printf("\n");

    // Sequential transform
    TimedRun([&potentialFriends, &subscribers, id, maxCandidates]() { potentialFriends = PotentialFriendsSequentialTransform(subscribers, id, maxCandidates); }, 
        "  Sequential transform");
    printf("\n");

    rows = min<int>(maxRows, (int)potentialFriends.size());
    printf("%d potential friends for this subscriber, and the number of mutual friends\n", rows);            
    Print(potentialFriends);
    printf("\n");

    // Parallel
    TimedRun([&potentialFriends, &subscribers, id, maxCandidates]() { potentialFriends = PotentialFriendsParallel(subscribers, id, maxCandidates); }, 
        "  Parallel");
    printf("\n");

    rows = min<int>(maxRows, (int)potentialFriends.size());
    printf("%d potential friends for this subscriber, and the number of mutual friends\n", rows);            
    Print(potentialFriends);
    printf("\n");

    printf("\nRun complete... press enter to finish.");
    getchar();
}
