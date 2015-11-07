// Copyright (C) 2015 BK

#include <vector>

#include "DiscreteChain.h"
#include "DiscreteMDPCounts.h"
#include "ExplorationPolicy.h"
#include "MersenneTwister.h"
#include "ModelBasedRL.h"
#include "OnlineAlgorithm.h"
#include "SampleBasedRL.h"

struct EpisodeStatistics {
  real total_reward;
  real discounted_reward;
  int steps;
  real mse;
  int n_runs;

  EpisodeStatistics()
      : total_reward(0.0),
        discounted_reward(0.0),
        steps(0),
        mse(0),
        n_runs(0) {}
};

struct Statistics {
  std::vector<EpisodeStatistics> ep_stats;
  std::vector<real> reward;
  std::vector<int> n_runs;
};

Statistics EvaluateAlgorithm(int episode_steps, int n_episodes, uint n_steps,
                             OnlineAlgorithm<int, int>* algorithm,
                             DiscreteEnvironment* environment, real gamma);

int main(int argc, char** argv) {
  ulong seed = time(NULL);

  int n_states = 5;
  int n_actions = 2;

  int episode_steps = -1;  // ignore maximum
  uint n_runs = 3;
  uint n_episodes = 1;
  uint n_steps = 1000;

  real gamma = 1.0;
  real lambda = 0.0;
  real epsilon = 0.01;
  real dirichlet_mass = 0.5;

  enum DiscreteMDPCounts::RewardFamily reward_prior = DiscreteMDPCounts::NORMAL;

  MersenneTwisterRNG mersenne_twister;
  RandomNumberGenerator* rng =
      dynamic_cast<RandomNumberGenerator*>(&mersenne_twister);

  std::cout << "Seed: " << seed << std::endl;
  srand48(seed);
  srand(seed);
  setRandomSeed(seed);
  rng->manualSeed(seed);

  Statistics statics;
  statics.ep_stats.resize(n_episodes);
  statics.reward.resize(n_steps);
  statics.n_runs.resize(n_steps);
  for (uint i = 0; i < n_steps; ++i) {
    statics.reward[i] = 0;
    statics.n_runs[i] = 0;
  }

  for (uint run = 0; run < n_runs; ++run) {
    // Create environment
    DiscreteEnvironment* env = nullptr;
    env = new DiscreteChain(n_states, 0.2, 2.0, 10.0);

    std::cout << "Chain env created: " << env->getNStates() << " states, "
              << env->getNActions() << " actions" << std::endl;

    VFExplorationPolicy* exp_policy = nullptr;  // only for primitive RL
    MDPModel* model = nullptr;                  // only for Bayesian RL
    OnlineAlgorithm<int, int>* algorithm = nullptr;

    // Model-based (Bayesian) RL
    model = new DiscreteMDPCounts(n_states, n_actions, dirichlet_mass,
                                  reward_prior);
    // algorithm =
    //     new ModelBasedRL(n_states, n_actions, gamma, epsilon, model, rng);
    algorithm = new SampleBasedRL(n_states, n_actions, gamma, epsilon, model,
                                  rng, 1, false);

    // Evaluate Algorithms
    std::cout << "Evaluate algorithms: run " << run << std::endl;

    Statistics run_statistics = EvaluateAlgorithm(
        episode_steps, n_episodes, n_steps, algorithm, env, gamma);

    if (statics.ep_stats.size() < run_statistics.ep_stats.size())
      statics.ep_stats.resize(run_statistics.ep_stats.size());

    for (uint i = 0; i < run_statistics.ep_stats.size(); ++i) {
      statics.ep_stats[i].total_reward +=
          run_statistics.ep_stats[i].total_reward;
      statics.ep_stats[i].discounted_reward +=
          run_statistics.ep_stats[i].discounted_reward;
      statics.ep_stats[i].steps += run_statistics.ep_stats[i].steps;
      statics.ep_stats[i].mse += run_statistics.ep_stats[i].mse;
      statics.ep_stats[i].n_runs++;
    }

    for (uint i = 0; i < run_statistics.reward.size(); ++i) {
      statics.reward[i] += run_statistics.reward[i];
      statics.n_runs[i]++;
    }

    if (env) delete env;
    if (exp_policy) delete exp_policy;
    if (model) delete model;
    if (algorithm) delete algorithm;
  }

  // for (uint i = 0; i < statics.ep_stats.size(); ++i) {
  //   statics.ep_stats[i].total_reward /= n_runs;
  //   statics.ep_stats[i].discounted_reward /= n_runs;
  //   statics.ep_stats[i].steps /= n_runs;
  //   statics.ep_stats[i].mse /= n_runs;
  //   std::cout << statics.ep_stats[i].n_runs << " "
  //             << statics.ep_stats[i].total_reward << " "
  //             << statics.ep_stats[i].discounted_reward << " # EPISODE_RETURN"
  //             << std::endl;
  //   std::cout << statics.ep_stats[i].steps << " " << statics.ep_stats[i].mse
  //             << "# MSE" << std::endl;
  // }

  // for (uint i = 0; i < statics.reward.size(); ++i) {
  //   statics.reward[i] /= n_runs;
  //   std::cout << statics.n_runs[i] << " " << statics.reward[i] << " #
  //   INST_PAYOFF"
  //             << std::endl;
  // }

  std::cout << "Done" << std::endl;

  return 0;
}

