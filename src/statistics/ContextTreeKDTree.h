/* -*- Mode: c++;  -*- */
// copyright (c) 2010 by Christos Dimitrakakis <christos.dimitrakakis@gmail.com>
/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef CONTEXT_TREE_KD_TREE_H
#define CONTEXT_TREE_KD_TREE_H

#include <vector>
#include "DeltaDistribution.h"
#include "MomentMatchingBetaEstimate.h"
#include "NormalDistribution.h"
#include "Ring.h"
#include "Vector.h"
#include "real.h"

#undef RANDOM_SPLITS
#undef USE_GAUSSIAN_MIX

/** Context tree non-parametric density estimation on \f$R^n\f$.

    This is a generalisation of the binary tree on [a,b] implemented
        in ContextTreeRealLine to a KD-tree. For any sample \f$x^t \sim
        D^t\f$, with \f$x_i \in R^n\f$ it estimates the measure
        \f[
        P^t(w) = P(w \mid x^t) \propto P_w(x^t)P^0(w)
        \f]
        and so the marginal
        \f[
        P(x_{t+1} \mid x^t) = \sum_w P_w(x_t) P(w \mid x^t).
        \f]
        The distribution \f$P^t(w)\f$ is simply a product distribution.

        The model can also be used for estimating conditional densities,
   directly.
        However, this is perhaps not a good idea.
 */
class ContextTreeKDTree {
 public:
  // public classes
  struct Node {
    ContextTreeKDTree& tree;  ///< a tree estimator
// MomentMatchingBetaEstimate beta_product; ///< beta product estimator
#ifdef USE_GAUSSIAN_MIX
    MultivariateNormalUnknownMeanPrecision gaussian;  ///< gaussian estimator
    // DeltaUniformDistribution gaussian; ///< discrete estimator
    real w_gaussian;
#endif
    Vector lower_bound;       ///< looks at x > lower_bound
    Vector upper_bound;       ///< looks at x < upper_bound
    real volume;              ///< volume of area in this node
    real mid_point;           ///< how to split
    int splitting_dimension;  ///< dimension on which to do the split.
    const int depth;          ///< depth of the node
                              /*
                      const int n_branches; ///< number of branches
                      const int max_depth; ///< maximum depth
                              */
    Node* prev;               ///< previous node
    std::vector<Node*> next;  ///< pointers to next nodes
    real P;                   ///< probability of next symbols
    Vector alpha;             ///< number of times seen in each quadrant
    real w;                   ///< backoff weight
    real log_w;               ///< log of w
    real log_w_prior;         ///< initial value

    Node(ContextTreeKDTree& tree_, const Vector& lower_bound_,
         const Vector& upper_bound_);
    Node(Node* prev_, const Vector& lower_bound_, const Vector& upper_bound_);
    ~Node();
    real Observe(const Vector& x, real probability);
    real pdf(const Vector& x, real probability);
    void Show();
    int NChildren();
    int S;
  };

  // public methods
  ContextTreeKDTree(int n_branches_, int max_depth_, const Vector& lower_bound,
                    const Vector& upper_bound);
  ~ContextTreeKDTree();
  real Observe(const Vector& x);
  real pdf(const Vector& x);
  void Show();
  int NChildren();

 protected:
  int n_branches;
  int max_depth;
  Node* root;
};

#endif
