/* -*- Mode: C++; -*- */
// copyright (c) 2014 by Nikolaos Tziortziotis <ntziorzi@gmail.com>
/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifdef MAKE_MAIN

#include "MonteCarloTreeSearch.h"
#include "Grid.h"

#include "Acrobot.h"
#include "Bike.h"
#include "CartPole.h"
#include "MountainCar.h"
#include "Pendulum.h"
#include "PuddleWorld.h"

#include <getopt.h>
#include <stdio.h>
#include <cstring>
#include <vector>

/** Options */
struct Options {
  real gamma;                  ///< discount factor
  char* environment_name;      ///< environment name
  RandomNumberGenerator& rng;  ///< random number generator
  int depth;                   ///< Maximum tree depth
  int horizon;                 ///< Maximum horizon for the rollouts
  int grids;                   ///< Number of grids per state space dimension
  int n_episodes;              ///< number of episodes
  int n_evaluations;           ///< number of evaluations
  Options(RandomNumberGenerator& rng_)
      : gamma(0.999),
        environment_name(NULL),
        rng(rng_),
        depth(1000),
        horizon(1000),
        grids(100),
        n_episodes(1000),
        n_evaluations(10) {}

  void ShowOptions() {
    logmsg("------------------------\n");
    logmsg("Options\n");
    logmsg("========================\n");
    logmsg("Gamma: %f\n", gamma);
    logmsg("Depth: %d\n", depth);
    logmsg("Episode horizon: %d\n", horizon);
    // logmsg("Grids: %d\n", grids);
    logmsg("n_episodes: %d\n", n_episodes);
    logmsg("n_evaluations: %d\n", n_evaluations);
    logmsg("------------------------\n");
  }
};

struct EpisodeStatistics {
  real total_reward;
  real discounted_reward;
  int steps;
  EpisodeStatistics() : total_reward(0.0), discounted_reward(0.0), steps(0) {}
};

/** Run a test */
template <class S, class A>
std::vector<EpisodeStatistics> RunTest(ContinuousStateEnvironment* environment,
                                       Options& options) {
  bool running;
  real reward;

  // EvenGrid
  // discretize(environment->StateLowerBound(),environment->StateUpperBound(),options.grids);

  // Start with a random policy!
  RandomPolicy random_policy(environment->getNActions(), &options.rng);

  // Placeholder for the policy
  AbstractPolicy<Vector, int>& policy = random_policy;

  MonteCarloTreeSearch<S, A> mcts(options.gamma, environment, &options.rng,
                                  policy, options.depth);
  int state_dimension = environment->getNStates();
  Vector S_L = environment->StateLowerBound();
  Vector S_U = environment->StateUpperBound();

  logmsg("State dimension: %d\n", state_dimension);
  logmsg("S_L: ");
  S_L.print(stdout);
  logmsg("S_U: ");
  S_U.print(stdout);

  /// Statistics.
  std::vector<EpisodeStatistics> statistics(options.n_episodes);

  for (int episode = 0; episode < options.n_episodes; ++episode) {
    // printf("Episode = %d\n",episode);
    int step = 0;
    real discounted_reward = 0;
    real total_reward = 0;
    // printf("Episode = %d\n",episode);
    environment->Reset();

    S state = environment->getState();
    // state.print(stdout);
    A action = mcts.SelectAction(state);
    do {
      running = environment->Act(action);
      reward = environment->getReward();

      total_reward += reward;
      discounted_reward += options.gamma * reward;

      state = environment->getState();
      // state.print(stdout);
      action = mcts.SelectAction(state);
      // printf("Action = %d \n",action);
      step++;
      // printf("Step = %d\n",step);
    } while (running && step < options.horizon);
    printf(
        "Episode = %d: Steps = %d, Total Reward = %f, Discounted Reward = %f\n",
        episode, step, total_reward, discounted_reward);

    // printf("Steps = %d\n",step);
    statistics[episode].total_reward = total_reward;
    statistics[episode].discounted_reward = discounted_reward;
    statistics[episode].steps = step;
    // printf("Steps = %d\n",step);
  }

  return statistics;
}

