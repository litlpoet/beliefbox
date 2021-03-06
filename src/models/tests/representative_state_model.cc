// -*- Mode: c++ -*-
// copyright (c) 2009 by Christos Dimitrakakis <christos.dimitrakakis@gmail.com>
/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

/** Test that the BPSR model predicts the next observations well.

    The BPSRModel gives probabilities of next observations.
    It needs a full history of observations and actions to predict
    the next observation.

    However, it always has observations up to x_t and actions
    up to a_{t-1}.

    The BayesianPredictiveStateRepresentation shares the same
    problem. The main difficulty is that the context in the BPSR
    is defined via Factored Markov Chains.

    The context in a FMC is the observation-action history from time
    t-D to time t. It is necessary to have the context in the FMC in
    order to find the right node in the context tree.

 */

#ifdef MAKE_MAIN

#include <cstdio>
#include <cstdlib>
#include <string>
#include <vector>
#include "Demonstrations.h"
#include "DiscreteChain.h"
#include "DiscreteChain.h"
#include "DoubleLoop.h"
#include "Grid.h"
#include "Gridworld.h"
#include "InventoryManagement.h"
#include "InventoryManagement.h"
#include "MersenneTwister.h"
#include "OneDMaze.h"
#include "OptimisticTask.h"
#include "RandomMDP.h"
#include "RepresentativeStateModel.h"
#include "RiverSwim.h"
#include "ValueIteration.h"

real TestRandomMDP(int n_states, int n_samples);
real TestChainMDP(int n_states, int n_samples);

int main(int argc, char** argv) {
  logmsg("Random MDP\n");
  real random_mdp_error = TestRandomMDP(32, 16);
  printf("\n\n");
  logmsg("Chain MDP\n");
  real chain_mdp_error = TestChainMDP(100, 100);
  logmsg("Done\n");
  return 0;
}

real TestRandomMDP(int n_states, int n_samples) {
  uint n_actions = 2;
  real randomness = 0.1;
  real step_value = -0.1;
  real pit_value = -1;
  real goal_value = 1.0;
  real discount_factor = 0.95;
  real accuracy = 1e-9;
  MersenneTwisterRNG rng;
  RandomMDP random_mdp(n_actions, n_states, randomness, step_value, pit_value,
                       goal_value, &rng);
  Gridworld gridworld("/home/olethros/projects/beliefbox/dat/maze5", randomness,
                      pit_value, goal_value, step_value);

  InventoryManagement inventory(n_states, 32, 0.1, 0.1);

  real total_error = 0;

  DiscreteEnvironment& environment = inventory;  // random_mdp;//gridworld;
  printf("%f\n", gridworld.getExpectedReward(0, 0));
  printf("%f\n", environment.getExpectedReward(0, 0));
  n_states = environment.getNStates();
  DiscreteMDP* mdp = environment.getMDP();
#if 0
    RepresentativeStateModel<DiscreteMDP, int, int> representative_model(discount_factor, accuracy, *mdp, n_samples, n_actions);
#else
  RepresentativeStateModel<DiscreteEnvironment, int, int> representative_model(
      discount_factor, accuracy, environment, n_samples, n_actions);
#endif

#if 1
  ValueIteration VI(mdp, discount_factor);
  VI.ComputeStateValuesStandard(accuracy);

  representative_model.ComputeStateValues();

  for (int i = 0; i < n_states; ++i) {
    real V = VI.getValue(i);
    real V_approx = representative_model.getValue(i);
    printf("%f %f # state-value\n", V, V_approx);
    total_error += abs(V - V_approx);
  }

  logmsg("state-action values\n");
  for (int i = 0; i < n_states; ++i) {
    for (int a = 0; a < n_actions; ++a) {
      real V = VI.getValue(i, a);
      real V_approx = representative_model.getValue(i, a);
      printf("%d %d %f %f %f\n", i, a, V, V_approx,
             mdp->getExpectedReward(i, a));
    }
  }
#endif

  delete mdp;
  return total_error;
}

real TestChainMDP(int n_states, int n_samples) {
  real discount_factor = 0.95;
  real accuracy = 1e-6;
  MersenneTwisterRNG rng;

  logmsg("Creating chain\n");
  DiscreteChain chain(n_states);
  DiscreteEnvironment& environment = chain;

  logmsg("Building model\n");
  DiscreteMDP* mdp = chain.getMDP();
  DiscreteMDP& rmdp = *mdp;
  logmsg("Creating Representative States\n");
  RepresentativeStateModel<DiscreteMDP, int, int> representative_model(
      discount_factor, accuracy, rmdp, (uint)n_samples,
      (uint)environment.getNActions());

  logmsg("Value iteration\n");
  fflush(stdout);
  ValueIteration VI(mdp, discount_factor);
  // VI.ComputeStateValues(accuracy);
  VI.ComputeStateValuesStandard(accuracy);

  logmsg("Approximate VI\n");
  representative_model.ComputeStateValues();

  real total_error = 0;
  logmsg("state values\n");
  for (int i = 0; i < n_states; ++i) {
    real V = VI.getValue(i);
    real V_approx = representative_model.getValue(i);
    printf("%f %f # state-value\n", V, V_approx);
    total_error += abs(V - V_approx);
  }

  logmsg("state-action values\n");
  for (int i = 0; i < n_states; ++i) {
    for (int a = 0; a < (int)environment.getNActions(); ++a) {
      real V = VI.getValue(i, a);
      real V_approx = representative_model.getValue(i, a);
      printf("%d %d %f %f %f\n", i, a, V, V_approx,
             mdp->getExpectedReward(i, a));
    }
  }

  delete mdp;
  return total_error;
}

#endif
