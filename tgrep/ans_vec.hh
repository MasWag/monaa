#pragma once
#include<vector>

template<class Container>
class AnsContainer
{
protected:
  Container vec;
public:
  std::size_t size() const {
    return vec.size();
  }
  void push_back(typename Container::value_type in) {
    vec.push_back(in);
  }
  void clear() {
    vec.clear();
  }
  void reserve(std::size_t n) {
    vec.reserve(n);
  }
};

template<class T>
class AnsVec : public AnsContainer<std::vector<T>>
{
public:
  typename std::vector<T>::iterator begin() {
    return AnsContainer<std::vector<T>>::vec.begin();
  }
  typename std::vector<T>::iterator end() {
    return AnsContainer<std::vector<T>>::vec.end();
  }
};


template<class T>
class IntContainer 
{
private:
  std::size_t count = 0;
public:
  std::size_t size() const {
    return count;
  }
  void push_back(T) {
    count++;
  }
  void clear() {
    count = 0;
  }
  void reserve(std::size_t) {}
  using value_type = T;
};

template<class T>
using AnsNum = AnsContainer<IntContainer<T>>;
