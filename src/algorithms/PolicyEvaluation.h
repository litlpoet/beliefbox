// -*- Mode: c++ -*-
// copyright (c) 2006 by Christos Dimitrakakis <christos.dimitrakakis@gmail.com>
// $Revision$
/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef POLICY_EVALUATION_H
#define POLICY_EVALUATION_H

#include <vector>
#include "DiscreteMDP.h"
#include "DiscretePolicy.h"
#include "real.h"

class PolicyEvaluation {
 public:
  FixedDiscretePolicy* policy;
  const DiscreteMDP* mdp;
  Matrix FeatureMatrix;
  real gamma;
  int n_states;
  int n_actions;
  Vector V;
  real Delta;
  real baseline;
  PolicyEvaluation(FixedDiscretePolicy* policy_, const DiscreteMDP* mdp_,
                   real gamma_, real baseline_ = 0.0);
  virtual ~PolicyEvaluation();
  virtual void ComputeStateValues(real threshold, int max_iter = -1);
  virtual void ComputeStateValuesFeatureExpectation(real threshold,
                                                    int max_iter = -1);
  virtual void RecomputeStateValuesFeatureExpectation();
  inline void SetPolicy(FixedDiscretePolicy* policy_) { policy = policy_; }
  void Reset();
  real getValue(int state, int action) const;
  inline real getValue(int state) const { return V[state]; }
  inline void setGamma(real gamma_) {
    assert(gamma_ >= 0 && gamma_ <= 1);
    gamma = gamma_;
  }
};

#endif
