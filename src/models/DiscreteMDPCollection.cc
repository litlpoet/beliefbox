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

#include "DiscreteMDPCollection.h"
#include "MultinomialDistribution.h"

/// Make a (random) collection.
DiscreteMDPCollection::DiscreteMDPCollection(int n_aggregates, int n_states,
                                             int n_actions)
    : MDPModel(n_states, n_actions) {
  mdp_dbg(
      "Creating DiscreteMDPCollection of %d aggregates, from %d states and %d "
      "actions\n",
      n_aggregates, n_states, n_actions);
  P.resize(n_aggregates);
  A.resize(n_aggregates);
  for (int i = 0; i < n_aggregates; i++) {
    P[i] = 1.0 / (real)n_aggregates;
    if (i == 0) {
      A[i] = new DiscreteMDPCounts(n_states, n_actions);
    } else {
      int n = (int)floor(log(n_states) / log(2));
      // make sure 1 < n_sets < n_states
      int n_sets = 1 + (n_states >> (((i - 1) % n) + 1));  /// warning
      mdp_dbg("Collection size: %d / %d\n", n_sets, n_states);
      A[i] = new DiscreteMDPAggregate(n_states, n_sets, n_actions);
    }
  }
}

/// Make a collection based on a grid world
DiscreteMDPCollection::DiscreteMDPCollection(Gridworld& gridworld,
                                             int n_aggregates, int n_states,
                                             int n_actions)
    : MDPModel(n_states, n_actions) {
  mdp_dbg(
      "Creating DiscreteMDPCollection of %d aggregates, from %d states and %d "
      "actions\n",
      n_aggregates, n_states, n_actions);
  P.resize(n_aggregates);
  A.resize(n_aggregates);
  for (int i = 0; i < n_aggregates; i++) {
    P[i] = 1.0 / (real)n_aggregates;
    if (i == 0) {
      A[i] = new DiscreteMDPCounts(n_states, n_actions);
    } else {
      int n = (int)floor(log(n_states) / log(2));
      // make sure 1 < n_sets < n_states
      int n_sets = 1 + (n_states >> (((i - 1) % n) + 1));
      mdp_dbg("Collection size: %d / %d\n", n_sets, n_states);
      A[i] = new DiscreteMDPAggregate(gridworld, n_states, n_sets, n_actions);
    }
  }
}

DiscreteMDPCollection::~DiscreteMDPCollection() {
  for (uint i = 0; i < A.size(); i++) {
    delete A[i];
  }
}

void DiscreteMDPCollection::AddTransition(int s, int a, real r, int s2) {
  // update top-level model
  real Z = 0.0;
  for (uint i = 0; i < A.size(); ++i) {
    // real Pi =  P[i];
    real Psi = A[i]->getTransitionProbability(s, a, s2);
    P[i] *= Psi;
    Z += P[i];  // post[i];
  }

  // normalise
  if (Z > 0.0) {
    real invZ = 1.0 / Z;
    for (uint i = 0; i < A.size(); ++i) {
      P[i] *= invZ;
    }
  } else {
    Swarning("normalisation failed\n");
    for (uint i = 0; i < A.size(); ++i) {
      P[i] = 1.0 / (real)A.size();
    }
  }

  // update models
  // printf ("P =");
  for (uint i = 0; i < A.size(); ++i) {
    // printf (" %f", P[i]);
    A[i]->AddTransition(s, a, r, s2);
  }
  // printf("\n");
}
real DiscreteMDPCollection::GenerateReward(int s, int a) const {
  int i = DiscreteDistribution::generate(P);
  return A[i]->GenerateReward(s, a);
}
int DiscreteMDPCollection::GenerateTransition(int s, int a) const {
  int i = DiscreteDistribution::generate(P);
  return A[i]->GenerateTransition(s, a);
}
real DiscreteMDPCollection::getTransitionProbability(int s, int a,
                                                     int s2) const {
  real p = 0.0;
  for (uint i = 0; i < A.size(); ++i) {
    mdp_dbg(" A_%d\n", i);
    real Pi = P[i];
    real Psi = A[i]->getTransitionProbability(s, a, s2);
    // printf (" -- %f * %f = %f\n", Pi, Psi, Pi*Psi);
    p += Pi * Psi;
  }
  // printf ("->P = %f\n", p);
  return p;
}
real DiscreteMDPCollection::getExpectedReward(int s, int a) const {
  real E = 0.0;
  for (uint i = 0; i < A.size(); ++i) {
    E += P[i] * A[i]->getExpectedReward(s, a);
  }
  return E;
}
void DiscreteMDPCollection::Reset() {
  // printf ("P =");
  for (uint i = 0; i < A.size(); ++i) {
    // printf (" %f", P[i]);
    A[i]->Reset();
  }
  // printf("\n");
}

int DiscreteMDPCollection::get_n_models() { return P.size(); }
std::vector<real>& DiscreteMDPCollection::GetModelProbabilities() { return P; }
std::vector<DiscreteMDPCounts*>& DiscreteMDPCollection::GetModels() {
  return A;
}
