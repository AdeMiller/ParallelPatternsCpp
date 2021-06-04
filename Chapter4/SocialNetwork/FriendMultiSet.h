#pragma once;

#include <set>
#include <hash_map>

using namespace ::std;

typedef int                                                     SubscriberID;
typedef int                                                     FriendID;

// A set of unique friendIDs.

typedef set<FriendID>                                           FriendsSet;
typedef pair<SubscriberID, FriendID>                            FriendsSetPair;

typedef shared_ptr<FriendsSet>                                  FriendsSetPtr;

// A map of subscribers. A subscriber is an ID and a set of friendIDs. 
// A hash_map is used here for O(1) lookup of subscribers by ID.

typedef hash_map<SubscriberID, FriendsSetPtr>                   SubscriberMap;
typedef pair<SubscriberID, FriendsSetPtr>                       SubscriberMapPair;

// An ordered std::multiset used to display to most likely potential friends. The collection is
// sorted by the number of mutual friends. 

// Function object (functor) to order items by friend count
// For friends with the same number of mutual friends then sort by friend ID.

struct LessMultisetItem
{
    bool operator()(const pair<FriendID, int> value1, const pair<FriendID, int> value2) const
    {
        return (value1.second == value2.second) ? (value1.first > value2.first) : (value1.second > value2.second);
    }
};

typedef public set<pair<FriendID, int>, LessMultisetItem>  FriendOrderedMultiSet;

// A multiset containing pairs of FriendID and their multiplicity.
// Not to be confused with std::multiset.

class FriendMultiSet;
typedef shared_ptr<FriendMultiSet>                              FriendMultiSetPtr;
typedef pair<FriendID, int>                                     FriendMultiSetPair;

class FriendMultiSet : public hash_map<FriendID, int>
{
public:
    FriendMultiSet() {}

    FriendMultiSet(FriendsSetPtr set) 
    {
        for_each(set->cbegin(), set->cend(), [this](FriendID id) { this->insert(FriendMultiSetPair(id, 1)); });
    }

    FriendMultiSet Union(vector<FriendMultiSetPtr>::const_iterator cbegin, vector<FriendMultiSetPtr>::const_iterator cend) const
    {
        FriendMultiSet result(*this);

        for_each(cbegin, cend, [&result](FriendMultiSetPtr set) { result.Union(set); });
        return result;
    }

    void Union(FriendMultiSetPtr set)
    {
        for_each(set->cbegin(), set->cend(), [this](FriendsSetPair val)
        {
            FriendMultiSet::iterator itr = this->find(val.first);
            if (itr == this->end())
                this->insert(FriendMultiSetPair(val.first, 1));
            else
                itr->second += val.second;
        });
    }

    FriendMultiSet Union(const FriendMultiSet& set) const
    {
        FriendMultiSet result(*this);

        for_each(set.cbegin(), set.cend(), [&result](FriendsSetPair val)
        {
            FriendMultiSet::iterator itr = result.find(val.first);
            if (itr == result.end())
                result.insert(FriendMultiSetPair(val.first, 1));
            else
                itr->second += val.second;
        });
        return result;
    }

    FriendOrderedMultiSet MostNumerous(size_t maxFriends) const
    {
        FriendOrderedMultiSet result;

        for_each(this->cbegin(), this->cend(), [&result, maxFriends](FriendsSetPair val)
        {
            result.insert(val);
            if (result.size() > maxFriends)
                result.erase(--result.end());
        });
        return result;
    }
};
