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

#include "LinearClassifier.h"
#include "Random.h"

LinearClassifier::LinearClassifier(int n_inputs_, int n_classes_)
    : n_inputs(n_inputs_), n_classes(n_classes_),
      params(n_inputs, n_classes), bias(n_classes)
{
    for (int i=0; i<n_inputs; ++i) {
        for (int j=0; j<n_classes; ++j) {
            params(i,j) = urandom() - 0.5;
        }
    }
}

Vector LinearClassifier::Output(const Vector& x)
{
    Matrix tmp(n_classes, n_inputs);
    for (int i=0; i<n_inputs; ++i) {
        for (int j=0; j<n_classes; ++j) {
            tmp(j,i) = params(i,j);
        }
    }
    Vector out = exp(((const Matrix&) tmp)* x + bias);
    return out / out.Sum();
}

void LinearClassifier::Show()
{
    params.print(stdout);
}


void StochasticGradientClassifier::Observe(const Vector& x, int label)
{
    assert(x.Size() == classifier.n_inputs);
    assert(label >= 0 && label < classifier.n_classes);
    real y = classifier.Output(x)[label];
    if (y < 0) {
        y = 0;
    }
    real eta = alpha * (exp(- y));
    for (int i=0; i<classifier.n_inputs; ++i) {
        params(i, label) += eta * x(i);
    }
    bias(label) += eta;
}
