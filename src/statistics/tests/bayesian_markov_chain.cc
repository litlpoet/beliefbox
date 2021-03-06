/* -*- Mode: C++; -*- */
/* VER: $Id: Distribution.h,v 1.3 2006/11/06 15:48:53 cdimitrakakis Exp
 * cdimitrakakis $*/
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

#ifdef MAKE_MAIN
#include "DiscreteHiddenMarkovModel.h"
#include "BVMM.h"
#include "BayesianMarkovChain.h"
#include "DenseMarkovChain.h"
#include "Dirichlet.h"
#include "DiscreteHiddenMarkovModelEM.h"
#include "DiscreteHiddenMarkovModelPF.h"
#include "EasyClock.h"
#include "MersenneTwister.h"
#include "Random.h"
#include "RandomDevice.h"
#include "SparseMarkovChain.h"

#include <ctime>
struct ErrorStatistics {
  std::vector<real> loss;
  ErrorStatistics(int T) : loss(T) {}
};

void print_result(const char* fname, ErrorStatistics& error) {
  FILE* f = fopen(fname, "w");
  if (!f) return;

  int T = error.loss.size();
  for (int t = 0; t < T; ++t) {
    fprintf(f, " %f", error.loss[t]);
  }
  fprintf(f, "\n");
  fclose(f);
}

