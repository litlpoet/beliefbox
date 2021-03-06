/* -*- Mode: c++;  -*- */
// copyright (c) 2007 by Christos Dimitrakakis <christos.dimitrakakis@gmail.com>
/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#ifndef DENSE_MARKOVCHAIN_H
#define DENSE_MARKOVCHAIN_H

#include "MarkovChain.h"

/**
   \ingroup StatisticsGroup
 */
/*@{*/

/// A dense implementation of a Markov chain
class DenseMarkovChain : public MarkovChain {
 protected:
  void EstimateProbabilities(MCState curr);
  int n_transitions;   ///< total number of transitions
  Vector transitions;  ///< history-wide transition table
  Vector Pr;           ///< transition probabilities

 public:
  DenseMarkovChain(int n_states, int mem_size);
  virtual ~DenseMarkovChain();

  /* probabilities */
  virtual real getTransition(MCState src, int dst);
  virtual real getProbability(MCState src, int dst);
  virtual void getProbabilities(MCState src, std::vector<real>& p);
  virtual void getNextStateProbabilities(std::vector<real>& p);
  virtual real pdf(MCState src, Vector q);
  virtual void setTransition(MCState src, int dst, real value);
  virtual void setThreshold(real threshold);

  /* Training and generation */
  virtual real ObserveNextState(int state);
  virtual real NextStateProbability(int state);
  virtual void Reset();
  virtual int GenerateStatic();
  virtual int generate();

  /* Debug functions */
  virtual int ShowTransitions();
};
/*@}*/
#endif
