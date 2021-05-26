#pragma once
#include <chrono>
#include <iostream>

#ifndef PRODUCT
#define printDuration(proc, mes)                                               \
  start = std::chrono::system_clock::now();                                    \
  proc;                                                                        \
  end = std::chrono::system_clock::now();                                      \
  dur = end - start;                                                           \
  nsec = std::chrono::duration_cast<std::chrono::nanoseconds>(dur).count();    \
  std::cout << std::scientific << mes << nsec / 1000000.0 << " ms" << std::endl
#else
#define printDuration(proc, mes) proc
#endif
