#include <iostream>
#include <vector>
#include <mutex>
#include <random>
#include <atomic>
#include "marked_ptr.cpp"
#include "tagged_ptr.cpp"
#include "exp_backoff.cpp"

class LockFreeSkipList{
public:

    LockFreeSkipList() {
        head.ptr = new Node(INT_MIN);
        tail.ptr = new Node(INT_MAX);

        for (auto & i : head->next) {
            i = MarkedPtr<Node>(tail.ptr, false, tail.tag);
        }
    }

    bool add(int key) {
        int topLevel = random_level();
        int bottomLevel = 0;
        std::vector<TaggedPtr<Node>> preds(maxLevel + 1);
        std::vector<TaggedPtr<Node>> succs(maxLevel + 1);

        while (true) {
            bool found = find(key, preds, succs);
            if (found) {
                return false;
            } else {
                Node *newNode = new Node(key, topLevel);
                for (int level = bottomLevel; level <= topLevel; level++) {
                    TaggedPtr<Node> succ = succs[level];
                    MarkedPtr<Node> newMarkedPtr(succ.ptr, false, succ.tag);
                    newNode->next[level].store(newMarkedPtr, std::memory_order_relaxed);
                }

                TaggedPtr<Node> pred = preds[bottomLevel];
                TaggedPtr<Node> succ = succs[bottomLevel];
                MarkedPtr<Node> oldMarkedNode(succ.ptr, false, succ.tag);
                newNode->next[bottomLevel].store(oldMarkedNode, std::memory_order_relaxed);
                MarkedPtr<Node> newMarkedNode(newNode, false,
                                              universalTag.load(std::memory_order_relaxed) + 1);

                if (!pred->next[bottomLevel].compare_exchange_weak(
                        oldMarkedNode, newMarkedNode,
                        std::memory_order_release, std::memory_order_relaxed)) {
                    continue;
                }

                for (int level = bottomLevel + 1; level <= topLevel; level++) {
                    while (true) {
                        pred = preds[level];
                        succ = succs[level];

                        MarkedPtr<Node> oldNode(succ.ptr, false, succ.tag);
                        if (pred->next[level].compare_exchange_weak(
                                oldNode, newMarkedNode,
                                std::memory_order_release, std::memory_order_relaxed)) {
                            break;
                        }
                        find(key, preds, succs);
                    }
                }
                return true;
            }
        }
    }

    bool remove(int key) {
        int bottomLevel = 0;
        std::vector<TaggedPtr<Node>> preds(maxLevel + 1);
        std::vector<TaggedPtr<Node>> succs(maxLevel + 1);
        MarkedPtr<Node> markedSucc;
        MarkedPtr<Node> newMarkedSucc;

        while(true) {
            bool found = find(key, preds, succs);
            if (!found) {
                return false;
            } else {
                TaggedPtr<Node> nodeToRemove = succs[bottomLevel];
                for (int level = nodeToRemove->topLevel; level >= bottomLevel + 1; level--) {
                    bool marked = false;
                    markedSucc = nodeToRemove->next[level].load(std::memory_order_relaxed);
                    while(!marked) {
                        newMarkedSucc = MarkedPtr<Node>(markedSucc.ptr, true, markedSucc.tag);
                        nodeToRemove->next[level].compare_exchange_weak(
                                markedSucc, newMarkedSucc,
                                std::memory_order_acquire, std::memory_order_relaxed);
                        markedSucc = nodeToRemove->next[level].load(std::memory_order_relaxed);
                        marked = markedSucc.marked;
                    }
                }
                bool marked = false;

                markedSucc = nodeToRemove->next[bottomLevel].load(std::memory_order_relaxed);
                while (true) {
                    MarkedPtr<Node> oldPtr(markedSucc.ptr, false, markedSucc.tag);
                    MarkedPtr<Node> newPtr(markedSucc.ptr, true, markedSucc.tag);
                    bool iMarkedIt = nodeToRemove->next[bottomLevel].compare_exchange_weak(
                            oldPtr, newPtr,
                            std::memory_order_acquire, std::memory_order_relaxed);
                    markedSucc = succs[bottomLevel]->next[bottomLevel].load(std::memory_order_relaxed);
                    marked = markedSucc.marked;
                    if (iMarkedIt) {
                        find(key, preds, succs);
                        return true;
                    } else if (marked) {
                        return false;
                    }
                }

            }
        }

    }

