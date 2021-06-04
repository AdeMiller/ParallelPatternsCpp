//===============================================================================
// Microsoft patterns & practices
// Parallel Programming Guide
//===============================================================================
// Copyright © Microsoft Corporation.  All rights reserved.
// This code released under the terms of the 
// Microsoft patterns & practices license (http://parallelpatterns.codeplex.com/license).
//===============================================================================

#include <vector>
#include <iostream>
#include <algorithm>
#include <random>
#include <ppl.h>
#include <concrt.h>
#include <concurrent_vector.h>
#include <concurrent_queue.h>
#include <assert.h>
#include "Tree.h"
#include "SampleUtilities.h"

using namespace ::std;
using namespace ::Concurrency;
using namespace ::SampleUtilities;

const DWORD Time = 10;          // Time (in ms) for CPU intensive work on each node.

// Basics

// Sequential tree traversal

void Example01(const Tree<int>& tree)
{
    vector<int> result;
    Tree<int>::SequentialWalk(tree, [&result](int nodeData) 
    {
        DoCpuIntensiveOperation(Time);
        result.push_back(nodeData);
    });
    printf("\n");
}

// Parallel tree traversal

void Example02(const Tree<int>& tree)
{
    concurrent_vector<int> result;
    Tree<int>::ParallelWalk(tree, [&result](int nodeData)
    {
        DoCpuIntensiveOperation(Time);
        result.push_back(nodeData);
    });
#if _DEBUG
    // Sort results for easy comparison when debugging.
    sort(result.begin(), result.end());
#endif
    printf("\n");
}

// Variations

// Parallel while not empty 1

template<typename T, typename Func>
void ParallelWhileNotEmpty1(vector<shared_ptr<TreeNode<T>>> initialValues, Func body)
{
    concurrent_vector<shared_ptr<TreeNode<T>>> from(initialValues.size());
    for (size_t i = 0; i < initialValues.size(); i++)
        from[i] = initialValues[i];

    while(!from.empty())
    {
        concurrent_vector<shared_ptr<TreeNode<T>>> to;
        function<void (shared_ptr<TreeNode<T>>)> addMethod = 
            [&to](shared_ptr<TreeNode<T>> n) { to.push_back(n); };
        parallel_for_each(from.cbegin(), from.cend(), 
            [&body, &addMethod](shared_ptr<TreeNode<T>> item) 
            { 
                body(item, addMethod); 
            }
        );
        from = to;
    }
}

template<typename T, typename Func>
void ParallelWalkWithWhileNotEmpty1(shared_ptr<TreeNode<T>> node, Func action)
{
    if (nullptr == node)
        return;
    vector<shared_ptr<TreeNode<T>>> nodes;
    nodes.push_back(node);

    ParallelWhileNotEmpty1(nodes, /* Func body */ [&action](shared_ptr<TreeNode<T>> item, function<void (shared_ptr<TreeNode<T>>)>  addMethod)
    {
        if (nullptr != item->Left()) addMethod(item->Left());
        if (nullptr != item->Right()) addMethod(item->Right());
        action(item->Data());
    });
}

void Example03(const Tree<int>& tree)
{
    concurrent_vector<int> result;
    ParallelWalkWithWhileNotEmpty1(tree.Root(), /* Func action */ [&result](int itemData)
    {
        DoCpuIntensiveOperation(Time);
        result.push_back(itemData);
    });
#if _DEBUG
    sort(result.begin(), result.end());
#endif
    printf("\n");
}

// Parallel while not empty 2

