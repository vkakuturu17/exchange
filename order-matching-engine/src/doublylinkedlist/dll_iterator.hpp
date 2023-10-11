
class Iterator : public std::iterator<std::bidirectional_iterator_tag, T> {
  private:

  public:
    NodePtr position_;
    
    Iterator() : position_(nullptr) {}
    Iterator(NodePtr position) : position_(position) {}
    Iterator(const Iterator& other) : position_(other.position_) {}
    Iterator& operator=(const Iterator& other) {
      if (this != &other) {
        position_ = other.position_;
      }
      return *this;
    }
    ~Iterator() {}

    bool operator==(const Iterator& other) const {
      return position_ == other.position_;
    }
    bool operator!=(const Iterator& other) const {
      return position_ != other.position_;
    }
    T& operator*() const {
      return position_->data;
    }
    T* operator->() const {
      return &(position_->data);
    }
    Iterator& operator++() {
      if (position_ != nullptr) position_ = position_->next;
      return *this;
    }
    Iterator operator++(int) {
      Iterator tmp(*this);
      ++(*this);
      return tmp;
    }
    Iterator& operator--() {
      if (position_ != nullptr) position_ = position_->prev;
      return *this;
    }
    Iterator operator--(int) {
      Iterator tmp(*this);
      --(*this);
      return tmp;
    }

};