    bool contains(int key) {
        int bottomLevel = 0;
        bool marked = false;
        TaggedPtr<Node> pred = head;
        MarkedPtr<Node> markedCurr{};
        MarkedPtr<Node> markedSucc{};


        for (int level = maxLevel; level >= bottomLevel; level--) {
            markedCurr = pred->next[level].load(std::memory_order_relaxed);
            while (true) {
                markedSucc = markedCurr.ptr->next[level].load(std::memory_order_relaxed);
                marked = markedSucc.marked;
                while(marked) {
                    markedCurr = pred.ptr->next[level].load(std::memory_order_relaxed);
                    markedSucc = markedCurr.ptr->next[level].load(std::memory_order_relaxed);
                    marked = markedSucc.marked;
                }
                if (markedCurr.ptr->key < key) {
                    pred = TaggedPtr<Node>(markedCurr.ptr, markedCurr.tag);
                    markedCurr = markedSucc;
                } else {
                    break;
                }
            }
        }
        return markedCurr.ptr->key == key;
    }

    ~LockFreeSkipList() {
        delete(head.ptr);
        delete(tail.ptr);
    }

private:
    struct Node {
        int key;
        int topLevel;
        std::vector<std::atomic<MarkedPtr<Node>>> next;

        explicit Node(int value) {
            key = value;
            next = std::vector<std::atomic<MarkedPtr<Node>>>(maxLevel + 1);
            for (auto & i : next) {
                i = MarkedPtr<Node>(false);
            }
            topLevel = maxLevel;
        }

        Node(int value, int height) {
            key = value;
            next = std::vector<std::atomic<MarkedPtr<Node>>>(height + 1);
            for (auto & i : next) {
                i = MarkedPtr<Node>(false);
            }
            topLevel = height;
        }
    };

    TaggedPtr<Node> head{0u};
    TaggedPtr<Node> tail{UINT_MAX};
    std::atomic<unsigned> universalTag{1u};
    std::random_device rd;
    const static int maxLevel = 32;


    int random_level() {
        std::mt19937 gen(rd());
        std::uniform_int_distribution<> dist(1, maxLevel);
        return dist(gen);
    }

    bool find(int key, std::vector<TaggedPtr<Node>>& preds, std::vector<TaggedPtr<Node>>& succs) {
        ExpBackoff backoff = ExpBackoff();
        int bottomLevel = 0;
        bool marked = false;

        TaggedPtr<Node> pred{};
        TaggedPtr<Node> curr{};
        TaggedPtr<Node> succ{};
        MarkedPtr<Node> currMarked;
        MarkedPtr<Node> succMarkedPtr;

        retry:
        while (true) {
            pred = head;
            for (int level = maxLevel; level >= bottomLevel; level--) {

                currMarked = pred->next[level].load(std::memory_order_relaxed);
                curr = TaggedPtr<Node>(currMarked.ptr, currMarked.tag);

                while (true) {
                    succMarkedPtr = curr->next[level].load(std::memory_order_relaxed);
                    succ = TaggedPtr<Node>(succMarkedPtr.ptr, succMarkedPtr.tag);
                    marked = succMarkedPtr.marked;
                    while (marked) {
                        MarkedPtr<Node> oldMarkedNode(curr.ptr, false, curr.tag);
                        MarkedPtr<Node> newMarkedNode(succ.ptr, false,
                                                      universalTag.load(std::memory_order_relaxed) + 1);

                        if (!pred->next[level].compare_exchange_weak(
                                oldMarkedNode, newMarkedNode,
                                std::memory_order_release, std::memory_order_relaxed)) {
//                            backoff();
                            goto retry;
                        }

                        currMarked = pred->next[level].load(std::memory_order_relaxed);
                        curr = TaggedPtr<Node>(currMarked.ptr, currMarked.tag);

                        succMarkedPtr = curr->next[level].load(std::memory_order_relaxed);
                        succ = TaggedPtr<Node>(succMarkedPtr.ptr, succMarkedPtr.tag);
                        marked = succMarkedPtr.marked;
                    }

                    if (curr->key < key) {
                        pred = curr;
                        curr = succ;
                    } else {
                        break;
                    }
                }
                preds[level] = pred;
                succs[level] = curr;
            }
            return curr->key == key;
        }
    }
};