template<typename T, typename Func>
void ParallelWhileNotEmpty2(vector<shared_ptr<TreeNode<T>>> initialValues, Func body)
{
    concurrent_queue<shared_ptr<TreeNode<T>>> items(initialValues.cbegin(), initialValues.cend());
    const int maxTasks = 
        CurrentScheduler::Get()->GetNumberOfVirtualProcessors();
    int taskCount = 0;
    function<void (shared_ptr<TreeNode<T>>)> addMethod = [&items](shared_ptr<TreeNode<T>> n) { items.push(n); };

    task_group tasks;
    while (true)
    {
        if (taskCount > 0)
        {
            tasks.wait();
            taskCount = 0;
        }

        if (items.empty())
            break;
        else
        {
            shared_ptr<TreeNode<T>> item;
            while ((taskCount < maxTasks) && items.try_pop(item))
            {
                tasks.run([&body, item, &addMethod]() 
                {
                    body(item, addMethod);
                });
                ++taskCount;
            }
        }
    }
}

template<typename T, typename Func>
void ParallelWalkWithWhileNotEmpty2(shared_ptr<TreeNode<T>> node, Func action)
{
    if (nullptr == node)
        return;
    vector<shared_ptr<TreeNode<T>>> nodes;
    nodes.push_back(node);

    ParallelWhileNotEmpty2(nodes, /* Func body */ [&action](shared_ptr<TreeNode<T>> item, function<void (shared_ptr<TreeNode<T>>)>  addMethod)
    {
        if (nullptr != item->Left()) addMethod(item->Left());
        if (nullptr != item->Right()) addMethod(item->Right());
        action(item->Data());
    });
}

static void Example04(const Tree<int>& tree)
{
    concurrent_vector<int> result;
    ParallelWalkWithWhileNotEmpty2(tree.Root(), /* Func action */ [&result](int itemData)
    {
        DoCpuIntensiveOperation(Time);
        result.push_back(itemData);
    });
#if _DEBUG
    sort(result.begin(), result.end());
#endif
    printf("\n");
}

// Example used for Chapter 6, "Adding Tasks to a Pending Wait Context"

template<typename T, typename Func>
void ParallelSubtreeHandler(task_group& tg, shared_ptr<TreeNode<T>> node, Func action) 
{
    while (nullptr != node)
    {
        // Start up processing the left subtree in a new task
        if (nullptr != node->Left())
        {
            tg.run([&tg, node, action]() { 
                ParallelSubtreeHandler(tg, node->Left(), action); 
            });
        }

        // Process this node i
        tg.run([node, action](){ 
            action(node->Data()); 
        });

        // Walk down the right side of the tree
        node = node->Right();
    }
}

template<typename T, typename Func>
void ParallelTreeUnwinding(shared_ptr<TreeNode<T>> node, Func action)
{
    if (nullptr == node)
        return;

    task_group tg;

    ParallelSubtreeHandler(tg, node, action);

    tg.wait();
}

static void Example05(const Tree<int>& tree)
{
    concurrent_vector<int> result;
    ParallelTreeUnwinding(tree.Root(), /* Func action */ [&result](int itemData)
    {
        DoCpuIntensiveOperation(Time);
        result.push_back(itemData);
    });
#if _DEBUG
    sort(result.begin(), result.end());
#endif
    printf("\n");
}

int main()
{
    printf("Basic Dynamic Task Samples\n\n");
#if _DEBUG
    printf("For most accurate timing results, use Release build.\n\n");
#endif

    const int TreeSize = 2000;                  // number of nodes in the tree
    const double TreeDensity = 0.75;            // P(left child node exists), P(right child node exists) for interior nodes
    const unsigned long seed = 42;              // Random number seed

    printf("Tree Walking\n");

    Tree<int> tree = Tree<int>::MakeTree(TreeSize, TreeDensity, seed);

    TimedRun([&tree]() { Example01(tree); }, "Tree traversal, sequential");

    TimedRun([&tree]() { Example02(tree); }, "Tree traversal, parallel");

    TimedRun([&tree]() { Example03(tree); }, "Tree traversal, parallel while not empty 1");

    TimedRun([&tree]() { Example04(tree); }, "Tree traversal, parallel while not empty 2");

    TimedRun([&tree]() { Example05(tree); }, "Tree traversal, parallel task groups");

    printf("\nRun complete... press enter to finish.");
    getchar();
}
