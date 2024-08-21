#pragma once

#include "ThreadPool.h"

class ThreadPoolSingleton {
public:
  static ThreadPool &get_instance() {
    static ThreadPool instance(32);
    static bool initialized = false;
    if (!initialized) {
      instance.init();
      initialized = true;
    }
    return instance;
  }

private:
  ThreadPoolSingleton() {}

public:
  ThreadPoolSingleton(ThreadPoolSingleton const &) = delete;
  void operator=(ThreadPoolSingleton const &) = delete;
};
