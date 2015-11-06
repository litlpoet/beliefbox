// Copyright (c) 2005 by Christos Dimitrakakis <christos.dimitrakakis@gmail.com>

#ifndef SRC_MODELS_STATEACTION_H_
#define SRC_MODELS_STATEACTION_H_

#include "HashCombine.h"

/// A convenience class
template <typename S, typename A>
class StateAction {
 public:
  int state;
  int action;

  StateAction(int s, int a) : state(s), action(a) {}

  bool operator==(const StateAction& rhs) const {
    if ((rhs.state != state) || (rhs.action != action)) {
      return false;
    }
    return true;
  }
};

typedef StateAction<int, int> DiscreteStateAction;

namespace std {
template <>
struct hash<DiscreteStateAction> {
  /// The hash is a shift-xor of two hashes
  size_t operator()(const DiscreteStateAction& s) const {
    size_t seed = 0;
    hash_combine(seed, std::hash<int>()(s.state));
    hash_combine(seed, std::hash<int>()(s.action));
    return seed;
  }
};
}  // namespace std

#endif  // SRC_MODELS_STATEACTION_H_
