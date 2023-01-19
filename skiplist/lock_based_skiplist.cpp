#include <iostream>
#include <vector>
#include <mutex>
#include <random>

class LockBasedSkipList{
public:

    LockBasedSkipList() {
        head = new Node{INT_MIN};
        tail = new Node{INT_MAX};

        for (int i = 0; i <= head->next.size(); i++) {
            head->next[i] = tail;
        }
    }

    bool add(int key) {
        int topLevel = random_level();
        std::vector<Node*> preds(maxLevel + 1);
        std::vector<Node*> succs(maxLevel + 1);

        while (true) {
            int lFound = find(key, preds, succs);


            if (lFound != -1) {
                Node *nodeFound = succs[lFound];
                if (!nodeFound->marked) {
                    while (!nodeFound->fullyLinked) {}
                    return false;
                }
                continue;
            }


            int highestLocked = -1;
            Node *pred;
            Node *succ;
            bool valid = true;

            for (int level = 0; valid && (level <= topLevel); level++) {

                pred = preds[level];
                succ = succs[level];

                pred->lock.lock();
//                pred->mtx.lock();
                highestLocked = level;
                valid = !pred->marked && !succ->marked && pred->next[level] == succ;
            }

            if (!valid) {
                for (int i = 0; i <= highestLocked; i++) {
                    preds[i]->lock.unlock();
                }
                continue;
            }

            Node *newNode = new Node(key, topLevel);

            for (int i = 0; i <= topLevel; i++) {
                newNode->next[i] = succs[i];
            }
            for (int i = 0; i <= topLevel; i++) {
                preds[i]->next[i] = newNode;
            }

            newNode->fullyLinked = true;
            for (int i = 0; i <= highestLocked; i++) {
                preds[i]->lock.unlock();
            }

            return true;
        }
    }

    bool remove(int key) {
        Node* victim = nullptr;
        bool isMarked = false;
        int topLevel = -1;
        std::vector<Node*> preds(maxLevel + 1);
        std::vector<Node*> succs(maxLevel + 1);

        while(true) {
            int lFound = find(key, preds, succs);
            if (lFound != -1) {
                victim = succs[lFound];
            }
            if (isMarked |
            (lFound != -1 &&
            (victim->fullyLinked
            && victim->topLevel == lFound
            && !victim->marked))) {
                if (!isMarked) {
                    topLevel = victim->topLevel;
                    victim->lock.lock();
                    if (victim->marked) {
                        victim->lock.unlock();
                        return false;
                    }
                    victim->marked = true;
                    isMarked = true;
                }
                int highestLocked = -1;
                Node* pred;

                bool valid = true;
                for (int level = 0; valid && (level <= topLevel); level++) {
                    pred = preds[level];
                    pred->lock.lock();
                    highestLocked = level;
                    valid = !pred->marked && pred->next[level] == victim;
                }
                if (!valid) {
                    for (int i = 0; i <= highestLocked; i++) {
                        preds[i]->lock.unlock();
                    }
                    continue;
                }

                for (int level = topLevel; level >= 0; level--) {
                    preds[level]->next[level] = victim->next[level];
                }
                victim->lock.unlock();

                for (int i = 0; i <= highestLocked; i++) {
                    preds[i]->lock.unlock();
                }
                return true;

            } else {
                return false;
            }
        }
    }

    bool contains(int key) {
        std::vector<Node*> preds(maxLevel + 1);
        std::vector<Node*> succs(maxLevel + 1);

        int lFound = find(key, preds, succs);
        return (lFound != -1
        && succs[lFound]->fullyLinked
        && !succs[lFound]->marked);
    }

    ~LockBasedSkipList() {
        delete(head);
        delete(tail);
    }

private:
    struct Node {
        int key;
        int topLevel;
        bool marked = false;
        bool fullyLinked = false;
        std::vector<Node*> next;
        std::recursive_mutex lock;

        explicit Node(int key) {
            this->key = key;
            next = std::vector<Node*>(maxLevel + 1);
            topLevel = maxLevel;
        }

        Node(int value, int height) {
            this->key = value;
            next = std::vector<Node*>(height + 1);
            topLevel = height;
        }
    };

    Node* head;
    Node* tail;
    std::random_device rd;
    const static int maxLevel = 32;

    int random_level() {
        std::mt19937 gen(rd());
        std::uniform_int_distribution<> dist(1, maxLevel);
        return dist(gen);
    }

    int find(int key, std::vector<Node*>& preds, std::vector<Node*>& succs) {

        Node* pred = head;
        int lFound = -1;
        for (int level = maxLevel; level >= 0; level--) {
            Node* curr = pred->next[level];
            while (key > curr->key) {
                pred = curr;
                curr = pred->next[level];
            }

            if (lFound == -1 && curr->key == key) {
                lFound = level;
            }
            preds[level] = pred;
            succs[level] = curr;
        }

        return lFound;
    }
};