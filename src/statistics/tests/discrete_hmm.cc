/* -*- Mode: C++; -*- */
/* VER: $Id: Distribution.h,v 1.3 2006/11/06 15:48:53 cdimitrakakis Exp
 * cdimitrakakis $*/
// copyright (c) 2006 by Christos Dimitrakakis <christos.dimitrakakis@gmail.com>
/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifdef MAKE_MAIN

#include "DiscreteHiddenMarkovModel.h"
#include "CumulativeStats.h"
#include "Dirichlet.h"
#include "DiscreteHiddenMarkovModelEM.h"
#include "DiscreteHiddenMarkovModelOnlineEM.h"
#include "DiscreteHiddenMarkovModelPF.h"
#include "Matrix.h"
#include "MersenneTwister.h"
#include "Random.h"
#include "RandomDevice.h"

template <typename T>
void PredictAndAdapt(T& model, int x, int t, CumulativeStats& stats) {
  int prediction = ArgMax(model.getPrediction());
  real loss = 1;
  if (prediction == x) {
    loss = 0;
  }
  stats.SetValue(t, loss);
  model.Observe(x);
}

void TestBelief(DiscreteHiddenMarkovModel* hmm, int T, real threshold,
                real stationarity, int n_particles,
                CumulativeStats& state_stats,
                CumulativeStats& observation_stats, CumulativeStats& pf_stats,
                CumulativeStats& pf_rep_stats, CumulativeStats& pf_rep_ex_stats,
                CumulativeStats& pf_mix_stats,
                CumulativeStats& pf_rep_mix_stats,
                CumulativeStats& pf_rep_ex_mix_stats, Vector& model_stats,
                Vector& rep_model_stats, Vector& rep_ex_model_stats) {
  RandomDevice rng(false);

  DiscreteHiddenMarkovModelStateBelief hmm_belief_state(hmm);
  DiscreteHiddenMarkovModelPF hmm_pf(threshold, stationarity, hmm->getNStates(),
                                     hmm->getNObservations(), n_particles);
  DiscreteHiddenMarkovModelPF_ISReplaceLowest hmm_rep_pf(
      threshold, stationarity, hmm->getNStates(), hmm->getNObservations(),
      n_particles);
  // DiscreteHiddenMarkovModelPF_ISReplaceLowestExact hmm_rep_ex_pf(threshold,
  // stationarity, hmm->getNStates(), hmm->getNObservations(), n_particles);
  DiscreteHiddenMarkovModelOnlineEM hmm_online_em(hmm->getNStates(),
                                                  hmm->getNObservations());

  DiscreteHiddenMarkovModelEM hmm_em(hmm->getNStates(), hmm->getNObservations(),
                                     stationarity, &rng, 1);

  DiscreteHiddenMarkovModelPF_ISReplaceLowestDirichlet hmm_pf_rep_mixture(
      threshold, stationarity, hmm->getNStates(), hmm->getNObservations(),
      n_particles);
  DiscreteHiddenMarkovModelPF_ISReplaceLowestDirichletExact
      hmm_pf_rep_ex_mixture(threshold, stationarity, hmm->getNStates(),
                            hmm->getNObservations(), n_particles);

  int max_states = 8;  // Max(16, 2 * hmm->getNStates())
  model_stats.Resize(max_states);
  rep_model_stats.Resize(max_states);
  for (int i = 0; i < max_states; ++i) {
    model_stats[i] = 0;
    rep_model_stats[i] = 0;
  }

  // DHMM_PF_Mixture<DiscreteHiddenMarkovModelPF_ISReplaceLowest>
  // hmm_pf_mixture(threshold, stationarity, hmm->getNObservations(),
  // n_particles, max_states);

  // DHMM_PF_Mixture<DiscreteHiddenMarkovModelPF_ReplaceLowest>
  // hmm_pf_rep_mixture(threshold, stationarity, hmm->getNObservations(),
  // n_particles, max_states);

  // DHMM_PF_Mixture<DiscreteHiddenMarkovModelPF_ReplaceLowestExact>
  // hmm_pf_rep_ex_mixture(threshold, stationarity, hmm->getNObservations(),
  // n_particles, max_states);

  for (int t = 0; t < T; ++t) {
    // generate next observation
    int x = hmm->generate();
    // int s = hmm->getCurrentState();

    // --- belief state given true model ---
    PredictAndAdapt(hmm_belief_state, x, t, observation_stats);

#if 0        
        // --- grid particle filter ---
        PredictAndAdapt(hmm_pf, x, t, pf_stats);

        // --- particle filter with replacement ---
        PredictAndAdapt(hmm_rep_pf, x, t, pf_rep_stats);
#endif
    // --- Online EM ---
    PredictAndAdapt(hmm_online_em, x, t, pf_rep_ex_stats);

    // --- EM ---
    PredictAndAdapt(hmm_em, x, t, pf_mix_stats);
#if 0
        // --- particle filter mixture with replacement---
        PredictAndAdapt(hmm_pf_rep_mixture, x, t, pf_rep_mix_stats);

        // --- particle filter mixture with replacement exact belief---
        PredictAndAdapt(hmm_pf_rep_ex_mixture, x, t, pf_rep_ex_mix_stats);
#endif
  }

// add mixture statistics
//    model_stats += hmm_pf_mixture.getWeights();
//    rep_model_stats += hmm_pf_rep_mixture.getWeights();
//    rep_ex_model_stats += hmm_pf_rep_ex_mixture.getWeights();

#if 1
  printf("## True HMM\n");
  hmm->Show();
#endif
}

