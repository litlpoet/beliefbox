/* -*- Mode: C++; -*- */
/* VER: $Id: DiscretePolicy.h,v 1.1 2006/10/23 08:33:32 olethros Exp
 * cdimitrakakis $*/
// copyright (c) 2007 by Christos Dimitrakakis <christos.dimitrakakis@gmail.com>
/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#ifndef DISCRETE_POLICY_H
#define DISCRETE_POLICY_H

#include "AbstractPolicy.h"
#include "Demonstrations.h"
#include "DiscreteMDP.h"
#include "Matrix.h"

/** A policy in a discrete state-action space.

        Abstract class
 */
class DiscretePolicy : public AbstractPolicy<int, int> {
 protected:
  int n_states;
  int n_actions;

 public:
  DiscretePolicy(int n_states_, int n_actions_)
      : n_states(n_states_), n_actions(n_actions_) {}
  virtual ~DiscretePolicy() {}
  virtual int SelectAction() = 0;
  virtual void Observe(const int& previous_state, const int& action, real r,
                       const int& next_state) = 0;
  virtual void Observe(real r, const int& next_state) = 0;
  virtual void Reset() {
    int reset_state = 0;
    Reset(reset_state);
  }
  virtual void Reset(const int& start_state) = 0;
  virtual real getActionProbability(const int& action) const = 0;
  virtual real getActionProbability(const int& state,
                                    const int& action) const = 0;
  virtual Vector getActionProbabilities(const int& state) const = 0;
  virtual void Show() = 0;
};

class FixedDiscretePolicy : public DiscretePolicy {
 public:
  std::vector<Vector> p;
  FixedDiscretePolicy(int n_states, int n_actions);
  FixedDiscretePolicy(int n_states, int n_actions,
                      const std::vector<Vector>& p);
  FixedDiscretePolicy(int n_states, int n_actions, const Matrix& Q);
  FixedDiscretePolicy(int n_states, int n_actions,
                      const Demonstrations<int, int>& D);
  virtual ~FixedDiscretePolicy();
  virtual int SelectAction();
  virtual void Observe(const int& previous_state, const int& action, real r,
                       const int& next_state);
  virtual void Observe(real r, const int& next_state);
  virtual void Reset() {
    int reset_state = 0;
    Reset(reset_state);
  }
  virtual void Reset(const int& start_state);
  virtual real getActionProbability(const int& action) const;
  virtual real getActionProbability(const int& state, const int& action) const;
  inline virtual Vector getActionProbabilities(const int& state) const {
    return p[state];
  }
  inline virtual Vector* getActionProbabilitiesPtr(const int& state) {
    return &p[state];
  }
  virtual void Show();
  FixedDiscretePolicy MakeGreedyPolicy();
};

class FixedSoftmaxPolicy : public FixedDiscretePolicy {
 public:
  std::vector<Vector> p;
  FixedSoftmaxPolicy(Matrix& Q, real beta);
  virtual ~FixedSoftmaxPolicy();
};

Matrix DiscountedStateOccupancy(const DiscreteMDP& mdp,
                                const FixedDiscretePolicy& policy, real gamma,
                                real epsilon);

#endif
