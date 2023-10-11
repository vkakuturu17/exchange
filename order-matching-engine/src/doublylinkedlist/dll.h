/**
 * @file doublylinkedlist.h
 */

#pragma once
#include <stddef.h>
#include <memory>


/**
 * Inspiration: https://en.cppreference.com/w/cpp/container/list
 */
template <class T>
class DoublyLinkedList {
  public:
    struct Node;
    typedef std::shared_ptr<Node> NodePtr;
    typedef std::weak_ptr<Node> NodeWeakPtr;
    struct Node {
      T data;
      NodePtr prev;
      NodePtr next;

      Node(const T& data) : data(data), prev(nullptr), next(nullptr) {}
      Node(const T& data, NodePtr prev, NodePtr next) : data(data), prev(prev), next(next) {}
      Node(const Node& other) : data(other.data), prev(other.prev), next(other.next) {}
    };
    
    #include "dll_iterator.hpp"

    DoublyLinkedList();
    DoublyLinkedList(const DoublyLinkedList&);
    DoublyLinkedList& operator=(const DoublyLinkedList&);
    ~DoublyLinkedList();

    // Element Access
    T& front();
    const T& front() const;
    T& back();
    const T& back() const;

    // Iterators
    Iterator begin();
    const Iterator begin() const;
    Iterator end();
    const Iterator end() const;

    // Capacity
    bool empty();
    size_t size() const;
    // size_t max_size();

    // Modifiers
    void clear();
    bool insert(size_t pos, const T& data);
    size_t insert(const T& data); // Uses comparator to insert in correct position
    void push_back(const T& data);
    // void push_back(T&& data);
    void pop_back();
    void push_front(const T& data);
    void pop_front();
    // void resize();
    Iterator erase(Iterator pos); // Not tested yet
    Iterator erase(Iterator first, Iterator last); // Not tested yet

    // Operations
    // void merge(DoublyLinkedList& other);
    // template <class Compare>
    // void merge(DoublyLinkedList& other, Compare comp);
    // void sort();
    // template <class Compare>
    // void sort(Compare comp);

  private:
    // static const bool IS_POINTER = std::is_pointer<T>::value;
    NodePtr head_;
    NodePtr tail_;
    size_t size_ = 0;
    T header_struct_; // optional
};

#include "dll.hpp"
