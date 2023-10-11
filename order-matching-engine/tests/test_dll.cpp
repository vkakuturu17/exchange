#include <catch2/catch.hpp>
#include <iostream>
#include <vector>

#include "doublylinkedlist/dll.h"

using std::vector;
using std::cout;
using std::endl;

// template <typename T>
// class DoublyLinkedList<T>::Iterator Iterator;

template <typename T>
void REQUIRE_DLL_MATCHES_VECTOR(const DoublyLinkedList<T>& dll, const vector<T>& vec) {
    REQUIRE(dll.size() == vec.size());
    auto dll_it = dll.begin();
    auto vec_it = vec.begin();
    while (dll_it != dll.end()) {
        REQUIRE(*dll_it == *vec_it);
        dll_it++;
        vec_it++;
    }
}

TEST_CASE("Doubly Linked List (Value)", "[dll][dll_value]") {
  SECTION("Modifiers", "[modifiers]") {
    DoublyLinkedList<int> dll;
    dll.push_back(1);
    dll.push_back(2);
    dll.push_back(3);
    dll.pop_back();
    dll.push_front(0);
    dll.pop_front();
    dll.pop_front();
    REQUIRE(dll.size() == 1);
    REQUIRE(dll.front() == 2);
    REQUIRE(dll.back() == 2);
    REQUIRE(*dll.begin() == 2);

    dll.clear();
    REQUIRE(0 == dll.size());
    REQUIRE(dll.empty());
  }

  SECTION("DoublyLinkedList<T>::insert", "[insert]") {
    SECTION("Insert by position") {
      DoublyLinkedList<int> dll;
      dll.push_back(1);
      dll.push_back(2);
      dll.push_back(3);
      dll.insert(1, 4);
      REQUIRE(dll.size() == 4);
      REQUIRE(dll.front() == 1);
      REQUIRE(dll.back() == 3);
      vector<int> vec = {1, 4, 2, 3};
      REQUIRE_DLL_MATCHES_VECTOR(dll, vec);
    }

    SECTION("Insert in order") {
      DoublyLinkedList<int> dll;
      dll.push_back(1);
      dll.push_back(2);
      dll.push_back(3);
      dll.push_back(100);
      dll.insert(4);
      dll.insert(2);
      dll.insert(0);
      dll.insert(101);
      REQUIRE(dll.size() == 8);
      REQUIRE(dll.front() == 0);
      REQUIRE(dll.back() == 101);
      vector<int> vec = {0, 1, 2, 2, 3, 4, 100, 101};
      REQUIRE_DLL_MATCHES_VECTOR(dll, vec);
    }
  }
}

TEST_CASE("Doubly Linked List (Pointer)", "[dll][dll_ptr]") {
  // Memory leak for pointers

  // SECTION("Modifiers") {
  //   DoublyLinkedList<int*> dll;
  //   int* a = new int(1);
  //   int* b = new int(2);
  //   int* c = new int(3);
  //   dll.push_back(a);
  //   dll.push_back(b);
  //   dll.push_back(c);
  //   dll.pop_back();
  //   dll.push_front(a);
  //   dll.pop_front();
  //   dll.pop_front();
  //   REQUIRE(dll.size() == 1);
  //   REQUIRE(*dll.front() == 2);
  //   REQUIRE(*dll.back() == 2);
  //   REQUIRE(**dll.begin() == 2);

  //   dll.clear();
  //   REQUIRE(dll.size() == 0);
  //   REQUIRE(dll.empty());
  // }
}

TEST_CASE("Doubly Linked List (shared_ptr)", "[dll][dll_ptr][dll_shared_ptr]") {
  SECTION("Modifiers") {
    DoublyLinkedList<std::shared_ptr<int>> dll;
    std::shared_ptr<int> a = std::make_shared<int>(1);
    std::shared_ptr<int> b = std::make_shared<int>(2);
    std::shared_ptr<int> c = std::make_shared<int>(3);
    dll.push_back(a);
    dll.push_back(b);
    dll.push_back(c);
    dll.pop_back();
    dll.push_front(a);
    dll.pop_front();
    dll.pop_front();
    REQUIRE(dll.size() == 1);
    REQUIRE(*dll.front() == 2);
    REQUIRE(*dll.back() == 2);
    REQUIRE(**dll.begin() == 2);

    dll.clear();
    REQUIRE(dll.size() == 0);
    REQUIRE(dll.empty());
  }

  SECTION("Erase") {
    DoublyLinkedList<std::shared_ptr<int>> dll;
    std::shared_ptr<int> a = std::make_shared<int>(1);
    std::shared_ptr<int> b = std::make_shared<int>(2);
    std::shared_ptr<int> c = std::make_shared<int>(3);
    dll.push_back(a);
    dll.push_back(b);
    dll.push_back(c);

    SECTION("Beginning") {
      auto it = dll.begin();
      dll.erase(it);
      REQUIRE(dll.size() == 2);
      REQUIRE(*dll.front() == 2);
      REQUIRE(*dll.back() == 3);
      vector<std::shared_ptr<int>> vec = {b, c};
      REQUIRE_DLL_MATCHES_VECTOR(dll, vec);
    }

    SECTION("Middle") {
      auto it = dll.begin();
      it++;
      dll.erase(it);
      REQUIRE(dll.size() == 2);
      REQUIRE(*dll.front() == 1);
      REQUIRE(*dll.back() == 3);
      vector<std::shared_ptr<int>> vec = {a, c};
      REQUIRE_DLL_MATCHES_VECTOR(dll, vec);
    }

    SECTION("End") {
      auto it = ++(++dll.begin());
      dll.erase(it);
      REQUIRE(dll.size() == 2);
      REQUIRE(*dll.front() == 1);
      REQUIRE(*dll.back() == 2);
      vector<std::shared_ptr<int>> vec = {a, b};
      REQUIRE_DLL_MATCHES_VECTOR(dll, vec);
    }
  }
}