/*** Evaluate an algorithm

     episode_steps: maximum number of steps per episode.
     If negative, then ignore
     n_steps: maximun number of total steps. If negative, then ignore.
     n_episodes: maximum number of episodes. Cannot be negative.
*/

Statistics EvaluateAlgorithm(int episode_steps, int n_episodes, uint n_steps,
                             OnlineAlgorithm<int, int>* algorithm,
                             DiscreteEnvironment* environment, real gamma) {
  std::cout << "# evaluating..." << environment->Name() << std::endl;

  Statistics statics;
  if (n_episodes > 0) statics.ep_stats.reserve(n_episodes);
  statics.reward.reserve(n_steps);

  real discount = 1.0;
  int current_time = 0;
  environment->Reset();

  std::cout << "(running)" << std::endl;
  int episode = -1;
  bool action_ok = false;
  real total_reward = 0.0;
  real discounted_reward = 0.0;

  for (uint step = 0; step < n_steps; ++step) {
    if (!action_ok) {
      int state = environment->getState();
      real reward = environment->getReward();

      algorithm->Act(reward, state);

      statics.reward.resize(step + 1);
      statics.reward[step] = reward;

      if (episode >= 0) {
        statics.ep_stats[episode].steps++;
        statics.ep_stats[episode].total_reward += reward;
        statics.ep_stats[episode].discounted_reward += discount * reward;
      }

      total_reward += reward;
      discounted_reward += discount * reward;
      discount *= gamma;
      episode++;
      statics.ep_stats.resize(episode + 1);
      statics.ep_stats[episode].total_reward = 0.0;
      statics.ep_stats[episode].discounted_reward = 0.0;
      statics.ep_stats[episode].steps = 0;

      discount = 1.0;
      environment->Reset();
      algorithm->Reset();
      action_ok = true;
      current_time = 0;
      // printf ("# episode %d complete\n", episode);

      if (n_episodes > 0 && episode >= n_episodes) {
        logmsg(" Breaking after %d episodes,  %d steps\n", episode, step);
        break;
      } else {
        statics.ep_stats.resize(episode + 1);
        statics.ep_stats[episode].total_reward = 0.0;
        statics.ep_stats[episode].discounted_reward = 0.0;
        statics.ep_stats[episode].steps = 0;
      }
      step++;
    }

    int state = environment->getState();
    real reward = environment->getReward();
    statics.reward.resize(step + 1);
    statics.reward[step] = reward;

    statics.ep_stats[episode].steps++;
    statics.ep_stats[episode].total_reward += reward;
    statics.ep_stats[episode].discounted_reward += discount * reward;
    total_reward += reward;
    discounted_reward += discount * reward;

    discount *= gamma;

    int action = algorithm->Act(reward, state);

    printf("%d %d %d %f # t-state-action-reward\n", step, state, action,
           reward);

    action_ok = environment->Act(action);
    current_time++;
  }
  printf(" %f %f # RUN_REWARD\n", total_reward, discounted_reward);
  fflush(stdout);

  if (static_cast<int>(statics.ep_stats.size()) != n_episodes)
    statics.ep_stats.resize(statics.ep_stats.size() - 1);

  printf("# Exiting after %d episodes, %d steps\n", episode, n_steps);

  return statics;
}
