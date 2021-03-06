/* -*- Mode: C++; -*- */
// copyright (c) 2012 by Christos Dimitrakakis <christos.dimitrakakis@gmail.com>
/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#ifndef WISHART_H
#define WISHART_H

#include "Distribution.h"
#include "Matrix.h"

/// Wishart probability distribution
class Wishart : public AbstractDistribution<Matrix> {
 protected:
  Matrix Precision;   ///< precision matrix
  Matrix Covariance;  ///< covariance matrix
 public:
  int k;   ///< dimensionality
  real n;  ///< degrees of freedom
  Wishart();
  Wishart(real n_, const Matrix& V, bool is_covariance = false);
  virtual ~Wishart();
  virtual void generate(Matrix& X) const;
  virtual Matrix generate() const;
  virtual real pdf(const Matrix& X) const { return exp(log_pdf(X)); }
  virtual real log_pdf(const Matrix& X) const;
  void setCovariance(const Matrix& V) {
    Covariance = V;
    Precision = V.Inverse();
  }
  void setPrecision(const Matrix& V) {
    Covariance = V.Inverse();
    Precision = V;
  }
  void Show() {
    logmsg("Precision:");
    Precision.print(stdout);

    logmsg("Covariance:");
    Covariance.print(stdout);
  }
};

#endif
