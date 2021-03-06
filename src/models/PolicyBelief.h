// -*- Mode: c++ -*-
// copyright (c) 2011 by Christos Dimitrakakis <christos.dimitrakakis@gmail.com>
/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef POLICY_BELIEF_H
#define POLICY_BELIEF_H

#include <vector>
#include "Dirichlet.h"
#include "real.h"

class DiscretePolicy;
class FixedDiscretePolicy;

template <class S, class C>
class Demonstrations;

/** A belief about rewards in discrete spaces.

 */
class DiscretePolicyBelief {
 protected:
  int n_states;
  int n_actions;

 public:
  // only set up
  DiscretePolicyBelief(int n_states_, int n_actions_)
      : n_states(n_states_), n_actions(n_actions_) {}

  virtual ~DiscretePolicyBelief() {}

  virtual real Update(int state, int action) = 0;
  virtual real CalculatePosterior(Demonstrations<int, int>& D) = 0;
  virtual FixedDiscretePolicy* Sample() const = 0;
  virtual FixedDiscretePolicy* getExpectedPolicy() const = 0;
  // virtual real getLogDensity(const DiscretePolicy& policy) const = 0;
  // virtual real getDensity(const DiscretePolicy& policy) const = 0;
};

/** A belief about rewards in discrete spaces.

 */
class DirichletProductPolicyBelief : public DiscretePolicyBelief {
 protected:
  std::vector<DirichletDistribution> P;  ///< dirichlet distribution
 public:
  // only set up
  DirichletProductPolicyBelief(int n_states_, int n_actions_)
      : DiscretePolicyBelief(n_states_, n_actions_), P(n_states) {
    for (int i = 0; i < n_states; ++i) {
      P[i].resize(n_actions);
    }
  }

  virtual ~DirichletProductPolicyBelief() {}

  virtual real Update(int state, int action);
  virtual real CalculatePosterior(Demonstrations<int, int>& D);
  virtual FixedDiscretePolicy* Sample() const;
  virtual FixedDiscretePolicy* getExpectedPolicy() const;
  virtual real getLogDensity(const FixedDiscretePolicy& policy) const;
  virtual real getDensity(const FixedDiscretePolicy& policy) const;
};

#endif
