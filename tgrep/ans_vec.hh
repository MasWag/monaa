#pragma once
#include<vector>
#include <iostream>
#include <iomanip>

#include "zone.hh"

template<class Container>
class AnsContainer
{
protected:
  Container vec;
public:
  AnsContainer(const Container vec) : vec(vec) {}
  AnsContainer() : vec() {}
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

class PrintContainer 
{
private:
  std::size_t count = 0;
  bool isQuiet = false;
public:
  PrintContainer(bool isQuiet) : isQuiet(isQuiet) {}
  std::size_t size() const {
    return count;
  }
  void push_back(const Zone &ans) {
    count++;
    if (!isQuiet) {
      printf("%lg %8s t %s %lg\n", -ans.value(0, 1).first,
             (ans.value(0, 1).second ? "<=" : "<"),
             (ans.value(1, 0).second ? "<=" : "<"),
             ans.value(1, 0).first);
      printf("%lg %8s t' %s %lg\n", -ans.value(0, 2).first,
             (ans.value(0, 2).second ? "<=" : "<"),
             (ans.value(2, 0).second ? "<=" : "<"),
             ans.value(2, 0).first);
      printf("%lg %8s t' - t %s %lg\n", -ans.value(1, 2).first,
             (ans.value(1, 2).second ? "<=" : "<"),
             (ans.value(2, 1).second ? "<=" : "<"),
             ans.value(2, 1).first);
      puts("=============================");
    }
  }
  void clear() {
    count = 0;
  }
  void reserve(std::size_t) {}
  using value_type = Zone;
};

using AnsPrinter = AnsContainer<PrintContainer>;
