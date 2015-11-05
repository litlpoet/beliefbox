// -*- Mode: c++ -*-
// copyright (c) 2010 by Christos Dimitrakakis <christos.dimitrakakis@gmail.com>

#ifndef RESOURCE_USE_H
#define RESOURCE_USE_H

#include <sys/resource.h>
#include <sys/time.h>
#include <unistd.h>
#include <ctime>

inline long getMaxResidentSetSize() {
  static struct rusage usage;
  getrusage(RUSAGE_SELF, &usage);
  return usage.ru_maxrss;
}

inline long getSharedMemorySize() {
  static struct rusage usage;
  getrusage(RUSAGE_SELF, &usage);
  return usage.ru_ixrss;
}

inline long getUnsharedDataSetSize() {
  static struct rusage usage;
  getrusage(RUSAGE_SELF, &usage);
  return usage.ru_idrss;
}

inline long getUnsharedStackSize() {
  static struct rusage usage;
  getrusage(RUSAGE_SELF, &usage);
  return usage.ru_isrss;
}

#endif
