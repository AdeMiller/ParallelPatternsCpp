//===============================================================================
// Microsoft patterns & practices
// Parallel Programming Guide
//===============================================================================
// Copyright © Microsoft Corporation.  All rights reserved.
// This code released under the terms of the 
// Microsoft patterns & practices license (http://parallelpatterns.codeplex.com/license).
//===============================================================================

#pragma once

#include <random>

using namespace ::std;

template<typename T>
struct TreeNode
{
private:
    T m_data;
    shared_ptr<TreeNode<T>> m_left;
    shared_ptr<TreeNode<T>> m_right;

public:
    TreeNode() : m_left(nullptr), m_right(nullptr) {}

    TreeNode(const T& data, shared_ptr<TreeNode<T>> left, shared_ptr<TreeNode<T>> right) 
        : m_data(data), m_left(left), m_right(right) 
    {}

    inline T& Data() { return m_data; }

    inline shared_ptr<TreeNode<T>> Left(){ return m_left; }

    inline shared_ptr<TreeNode<T>> Right(){ return m_right; }
};

template<typename T>
class Tree
{
private:
    shared_ptr<TreeNode<T>> m_root;

public:
    Tree(shared_ptr<TreeNode<T>> root) : m_root(root) {}

    shared_ptr<TreeNode<T>> Root() const { return m_root; } 

    static Tree<int> MakeTree(int nodeCount, double density, unsigned long seed)
    {
        default_random_engine engine;
        engine.seed(seed);
        uniform_int_distribution<double> distribution(0.0, 1.0);
        auto random = bind(distribution, engine);

        return Tree<int>(MakeTreeNode(nodeCount, density, 0, random));
    }

    template<typename Func>
    static void SequentialWalk(const Tree<T>& tree, Func action)
    {
        SequentialWalk(tree.Root(), action);
    }

    template<typename Func>
    static void ParallelWalk(const Tree<T>& tree, Func action)
    {
        ParallelWalk(tree.Root(), action);
    }

private:
    template <typename Random>

    static shared_ptr<TreeNode<int>> MakeTreeNode(int nodeCount, double density, int offset, Random& random)
    {
        bool addNodeLeft = random() > density;
        bool addNodeRight = random() > density;
        int newCount = nodeCount - 1;
        int countLeft = (addNodeLeft && addNodeRight) ? (newCount / 2) : addNodeLeft ? newCount : 0;
        int countRight = newCount - countLeft;
        if (random() > 0.5)
            swap(countLeft, countRight);

        return shared_ptr<TreeNode<int>>(new TreeNode<int>(offset,
                         (countLeft > 0) ? MakeTreeNode(countLeft, density, offset + 1, random) : nullptr,
                         (countRight > 0) ? MakeTreeNode(countRight, density, offset + 1 + countLeft, random) : nullptr));
    }

    template<typename Func>
    static void SequentialWalk(shared_ptr<TreeNode<T>> node, 
                               Func action)
    {
        if (nullptr == node) return;

        action(node->Data());
        SequentialWalk(node->Left(), action);
        SequentialWalk(node->Right(), action);
    }

    template<typename Func>
    static void ParallelWalk(shared_ptr<TreeNode<T>> node, 
                             Func action)
    {
        if (nullptr == node) return;

        parallel_invoke(
            [&node, &action] { action(node->Data()); },
            [&node, &action] { Tree<T>::ParallelWalk(node->Left(), action); },
            [&node, &action] { Tree<T>::ParallelWalk(node->Right(), action); }
        );
    }
};
