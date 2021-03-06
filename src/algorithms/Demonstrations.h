/// -*- Mode: c++ -*-
// copyright (c) 2012 by Christos Dimitrakakis <christos.dimitrakakis@gmail.com>
/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef DEMONSTRATIONS_H
#define DEMONSTRATIONS_H

#include "AbstractPolicy.h"
#include "Environment.h"
#include "Random.h"
#include "Trajectory.h"

template <class S, class A>
class Demonstrations {
 public:
  std::vector<Trajectory<S, A> > trajectories;
  Trajectory<S, A>* current_trajectory;
  std::vector<real> total_rewards;
  std::vector<real> discounted_rewards;
  std::vector<int> steps;
  Demonstrations(bool new_episode = true) : current_trajectory(NULL) {
    // fprintf(stderr, "Creating Demonstrators\n");
    if (new_episode) {
      NewEpisode();
    }
  }
  void Observe(S s, A a) {
    // fprintf(stderr, "Size of trajectories: %d\n", trajectories.size())
    if (!current_trajectory) NewEpisode();
    current_trajectory->Observe(s, a);
  }
  void Observe(S s, A a, real r) {
    // fprintf(stderr, "Size of trajectories: %d\n", trajectories.size())
    if (!current_trajectory) NewEpisode();
    current_trajectory->Observe(s, a, r);
  }
  void Terminate() {
    // fprintf(stderr, "Size of trajectories: %d\n", trajectories.size())
    if (current_trajectory) current_trajectory->Terminate();
  }

  /// Start a new episode
  void NewEpisode() {
    // fprintf(stderr, "Adding Episode in Trajectories\n");
    trajectories.push_back(Trajectory<S, A>());
    current_trajectory = &trajectories[trajectories.size() - 1];
    total_rewards.push_back(0.0);
    discounted_rewards.push_back(0.0);
    steps.push_back(0);
  }
  /// Use a negative horizon to use a geometric stopping distribution
  void Simulate(Environment<S, A>& environment, AbstractPolicy<S, A>& policy,
                real gamma, int horizon) {
    environment.Reset();
    policy.Reset();
    NewEpisode();
    bool running = true;
    real discount = 1.0;
    real total_reward = 0.0;
    real discounted_reward = 0.0;
    int t = 0;
    real reward = 0.0;
    do {
      // get current state
      S state = environment.getState();

      // choose an action
      policy.Observe(reward, state);
      A action = policy.SelectAction();
      running = environment.Act(action);

      // get reward
      reward = environment.getReward();
      // save sample
      Observe(state, action, reward);

      total_reward += reward;
      discounted_reward += discount * reward;
      if (horizon >= 0) {
        discount *= gamma;
      }

      if (!running) {
        S state = environment.getState();
        real reward = 0.0;  // environment.getReward();
        policy.Observe(reward, state);
        A action = policy.SelectAction();
        Observe(state, action, reward);
        Terminate();
      }
      if (horizon >= 0 && t >= horizon) {
        running = false;
      } else if (horizon < 0 && urandom() < 1.0 - gamma) {
        running = false;
      }
      ++t;
    } while (running);
    // logmsg("Terminating after %d steps\n", t);
    int index = trajectories.size() - 1;
    total_rewards[index] = total_reward;
    discounted_rewards[index] = discounted_reward;
    steps[index] = t;
  }

  /// total number of trajectories
  uint size() const {
    if (trajectories.size() == 0) {
      return 0;
    }
    if (trajectories[trajectories.size() - 1].size() > 0) {
      return trajectories.size();
    }
    return trajectories.size() - 1;
  }

  /// state of the i-th trajectory at time t
  S state(uint i, uint t) const {
    assert(i < size());
    assert(t < length(i));
    assert(t < trajectories[i].size());
    return trajectories[i].state(t);
  }

  /// action of the i-th trajectory at time t
  A action(uint i, uint t) const {
    assert(i < size());
    assert(t < length(i));
    assert(t < trajectories[i].size());
    return trajectories[i].action(t);
  }

  /// reward of the i-th trajectory at time t
  real reward(uint i, uint t) const {
    assert(i < size());
    assert(t < trajectories[i].rewards.size());
    return trajectories[i].reward(t);
  }
  /// total reward of the i-th trajectory
  real total_reward(uint i) const {
    assert(i < size());
    return total_rewards[i];
  }
  /// discounted reward of the i-th trajectory
  real discounted_reward(uint i) const {
    assert(i < size());
    return discounted_rewards[i];
  }
  /// Length of the i-th trajectory
  uint length(uint i) const { return trajectories[i].size(); }

  /// return true if the demonstration was terminated
  bool terminated(uint i) const { return trajectories[i].terminated(); }
};

#endif
