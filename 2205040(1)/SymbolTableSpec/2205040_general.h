#ifndef GENERAL_H
#define GENERAL_H

#include <string>

template <typename T>
class mylist {
public:
    struct Node {
        T data;
        Node* next;
        Node(const T& d) : data(d), next(nullptr) {}
    };

private:
    Node* head;

public:
    class iterator {
        Node* ptr;
    public:
        iterator(Node* p) : ptr(p) {}
        T& operator*() { return ptr->data; }
        T* operator->() { return &ptr->data; }
        iterator& operator++() { ptr = ptr->next; return *this; }
        bool operator!=(const iterator& other) const { return ptr != other.ptr; }
    };

    iterator begin() { return iterator(head); }
    iterator end() { return iterator(nullptr); }

    mylist() : head(nullptr) {}

    Node* getHead() { return head; }

    void push_back(const T& data) {
        Node* n = new Node(data);
        if (!head) {
            head = n;
            return;
        }
        Node* curr = head;
        while (curr->next) curr = curr->next;
        curr->next = n;
    }

    template <typename Pred>
    Node* find_if(Pred pred) {
        Node* curr = head;
        while (curr) {
            if (pred(curr->data)) return curr;
            curr = curr->next;
        }
        return nullptr;
    }

    template <typename Pred>
    bool remove_if(Pred pred) {
        Node* curr = head;
        Node* prev = nullptr;
        while (curr) {
            if (pred(curr->data)) {
                if (prev) prev->next = curr->next;
                else head = curr->next;
                delete curr;
                return true;
            }
            prev = curr;
            curr = curr->next;
        }
        return false;
    }

    template <typename Func>
    void for_each(Func func) {
        Node* curr = head;
        while (curr) {
            func(curr->data);
            curr = curr->next;
        }
    }

    ~mylist() {
        Node* curr = head;
        while (curr) {
            Node* temp = curr;
            curr = curr->next;
            delete temp;
        }
    }
};

#endif