static const char* const help_text =
    "Usage: test [options]\n\
\nOptions:\n\
    --environment:           {MountainCar, Pendulum, Puddle, Bicycle, CartPole, Acrobot}\n\
    --discount:              reward discounting in [0,1]\n\
    --depth:                 maximum tree depth\n\
--grids: number of grids per state space dimension\n\
    --horizon:               maximumn number of steps during rollout\n\
    --n_evaluations:         number of evaluations\n\
    --n_episodes:            number of episodes per evaluation\n\
    --seed:                  seed all the RNGs with this\n\
    --seed_file:             select a binary file to choose seeds from (use in conjunction with --seed to select the n-th seed in the file)\n\
    --Rmax:                  maximum reward value\n\
\n";
int main(int argc, char* argv[]) {
  ulong seed = time(NULL);
  char* seed_filename = 0;

  MersenneTwisterRNG rng;
  Options options(rng);
  {
    // options
    int c;
    int digit_optind = 0;
    while (1) {
      int this_option_optind = optind ? optind : 1;
      int option_index = 0;
      static struct option long_options[] = {
          {"discount", required_argument, 0, 0},  // 0
          {"environment", required_argument, 0, 0},  // 1
          {"depth", required_argument, 0, 0},  // 2
          {"horizon", required_argument, 0, 0},  // 3
          {"grids", required_argument, 0, 0},  // 4
          {"seed", required_argument, 0, 0},  // 5
          {"n_evaluations", required_argument, 0, 0},  // 6
          {"seed_file", required_argument, 0, 0},  // 7
          {0, 0, 0, 0}};
      c = getopt_long(argc, argv, "", long_options, &option_index);
      if (c == -1) break;

      switch (c) {
        case 0:
#if 0
   printf ("option %s (%d)", long_options[option_index].name, option_index);
   if (optarg)
     printf(" with arg %s", optarg);
   printf("\n");
#endif
          switch (option_index) {
            case 0:
              options.gamma = atof(optarg);
              break;
            case 1:
              options.environment_name = optarg;
              break;
            case 2:
              options.depth = atoi(optarg);
              break;
            case 3:
              options.horizon = atoi(optarg);
              break;
            case 4:
              options.grids = atoi(optarg);
              break;
            case 5:
              seed = atoi(optarg);
              break;
            case 6:
              options.n_evaluations = atoi(optarg);
              break;
            case 7:
              seed_filename = optarg;
              break;
            default:
              fprintf(stderr, "Invalid options\n");
              exit(0);
              break;
          }
          break;
        case '0':
        case '1':
        case '2':
          if (digit_optind != 0 && digit_optind != this_option_optind)
            printf("digits occur in two different argv-elements.\n");
          digit_optind = this_option_optind;
          printf("option %c\n", c);
          break;
        default:
          std::cout << help_text;
          exit(-1);
      }
    }
    if (optind < argc) {
      printf("non-option ARGV-elements: ");
      while (optind < argc) {
        printf("%s ", argv[optind++]);
      }
      printf("\n");
    }
  }

  if (seed_filename) {
    RandomNumberFile rnf(seed_filename);
    rnf.manualSeed(seed);
    seed = rnf.random();
  }

  logmsg("seed: %ld\n", seed);
  srand(seed);
  srand48(seed);
  rng.manualSeed(seed);
  setRandomSeed(seed);

  if (!options.environment_name) {
    Serror("Must specify environment\n");
    exit(-1);
  }
  if (options.gamma < 0 || options.gamma > 1) {
    Serror("gamma must be in [0,1]\n");
    exit(-1);
  }
  if (options.horizon < 1) {
    Serror("horizon must be >= 1\n");
    exit(-1);
  }
  if (options.depth < 1) {
    Serror("depth must be >= 1\n");
    exit(-1);
  }
  logmsg("Starting environment %s\n", options.environment_name);
  options.ShowOptions();

  ContinuousStateEnvironment* environment = NULL;

  if (!strcmp(options.environment_name, "MountainCar")) {
    environment = new MountainCar(0.0);
  } else if (!strcmp(options.environment_name, "Pendulum")) {
    environment = new Pendulum();
  } else if (!strcmp(options.environment_name, "PuddleWorld")) {
    environment = new PuddleWorld();
  } else if (!strcmp(options.environment_name, "Bike")) {
    environment = new Bike();
  } else if (!strcmp(options.environment_name, "Acrobot")) {
    environment = new Acrobot();
  } else if (!strcmp(options.environment_name, "CartPole")) {
    environment = new CartPole();
  } else {
    fprintf(stderr, "Unknown environment %s \n", options.environment_name);
  }

  Matrix StatDR(options.n_evaluations, options.n_episodes);
  Matrix StatTR(options.n_evaluations, options.n_episodes);
  Matrix StatSt(options.n_evaluations, options.n_episodes);

  for (int n_runs = 0; n_runs < options.n_evaluations; ++n_runs) {
    std::vector<EpisodeStatistics> statistics =
        RunTest<Vector, int>(environment, options);
    for (uint episodes = 0; episodes < options.n_episodes; episodes++) {
      StatDR(n_runs, episodes) = statistics[episodes].discounted_reward;
      StatTR(n_runs, episodes) = statistics[episodes].total_reward;
      StatSt(n_runs, episodes) = statistics[episodes].steps;
    }
  }
  char buffer[100];

  sprintf(buffer, "MCTS_RESULTS_STEPS_%s", options.environment_name);
  FILE* output = fopen(buffer, "w");
  if (output != NULL) {
    StatDR.print(output);
    StatTR.print(output);
    StatSt.print(output);
  }
  fclose(output);

  return 0;
}

#endif
