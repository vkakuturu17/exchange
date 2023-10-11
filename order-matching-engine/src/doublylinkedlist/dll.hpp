/**
 * @file doublylinkedlist.hpp
 */
// #include <stddef.h>
#include "dll.h" // Not going to lie, this is very sus. 
// Might want to look into forward declarations like below.
// template <typename T>
// class DoublyLinkedList;
#include "util_ptr.h"

template <typename T>
DoublyLinkedList<T>::DoublyLinkedList() : head_(nullptr), tail_(nullptr), size_(0) {}

template <typename T>
DoublyLinkedList<T>::DoublyLinkedList(const DoublyLinkedList<T>& other) {}

template <typename T>
DoublyLinkedList<T>& DoublyLinkedList<T>::operator=(const DoublyLinkedList<T>& other) {}

template <typename T>
DoublyLinkedList<T>::~DoublyLinkedList() { clear(); }

template <typename T>
T& DoublyLinkedList<T>::front() {
  return head_->data;
}

template <typename T>
const T& DoublyLinkedList<T>::front() const {
  return head_->data;
}

template <typename T>
T& DoublyLinkedList<T>::back() {
  return tail_->data;
}

template <typename T>
const T& DoublyLinkedList<T>::back() const {
  return tail_->data;
}

template <typename T>
typename DoublyLinkedList<T>::Iterator DoublyLinkedList<T>::begin() {
  return Iterator(head_);
}

template <typename T>
const typename DoublyLinkedList<T>::Iterator DoublyLinkedList<T>::begin() const {
  return Iterator(head_);
}

template <typename T>
typename DoublyLinkedList<T>::Iterator DoublyLinkedList<T>::end() {
  return Iterator();
}

template <typename T>
const typename DoublyLinkedList<T>::Iterator DoublyLinkedList<T>::end() const {
  return Iterator();
}

template <typename T>
bool DoublyLinkedList<T>::empty() {
  return size_ == 0;
}

template <typename T>
size_t DoublyLinkedList<T>::size() const {
  return size_;
}

template <typename T>
void DoublyLinkedList<T>::clear() {
  size_t size = size_;
  for (size_t i = 0; i < size; i++) {
    pop_front();
  }
}

template <typename T>
bool DoublyLinkedList<T>::insert(size_t pos, const T& data) {
  if (pos > size_) {
    return false;
  }

  if (pos == 0) {
    push_front(data);
    return true;
  }

  if (pos == size_) {
    push_back(data);
    return true;
  }

  NodePtr curr = head_;
  for (size_t i = 0; i < pos; i++) {
    curr = curr->next;
  }

  NodePtr new_node = std::make_shared<Node>(data, curr->prev, curr);
  curr->prev->next = new_node;
  curr->prev = new_node;
  size_++;
  return true;
}

template <typename T>
size_t DoublyLinkedList<T>::insert(const T& data) {
  if (size_ == 0 || utils<T>::compare_less(data, head_->data)) {
    push_front(data);
    return 0;
  }

  if (utils<T>::compare_less(tail_->data, data)) {
    push_back(data);
    return size_ - 1;
  }

  NodePtr curr = head_;
  while (curr->next != nullptr && utils<T>::compare_less(curr->data, data)) { // Must use our own compare function for shared ptr
    curr = curr->next;
  }

  if (curr != nullptr) {
    NodePtr new_node = std::make_shared<Node>(data, curr->prev, curr);
    if (curr->prev != nullptr) curr->prev->next = new_node;
    curr->prev = new_node;
    size_++;
    return 0;
  }

  push_back(data);
  return size_ - 1;
}

template <typename T>
void DoublyLinkedList<T>::push_back(const T& data) {
  NodePtr new_node = std::make_shared<Node>(data, tail_, nullptr);
  if (tail_ != nullptr) tail_->next = new_node;
  tail_ = new_node;
  if (head_ == nullptr) head_ = new_node;
  size_++;
}

// template <typename T>
// void DoublyLinkedList<T>::push_back(T&& data) {
//   push_back(data);
// }

template <typename T>
void DoublyLinkedList<T>::pop_back() {
  if (tail_ != nullptr) {
    NodePtr new_tail = tail_->prev;
    tail_.reset(); // Don't need to use utils::remove because tail_ is NodePtr
    tail_ = new_tail;
    if (tail_ != nullptr) tail_->next = nullptr;
    size_--;
  }
}

template <typename T>
void DoublyLinkedList<T>::push_front(const T& data) {
  NodePtr new_node = std::make_shared<Node>(data, nullptr, head_);
  if (head_ != nullptr) head_->prev = new_node;
  head_ = new_node;
  if (tail_ == nullptr) tail_ = new_node;
  size_++;
}

template <typename T>
void DoublyLinkedList<T>::pop_front() {
  if (head_ != nullptr) {
    NodePtr new_head = head_->next;
    head_.reset(); // Don't need to use utils::remove because head_ is NodePtr
    head_ = new_head;
    if (head_ != nullptr) head_->prev = nullptr;
    size_--;
  }
}

template <typename T>
DoublyLinkedList<T>::Iterator DoublyLinkedList<T>::erase(Iterator pos) {
  if (pos == end()) {
    return end();
  }

  NodePtr curr = pos.position_;
  if (curr == head_) {
    pop_front();
    return Iterator(head_);
  }

  if (curr == tail_) {
    pop_back();
    return end();
  }

  curr->prev->next = curr->next;
  curr->next->prev = curr->prev;
  size_--;
  Iterator next_itr(curr->next);
  curr.reset();
  return next_itr;
}

template <typename T>
DoublyLinkedList<T>::Iterator DoublyLinkedList<T>::erase(Iterator first, Iterator last) {
  if (first == end()) {
    return end();
  }

  if (last == end()) {
    pop_back();
    return end();
  }

  NodePtr curr = first.position_;
  if (curr == head_) {
    pop_front();
    return Iterator(head_);
  }

  curr->prev->next = last.position_;
  last.position_->prev = curr->prev;
  size_ -= std::distance(first, last);
  utils<T>::remove(curr);
  return Iterator(last.position_);
}
