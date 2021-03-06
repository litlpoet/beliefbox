/* VER: $Id: Distribution.h,v 1.3 2006/11/06 15:48:53 cdimitrakakis Exp
 * cdimitrakakis $*/
// copyright (c) 2004 by Christos Dimitrakakis <christos.dimitrakakis@gmail.com>
/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef SRC_STATISTICS_MERSENNETWISTER_H_
#define SRC_STATISTICS_MERSENNETWISTER_H_

#include "RandomNumberGenerator.h"

// This is a static Mersenne Twister random number generator
class MersenneTwister {
 protected:
  static unsigned long initial_seed;
  static const int n;
  static const int m;
  static unsigned long state[]; /* the array for the state vector  */
  static int left;
  static int initf;
  static unsigned long *next;
  static void nextState();

 public:
  ~MersenneTwister();

  /// Initializes the random number generator with the computer clock.
  static void seed();

  /// Initializes the random number generator with the given long "the_seed_".
  static void manualSeed(unsigned long the_seed_);

  /// Returns the starting seed used.
  static unsigned long getInitialSeed();

  /// Generates a uniform 32 bits integer.
  static unsigned long random();

  /// Generates a uniform random number on [0,1[.
  static real uniform();
};

/// This Mersenne Twister random number generator can be passed
/// as an argument.
class MersenneTwisterRNG : public RandomNumberGenerator {
 protected:
  unsigned long initial_seed;
  const int n;
  const int m;
  unsigned long *state; /* the array for the state vector  */
  int left;
  int initf;
  unsigned long *next;
  void nextState();

 public:
  MersenneTwisterRNG();

  virtual ~MersenneTwisterRNG();

  /// Seed to time
  virtual void seed();

  /// Initializes the random number generator with the given long "the_seed_".
  virtual void manualSeed(unsigned long the_seed_);

  /// Returns the starting seed used.
  virtual unsigned long getInitialSeed();

  /// Generates a uniform 32 bits integer.
  virtual unsigned long random();

  /// Generates a uniform random number in [0,1[.
  virtual real uniform();

  /// Generates a uniform random number in [0,n)
  inline virtual int discrete_uniform(int n) {
    return (int)floor(uniform() * ((real)n));
  }

  inline virtual real uniform(real lower_bound, real upper_bound) {
    return lower_bound + (upper_bound - lower_bound) * uniform();
  }
};

#endif  // SRC_STATISTICS_MERSENNETWISTER_H_
