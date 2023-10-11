#pragma once
#include <memory>

template <class T>
class utils {
  public:
    static bool compare_less(const T& a, const T& b) {
      return a < b;
    }

    static void remove(const T& a) {}
};

template <class T>
class utils<std::shared_ptr<T>>{
  public:
      static bool compare_less(const std::shared_ptr<T>& a, const std::shared_ptr<T>& b) {
        return *a < *b;
        // Theoretically this line should use compare_less(*a, *b) but I can't get it to work.
        // The reason you use compare_less is because if it is nested you must overload again until
        // you no longer have a shared ptr... at least I think.
      }

      static void remove(const std::shared_ptr<T>& a) {
        a.reset();
      }
};

template <class T>
class utils<T*> {
  public:
    static bool compare_less(const T* a, const T* b) {
      return *a < *b;
    }

    static void remove(const T* a) {
      delete a;
    }
};