int main(int argc, char** argv) {
  int n_observations = 4;
  int max_states = 4;
  int n_mc_states = 4;
  float prior = 0.5f;
  int T = 100;
  int n_iter = 100;
  // real stationarity = 0.99;

  setRandomSeed(98234239846);  // time(NULL));

  if (argc > 1) {
    T = atoi(argv[1]);
  }

  if (argc > 2) {
    n_iter = atoi(argv[2]);
  }

  if (argc > 3) {
    n_observations = atoi(argv[3]);
  }

  if (argc > 4) {
    max_states = atoi(argv[4]);
  }

  if (argc > 5) {
    n_mc_states = atoi(argv[5]);
  }

  double oracle_time = 0;
  double bmc_time = 0;
  double bpsr_time = 0;
  double hmm_pf_time = 0;
  double hmm_is_pf_time = 0;
  double hmm_em_time = 0;
  double hmm_pf_mix_time = 0;

  double initial_time = GetCPU();
  double elapsed_time = 0;

  ErrorStatistics oracle_error(T);
  ErrorStatistics bmc_error(T);
  ErrorStatistics bpsr_error(T);
  ErrorStatistics hmm_pf_error(T);
  ErrorStatistics hmm_is_pf_error(T);
  ErrorStatistics hmm_em_error(T);
  ErrorStatistics hmm_pf_mix_error(T);
  // RandomDevice random_device(false);
  MersenneTwisterRNG random_device;
  random_device.manualSeed(209482335097);

  for (int iter = 0; iter < n_iter; iter++) {
    // real stationarity = 0.9;
    real true_stationarity = 0.5 + 0.5 * urandom();
    double remaining_time = (real)(n_iter - iter) * elapsed_time / (real)iter;
    printf("# iter: %d, %.1f running, %.1f remaining\n", iter, elapsed_time,
           remaining_time);

    // logmsg ("Making Bayesian Markov chain\n");
    // our model for the chains
    bool dense = false;
    BayesianMarkovChain bmc(n_observations, 1 + max_states, prior, dense);
    BVMM bpsr(n_observations, 1 + max_states, prior, dense);

    // logmsg ("Making Markov chain\n");
    // the actual model that generates the data
    DiscreteHiddenMarkovModel* hmm = MakeRandomDiscreteHMM(
        n_mc_states, n_observations, true_stationarity, &random_device);
    DiscreteHiddenMarkovModelStateBelief oracle(hmm);

    real hmm_threshold = 0.5;
    real hmm_stationarity = 0.5;
    int hmm_particles = 128;
    DiscreteHiddenMarkovModelPF hmm_pf(hmm_threshold, hmm_stationarity,
                                       n_mc_states, n_observations,
                                       hmm_particles);
    DiscreteHiddenMarkovModelPF_ISReplaceLowest hmm_is_pf(
        hmm_threshold, hmm_stationarity, n_mc_states, n_observations,
        hmm_particles);
    DiscreteHiddenMarkovModelEM hmm_em(n_mc_states, n_observations,
                                       hmm_stationarity, &random_device, 1);
    // DHMM_PF_Mixture<DiscreteHiddenMarkovModelPF> hmm_pf(hmm_threshold,
    // hmm_stationarity, n_observations, hmm_particles, 2 * n_mc_states);
    // DiscreteHiddenMarkovModelEM hmm_pf_mix(n_mc_states, n_observations,
    // hmm_stationarity, &random_device, 1);

    ////DHMM_PF_Mixture<DiscreteHiddenMarkovModelPF_ReplaceLowest>
    ///hmm_pf_mix(hmm_threshold, hmm_stationarity, n_observations,
    ///hmm_particles, 2 * n_mc_states);

    // logmsg ("Observing chain outputs\n");
    oracle.Reset();
    bmc.Reset();
    bpsr.Reset();
    hmm->Reset();
    hmm_pf.Reset();
    hmm_is_pf.Reset();
    hmm_em.Reset();
    // hmm_pf_mix.Reset();

    DiscreteHiddenMarkovModel* estimated_hmm_ptr = MakeRandomDiscreteHMM(
        hmm->getNStates(), hmm->getNObservations(), 0.5, &random_device);
    DiscreteHiddenMarkovModel& estimated_hmm = *estimated_hmm_ptr;
    ExpectationMaximisation<DiscreteHiddenMarkovModel, int> EM_algo(
        estimated_hmm);
    DiscreteHiddenMarkovModelStateBelief oracle_em(estimated_hmm_ptr);

    std::vector<int> data(T);
    for (int t = 0; t < T; ++t) {
      data[t] = hmm->generate();
      EM_algo.Observe(data[t]);
    }

    estimated_hmm_ptr->Show();
    int max_iter = 1000;  // 256; //16384; //4096; //256;
    real log_likelihood = LOG_ZERO;
    for (int iter = 0; iter < max_iter; ++iter) {
      real log_likelihood2 = EM_algo.Iterate(1);
      printf("%f # log likelihood \n", log_likelihood2);
      if (0) {  // log_likelihood2 - log_likelihood < 0) {
        break;
      }
      log_likelihood = log_likelihood2;
    }
    estimated_hmm_ptr->Show();
    hmm->Show();
    printf("# HMM estimation complete. Online prediction starting\n");
    for (int t = 0; t < T; ++t) {
      int observation = data[t];

      int oracle_prediction = oracle.predict();
      if (oracle_prediction != observation) {
        oracle_error.loss[t] += 1.0;
      }

#if 0
            int bmc_prediction = bmc.predict();
            if (bmc_prediction != observation) {
                bmc_error.loss[t] += 1.0;
            }

            int bpsr_prediction = bpsr.predict();
            if (bpsr_prediction != observation) {
                bpsr_error.loss[t] += 1.0;
            }
#endif
      int hmm_pf_prediction = hmm_pf.predict();
      if (hmm_pf_prediction != observation) {
        hmm_pf_error.loss[t] += 1.0;
      }

#if 0
            int hmm_is_pf_prediction = hmm_is_pf.predict();
            if (hmm_is_pf_prediction != observation) {
                hmm_is_pf_error.loss[t] += 1.0;
            }
#endif
      int hmm_em_prediction = hmm_em.predict();
      if (hmm_em_prediction != observation) {
        hmm_em_error.loss[t] += 1.0;
      }
#if 1
      int hmm_pf_mix_prediction = oracle_em.predict();
      if (hmm_pf_mix_prediction != observation) {
        hmm_pf_mix_error.loss[t] += 1.0;
      }
#endif
      double start_time, end_time;

      start_time = GetCPU();
      oracle.Observe(observation);
      end_time = GetCPU();
      oracle_time += end_time - start_time;

#if 0
            start_time = GetCPU();
            bmc.ObserveNextState(observation);
            end_time = GetCPU();
            bmc_time += end_time - start_time;


            start_time = GetCPU();
            bpsr.ObserveNextState(observation);                               
            end_time = GetCPU();
            bpsr_time += end_time - start_time;
#endif
      start_time = GetCPU();
      hmm_pf.Observe(observation);
      end_time = GetCPU();
      hmm_pf_time += end_time - start_time;

#if 0
            start_time = GetCPU();
            hmm_is_pf.Observe(observation);                               
            end_time = GetCPU();
            hmm_is_pf_time += end_time - start_time;
#endif
      start_time = GetCPU();
      hmm_em.Observe(observation);
      end_time = GetCPU();
      hmm_em_time += end_time - start_time;

      start_time = GetCPU();
      oracle_em.Observe(observation);
      end_time = GetCPU();
      hmm_pf_mix_time += end_time - start_time;
    }
    printf("#  Online prediction Complete\n");

    double end_time = GetCPU();
    elapsed_time += end_time - initial_time;
    initial_time = end_time;

    delete hmm;
    delete estimated_hmm_ptr;
  }

  printf(
      "# Time -- Oracle: %f, HMM PF: %f, HMM IS PF: %f, HMM PF EX: %f, EM "
      "ORACLE: %f, BHMC: %f, BPSR: %f\n",
      oracle_time, hmm_pf_time, hmm_is_pf_time, hmm_em_time, hmm_pf_mix_time,
      bmc_time, bpsr_time);

  real inv_iter = 1.0 / (real)n_iter;
  for (int t = 0; t < T; ++t) {
    hmm_pf_error.loss[t] *= inv_iter;
    hmm_is_pf_error.loss[t] *= inv_iter;
    hmm_em_error.loss[t] *= inv_iter;
    hmm_pf_mix_error.loss[t] *= inv_iter;
    oracle_error.loss[t] *= inv_iter;
    bmc_error.loss[t] *= inv_iter;
    bpsr_error.loss[t] *= inv_iter;
  }
  print_result("hmm_pf.error", hmm_pf_error);
  print_result("hmm_is_pf.error", hmm_is_pf_error);
  print_result("hmm_em.error", hmm_em_error);
  print_result("oracle_em.error", hmm_pf_mix_error);
  print_result("oracle.error", oracle_error);
  print_result("bmc.error", bmc_error);
  print_result("bpsr.error", bpsr_error);
}

#endif