int main(int argc, char** argv) {
  if (argc != 7) {
    fprintf(stderr,
            "Usage: discrete_hmm n_states n_observations stationarity "
            "n_particles T n_iter\n");
    return -1;
  }
  int n_states = atoi(argv[1]);
  if (n_states <= 0) {
    fprintf(stderr, "Invalid number of states %d\n", n_states);
  }

  int n_observations = atoi(argv[2]);
  if (n_observations <= 0) {
    fprintf(stderr, "Invalid number of states %d\n", n_observations);
  }

  real stationarity = atof(argv[3]);
  if (stationarity < 0 || stationarity > 1) {
    fprintf(stderr, "Invalid stationarity %f\n", stationarity);
  }

  int n_particles = atoi(argv[4]);
  if (n_particles <= 0) {
    fprintf(stderr, "Invalid n_particles %d\n", n_particles);
  }

  int T = atoi(argv[5]);
  if (T <= 0) {
    fprintf(stderr, "Invalid T %d\n", T);
  }

  int n_iter = atoi(argv[6]);
  if (n_iter <= 0) {
    fprintf(stderr, "Invalid n_iter %d\n", n_iter);
  }

  real threshold = 0.5;  // threshold for the prior in the estimated HMMs.

  Vector x(10);
  Vector y = exp(x);

  CumulativeStats state_stats(T, n_iter);
  CumulativeStats observation_stats(T, n_iter);
  CumulativeStats pf_stats(T, n_iter);
  CumulativeStats pf_rep_stats(T, n_iter);
  CumulativeStats pf_rep_ex_stats(T, n_iter);
  CumulativeStats pf_mix_stats(T, n_iter);
  CumulativeStats pf_rep_mix_stats(T, n_iter);
  CumulativeStats pf_rep_ex_mix_stats(T, n_iter);
  Vector mixture_statistics(8);
  Vector rep_mixture_statistics(8);
  Vector rep_ex_mixture_statistics(8);

  // use this for high quality random generation
  RandomDevice random_device(false);

  // use this for consistent experiments
  MersenneTwisterRNG fixed_mersenne_twister;
  fixed_mersenne_twister.manualSeed(0xDEADBEEF);

  for (int i = 0; i < n_iter; ++i) {
    real true_stationarity = 0.5 + 0.5 * urandom();
    state_stats.SetSequence(i);
    observation_stats.SetSequence(i);
    pf_stats.SetSequence(i);
    pf_rep_stats.SetSequence(i);
    pf_rep_ex_stats.SetSequence(i);
    pf_mix_stats.SetSequence(i);
    pf_rep_mix_stats.SetSequence(i);
    pf_rep_ex_mix_stats.SetSequence(i);
    fprintf(stderr, "Iter: %d / %d\n", i + 1, n_iter);
    DiscreteHiddenMarkovModel* hmm = MakeRandomDiscreteHMM(
        n_states, n_observations, true_stationarity, &fixed_mersenne_twister);
    TestBelief(hmm, T, threshold, stationarity, n_particles, state_stats,
               observation_stats, pf_stats, pf_rep_stats, pf_rep_ex_stats,
               pf_mix_stats, pf_rep_mix_stats, pf_rep_ex_mix_stats,
               mixture_statistics, rep_mixture_statistics,
               rep_ex_mixture_statistics);
    delete hmm;
  }

  real percentile = 0.1;

  // Matrix M = pf_stats.C;
  CumulativeStats pf_to_mix_stats(pf_stats.C - pf_mix_stats.C);
  CumulativeStats pf_to_rep_stats(pf_stats.C - pf_rep_stats.C);

  observation_stats.Sort();
  Vector hmm_mean = observation_stats.Mean();
  Vector hmm_top = observation_stats.TopPercentile(percentile);
  Vector hmm_bottom = observation_stats.BottomPercentile(percentile);

  pf_stats.Sort();
  Vector pf_mean = pf_stats.Mean();
  Vector pf_top = pf_stats.TopPercentile(percentile);
  Vector pf_bottom = pf_stats.BottomPercentile(percentile);

  pf_rep_stats.Sort();
  Vector pf_rep_mean = pf_rep_stats.Mean();
  Vector pf_rep_top = pf_rep_stats.TopPercentile(percentile);
  Vector pf_rep_bottom = pf_rep_stats.BottomPercentile(percentile);

  pf_rep_ex_stats.Sort();  // Online EM
  Vector pf_rep_ex_mean = pf_rep_ex_stats.Mean();
  Vector pf_rep_ex_top = pf_rep_ex_stats.TopPercentile(percentile);
  Vector pf_rep_ex_bottom = pf_rep_ex_stats.BottomPercentile(percentile);

  pf_mix_stats.Sort();  // EM
  Vector pf_mix_mean = pf_mix_stats.Mean();
  Vector pf_mix_top = pf_mix_stats.TopPercentile(percentile);
  Vector pf_mix_bottom = pf_mix_stats.BottomPercentile(percentile);

  pf_rep_mix_stats.Sort();
  Vector pf_rep_mix_mean = pf_rep_mix_stats.Mean();
  Vector pf_rep_mix_top = pf_rep_mix_stats.TopPercentile(percentile);
  Vector pf_rep_mix_bottom = pf_rep_mix_stats.BottomPercentile(percentile);

  pf_rep_ex_mix_stats.Sort();
  Vector pf_rep_ex_mix_mean = pf_rep_ex_mix_stats.Mean();
  Vector pf_rep_ex_mix_top = pf_rep_ex_mix_stats.TopPercentile(percentile);
  Vector pf_rep_ex_mix_bottom =
      pf_rep_ex_mix_stats.BottomPercentile(percentile);

  for (int t = 0; t < T; ++t) {
    printf(
        "%f %f %f %f %f %f %f %f %f %f %f %f %f %f %f %f %f %f %f %f %f # "
        "loss\n",
        hmm_bottom[t], hmm_mean[t], hmm_top[t], pf_bottom[t], pf_mean[t],
        pf_top[t], pf_rep_bottom[t], pf_rep_mean[t], pf_rep_top[t],
        pf_rep_ex_bottom[t], pf_rep_ex_mean[t], pf_rep_ex_top[t],  // EM
        pf_mix_bottom[t], pf_mix_mean[t], pf_mix_top[t],           // MIX
        pf_rep_mix_bottom[t], pf_rep_mix_mean[t], pf_rep_mix_top[t],
        pf_rep_ex_mix_bottom[t], pf_rep_ex_mix_mean[t], pf_rep_ex_mix_top[t]);
  }

  pf_to_mix_stats.Sort();
  Vector pf_to_mix_mean = pf_to_mix_stats.Mean();
  Vector pf_to_mix_top = pf_to_mix_stats.TopPercentile(percentile);
  Vector pf_to_mix_bottom = pf_to_mix_stats.BottomPercentile(percentile);
  for (int t = 0; t < T; ++t) {
    printf("%f %f %f # pf_to_mix\n", pf_to_mix_bottom[t], pf_to_mix_mean[t],
           pf_to_mix_top[t]);
  }

  pf_to_rep_stats.Sort();
  Vector pf_to_rep_mean = pf_to_rep_stats.Mean();
  Vector pf_to_rep_top = pf_to_rep_stats.TopPercentile(percentile);
  Vector pf_to_rep_bottom = pf_to_rep_stats.BottomPercentile(percentile);
  for (int t = 0; t < T; ++t) {
    printf("%f %f %f # pf_to_rep\n", pf_to_rep_bottom[t], pf_to_rep_mean[t],
           pf_to_rep_top[t]);
  }

  mixture_statistics /= (real)n_iter;
  for (int k = 0; k < mixture_statistics.Size(); ++k) {
    printf("%f ", mixture_statistics[k]);
  }
  printf(" # mixture\n");

  rep_mixture_statistics /= (real)n_iter;
  for (int k = 0; k < rep_mixture_statistics.Size(); ++k) {
    printf("%f ", rep_mixture_statistics[k]);
  }
  printf(" # rep_mixture\n");

  rep_ex_mixture_statistics /= (real)n_iter;
  for (int k = 0; k < rep_ex_mixture_statistics.Size(); ++k) {
    printf("%f ", rep_ex_mixture_statistics[k]);
  }
  printf(" # rep_ex_mixture\n");

  return 0;
}

#endif
