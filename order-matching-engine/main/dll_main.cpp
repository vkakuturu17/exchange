#include <iostream>
#include <memory>
#include <vector>
#include "struct/structs.h"
#include "doublylinkedlist/dll.h"

using std::cout;
using std::endl;
using std::unique_ptr;
using std::vector;

int main() {
    int a = 5;
    int b = 6;
    int c = 7;
    DoublyLinkedList<int> dll;
    dll.push_back(a);
    dll.push_back(b);
    dll.insert(1, c);
    
    cout << dll.front() << endl;
    cout << dll.back() << endl;

    dll.clear();
    return 0;
}