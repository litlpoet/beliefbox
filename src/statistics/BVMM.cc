/* -*- Mode: c++;  -*- */
// copyright (c) 2009-2010 by Christos Dimitrakakis
// <christos.dimitrakakis@gmail.com>
/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "BVMM.h"
#include "DenseMarkovChain.h"
#include "Distribution.h"
#include "Matrix.h"
#include "Random.h"
#include "SparseMarkovChain.h"

/** Initialise the models

        This is done as follows:

        The weights are a bit tricky to set.
        Consider that we want to always have the same weight
        for w_1 = P(K = 1) =
 */
BVMM::BVMM(int n_states, int n_models, float prior, bool polya_, bool dense)
    : BayesianMarkovChain(n_states, n_models, prior, dense),
      polya(polya_),
      P_obs(n_models, n_states),
      Lkoi(n_models, n_states),
      weight(n_models) {
  beliefs.resize(n_models);
  for (int i = 0; i < n_models; ++i) {
#if 1
    // for alice, this works well when p = 0.1
    Pr[i] = pow(prior, (real)(i + 1));
    log_prior[i] = ((real)(i + 1)) * log(prior);
#else
    Pr[i] = prior;
    log_prior[i] = log(Pr[i]);
#endif
  }
}

BVMM::~BVMM() {
  // printf("Killing BPSR\n");
}

void BVMM::Reset() {
  for (int i = 0; i < n_models; ++i) {
    mc[i]->Reset();
  }
}

/// Adapt the model given the next state
void BVMM::ObserveNextState(int state) {
  //    Matrix P_obs(n_models, n_states);
  //    Matrix Lkoi(n_models, n_states);

  int top_model = std::min(n_models - 1, n_observations);

  // calculate predictions for each model
  for (int model = 0; model <= top_model; ++model) {
    // std::vector<real> p(n_states);

    for (int j = 0; j < n_states; j++) {
      P_obs(model, j) = mc[model]->NextStateProbability(j);
    }
    // printf("p(%d): ", i);
    if (model == 0) {
      weight[model] = 1;
      for (int j = 0; j < n_states; j++) {
        Lkoi(model, j) = P_obs(model, j);
      }
    } else {
      if (polya) {
        weight[model] = (0.5 + get_belief_param(model)) /
                        (0.5 + get_belief_param(model - 1));
      } else {
        weight[model] = exp(log_prior[model] + get_belief_param(model));
      }
      // so the actual weight of model k is \prod_{i=k+1}^D (1-w_k)
      for (int j = 0; j < n_states; j++) {
        Lkoi(model, j) = weight[model] * P_obs(model, j) +
                         (1.0 - weight[model]) * Lkoi(model - 1, j);
      }
    }
  }

  real p_w = 1.0;
  real p_w_sum = 0;
  for (int model = top_model; model >= 0; model--) {
    Pr[model] = p_w * weight[model];
    p_w *= (1.0 - weight[model]);
    p_w_sum += Pr[model];
    //		printf ("w[%d]=%f, P= %f, P(K >= %d) = %f\n",
    // model, weight[model], Pr[model], model, p_w_sum);
  }

  // real sum_pr_s = 0.0;
  for (int s = 0; s < n_states; ++s) {
    Pr_next[s] = Lkoi(top_model, s);
  }

  // insert new observations
  n_observations++;

  for (int model = 0; model <= top_model; ++model) {
    if (polya) {
      set_belief_param(model, 1.0 + get_belief_param(model));
      real posterior = weight[model] * P_obs(model, state) / Lkoi(model, state);
      set_belief_param(model, log(posterior) - log_prior[model]);
    } else {
      real posterior = weight[model] * P_obs(model, state) / Lkoi(model, state);
      real p = log(posterior) - log_prior[model];
      // printf ("%d %d %f #Weight\n", n_observations, model, posterior);
      set_belief_param(model, p);
    }
  }

  // update expert parameters
  for (int model = 0; model < n_models; ++model) {
    // for (int model=0; model<=top_model; ++ model) {
    mc[model]->ObserveNextState(state);
  }
}

/// Get the probability of the next state
real BVMM::NextStateProbability(int state) {
  // Pr_next /= Pr_next.Sum();
  return Pr_next[state];
}

/// Predict the next state
///
/// We are flattening the hierarchical distribution to a simple
/// multinomial.
///
int BVMM::predict() {
  //    Matrix P_obs(n_models, n_states);
  //    Matrix Lkoi(n_models, n_states);
  int top_model = std::min(n_models - 1, n_observations);

  // calculate predictions for each model
  for (int model = 0; model <= top_model; ++model) {
    std::vector<real> p(n_states);

    for (int state = 0; state < n_states; state++) {
      P_obs(model, state) = mc[model]->NextStateProbability(state);
    }
    // printf("p(%d): ", i);
    if (model == 0) {
      weight[model] = 1;
      for (int state = 0; state < n_states; state++) {
        Lkoi(model, state) = P_obs(model, state);
      }
    } else {
      if (polya) {
        weight[model] = (exp(log_prior[model]) + get_belief_param(model)) /
                        exp(log_prior[model - 1] + get_belief_param(model));
      } else {
        weight[model] = exp(log_prior[model] + get_belief_param(model));
      }
      for (int state = 0; state < n_states; state++) {
        Lkoi(model, state) = weight[model] * P_obs(model, state) +
                             (1.0 - weight[model]) * Lkoi(model - 1, state);
      }
    }
  }

  real p_w = 1.0;
  for (int model = top_model; model >= 0; model--) {
    Pr[model] = p_w * weight[model];
    p_w *= (1.0 - weight[model]);
  }
#if 0
    for (int i=0; i<=top_model; i++) {
        printf ("%f ", Pr[i]);
    }
    printf("#BPSR \n");
#endif

  Pr /= Pr.Sum(0, top_model);
  for (int state = 0; state < n_states; ++state) {
    Pr_next[state] = 0.0;
    for (int model = 0; model <= top_model; model++) {
      Pr_next[state] += Pr[model] * P_obs(model, state);
    }
  }

#if 0
    for (int model=0; model<=top_model; model++) {
        real s = P_obs.RowSum(model);
        if (fabs(s - 1.0) > 0.001) {
            if (polya) {
                fprintf (stderr, "polya ");
            }
            fprintf(stderr, "sum[%d]: %f\n", model, s);
        }
    }
    real sum_pr_s = Pr_next.Sum();
    if (fabs(sum_pr_s - 1.0) > 0.001) {
        if (polya) {
            fprintf (stderr, "polya ");
        }
        fprintf(stderr, "sum2: %f\n", sum_pr_s);
        fprintf(stderr, "model sum: %f\n", Pr.Sum(0, top_model));
        //exit(-1);
    }
#endif

#if 0
    printf ("# BPSR ");
    for (int i=0; i<n_states; ++i) {
        printf ("(%f %f", Pr_next[i], Lkoi(top_model,i));
    }
    printf("\n");
#endif
  return ArgMax(&Pr_next);
  // return DiscreteDistribution::generate(Pr_next);
}

/// Generate the next state.
///
/// We are flattening the hierarchical distribution to a simple
/// multinomial.  This allows us to more accurately generate random
/// samples (!) does it ?
///
/// Side-effects: Changes the current state.
int BVMM::generate() {
  int i = predict();
  for (int j = 0; j < n_models; ++j) {
    mc[j]->PushState(i);
  }
  return i;
}
