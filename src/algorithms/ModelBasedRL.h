// -*- Mode: c++ -*-
// copyright (c) 2009 by Christos Dimitrakakis <christos.dimitrakakis@gmail.com>
// $Revision$
/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#ifndef SRC_ALGORITHMS_MODELBASEDRL_H_
#define SRC_ALGORITHMS_MODELBASEDRL_H_

#include "OnlineAlgorithm.h"

#include <vector>

#include "DiscreteMDP.h"
#include "DiscretePolicy.h"
#include "ExplorationPolicy.h"
#include "MDPModel.h"
#include "RandomNumberGenerator.h"
#include "ValueIteration.h"

/**
    \ingroup ReinforcementLearning
 */
/*@{*/

/** Direct model-based reinforcement learning.

    This class maintains a model of the (discrete) MDP.

 */
class ModelBasedRL : public OnlineAlgorithm<int, int> {
 protected:
  const int n_states;   ///< number of states
  const int n_actions;  ///< number
  real gamma;           ///< discount factor
  real epsilon;         ///< randomness
  int state;            ///< current state
  int action;           ///< current action
  MDPModel* model;
  const DiscreteMDP* mdp;
  ValueIteration* value_iteration;
  std::vector<real> tmpQ;
  RandomNumberGenerator* rng;
  bool use_value_iteration;
  int total_steps;

 public:
  ModelBasedRL(int n_states_, int n_actions_, real gamma_, real epsilon_,
               MDPModel* model_, RandomNumberGenerator* rng_,
               bool use_value_iteration_ = true);

  virtual ~ModelBasedRL();

  virtual void Reset();

  /// Full observation
  virtual real Observe(int state, int action, real reward, int next_state,
                       int next_action);

  /// Partial observation
  virtual real Observe(real reward, int next_state, int next_action);

  /// Get an action using the current exploration policy.
  /// it calls Observe as a side-effect.
  virtual int Act(real reward, int next_state);

  virtual real getValue(int state, int action) {
    if (use_value_iteration) {
      return value_iteration->getValue(state, action);
    } else {
      return 0.0;
    }
  }

  virtual void setFixedRewards(const Matrix& rewards) {
    model->setFixedRewards(rewards);
    value_iteration->ComputeStateValues(1e-6, -1);
  }
};

/*@}*/
#endif  // SRC_ALGORITHMS_MODELBASEDRL_H_
