/* -*- Mode: C++; -*- */
// copyright (c) 2009 by Christos Dimitrakakis <christos.dimitrakakis@gmail.com>
/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef BASIS_SET_H
#define BASIS_SET_H

#include <cassert>
#include <vector>
#include "Grid.h"
#include "Vector.h"

/** A simple radial basis function */
class RBF {
 public:
  Vector center;  ///< The centroid
  Vector beta;    ///< the variance
  /// Constructor
  RBF(const Vector& c, real b) : center(c) {
    assert(b > 0);
    Vector var(c.Size(), &b);
    beta = var;
  }

  RBF(const Vector& c, const Vector& b) : center(c), beta(b) { assert(b > 0); }

  /// Get the density at point x
  real Evaluate(const Vector& x) {
    Vector d = pow((x - center) / beta, 2.0);
    real r = d.Sum();
    // real d = EuclideanNorm(&x, &center);
    return exp(-0.5 * r);
  }
  /// Evaluate the log density
  real logEvaluate(const Vector& x) {
    Vector d = pow((x - center) / beta, 2);

    return d.Sum();
    //    return (-beta) * EuclideanNorm(&x, &center);
  }
};

class RBFBasisSet {
 protected:
  std::vector<RBF*> centers;

  Vector log_features;
  Vector features;
  bool valid_features;
  bool valid_log_features;
  int n_bases;

 public:
  RBFBasisSet()
      : valid_features(false), valid_log_features(false), n_bases(0) {}
  RBFBasisSet(const EvenGrid& grid, real scale = 1);
  ~RBFBasisSet();
  void AddCenter(const Vector& v, const Vector& b);
  void AddCenter(const Vector& v, real b);
  void Evaluate(const Vector& x);
  void logEvaluate(const Vector& x);
  int size() { return n_bases; }
  real log_F(int j) {
    assert(j >= 0 && j < n_bases);
    assert(valid_log_features);
    return log_features[j];
  }
  Vector log_F() { return log_features; }
  real F(int j) {
    assert(j >= 0 && j < n_bases);
    assert(valid_features);
    return features[j];
  }
  Vector F() { return features; }
};
#endif
