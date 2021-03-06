/* -*- Mode: c++;  -*- */
/*VER: $Id: MarkovChain.h,v 1.7 2006/08/17 05:35:00 olethros Exp $*/
// copyright (c) 2004 by Christos Dimitrakakis <dimitrak@idiap.ch>
/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "BayesianMarkovChain.h"
#include "DenseMarkovChain.h"
#include "Distribution.h"
#include "Random.h"
#include "SparseMarkovChain.h"

// BUG: It appeasr that the prior does not affect this chain.

BayesianMarkovChain::BayesianMarkovChain(int n_states, int n_models, real prior,
                                         bool dense) {
  this->n_models = n_models;
  this->n_states = n_states;

  fprintf(stderr, "Making Bayesian markov chain with %d states and %d models\n",
          n_states, n_models);

  n_observations = 0;

  mc.resize(n_models);
  log_prior.resize(n_models);

  Pr.Resize(n_models);
  Pr_next.Resize(n_states);

  for (int i = 0; i < n_states; ++i) {
    Pr_next[i] = 1.0 / (real)n_states;
  }

  real sum = 0.0;
  for (int i = 0; i < n_models; ++i) {
    // Pr[i] = 0.5; // (1.0 + real(i));
    // Pr[i] = pow((real) n_states, (real) (-i));
    Pr[i] = pow(prior, (real)i);
    sum += Pr[i];
    if (dense) {
      mc[i] = new DenseMarkovChain(n_states, i);
    } else {
      mc[i] = new SparseMarkovChain(n_states, i);
    }
    mc[i]->setThreshold(0.5);
  }
  for (int i = 0; i < n_models; ++i) {
    Pr[i] /= sum;
    log_prior[i] = log(Pr[i]);
  }
}

BayesianMarkovChain::~BayesianMarkovChain() {
  // printf("# Killing BMC\n");
  for (int i = 0; i < n_models; ++i) {
    // printf("%f ", Pr[i]);
    delete mc[i];
  }
  // printf("\n");
}

void BayesianMarkovChain::Reset() {
  for (int i = 0; i < n_models; ++i) {
    mc[i]->Reset();
  }
}

/// Get the probability of the current state.
void BayesianMarkovChain::ObserveNextState(int state) {
  real log_sum = LOG_ZERO;

  for (int i = 0; i < n_models; ++i) {
    log_prior[i] += log(mc[i]->NextStateProbability(state));
    log_sum = logAdd(log_prior[i], log_sum);
  }

  for (int i = 0; i < n_models; ++i) {
    mc[i]->ObserveNextState(state);
    log_prior[i] -= log_sum;
    Pr[i] = exp(log_prior[i]);
  }
  n_observations++;
}

/// Get the probability of the next state
real BayesianMarkovChain::NextStateProbability(int state) {
#if 0
    real log_sum = LOG_ZERO;
    for (int i=0; i<n_models; ++i) {
        log_sum = logAdd(log_sum, Pr[i] + log(mc[i]->NextStateProbability(state)));
    }
#else
  int top_model = std::min(n_models - 1, n_observations);
  real sum = 0.0;
  for (int i = 0; i <= top_model; ++i) {
    sum += Pr[i] * mc[i]->NextStateProbability(state);
  }
  return sum;
#endif
}

/// Generate the next state.
///
/// We are flattening the hierarchical distribution to a simple
/// multinomial.  This allows us to more accurately generate random
/// samples (does it ?)
///
/// Side-effects: Changes the current state.
int BayesianMarkovChain::generate() {
  int i = predict();
  for (int j = 0; j < n_models; ++j) {
    mc[j]->PushState(i);
  }
  return i;
}

/// Generate the next state.
///
/// We are flattening the hierarchical distribution to a simple
/// multinomial.
///
int BayesianMarkovChain::predict() {
  for (int i = 0; i < n_states; ++i) {
    Pr_next[i] = NextStateProbability(i);
  }
  return ArgMax(&Pr_next);
}

void BayesianMarkovChain::ShowTransitions() {
  for (int j = 0; j < n_models; ++j) {
    mc[j]->ShowTransitions();
  }
}
