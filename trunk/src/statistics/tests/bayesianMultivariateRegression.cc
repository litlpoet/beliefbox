/* -*- Mode: C++; -*- */
// copyright (c) 2012 by Nikolaos Tziortziotis <ntziorzi@gmail.com>
/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifdef MAKE_MAIN

#include "EasyClock.h"
#include "NormalDistribution.h"
#include "BayesianMultivariateRegression.h"
#include "Matrix.h"
#include "Vector.h"
#include "Random.h"
#include <vector>
#include <memory>

int main(int argc, char* argv[])
{
	if (argc != 6) {
		fprintf(stderr, "Usage: Bayesian Multivariate Regression points dimensions a N_0\n");
		exit(-1);
	}
	int n_points = atoi(argv[1]);
	int n_dimensions_x = atoi(argv[2]);
	int n_dimensions_y = atoi(argv[3]);
	real a = atof(argv[4]);
	real N0 = atof(argv[5]);

	BayesianMultivariateRegression bmr(n_dimensions_x, n_dimensions_y, N0 * Matrix::Unity(n_dimensions_y, n_dimensions_y), N0, a);
	std::auto_ptr<Distribution> distribution_x(new NormalDistribution(0.0,1.0));
	std::auto_ptr<Distribution> distribution_y(new NormalDistribution(1,0));
   
	std::vector<Vector> X(n_points); ///< Input vector
	std::vector<Vector> Y(n_points); ///< Output vector
	for( int i = 0; i < n_points; ++i) {
		X[i].Resize(n_dimensions_x);
		Y[i].Resize(n_dimensions_y);
		for(int j = 0; j < n_dimensions_y; ++j) {
			real ZX  = distribution_x->generate();
			X[i][j] = urandom(2.0,7.0);
			Y[i][j] = 2.0*X[i][j] + ZX;
		}
		X[i][n_dimensions_y] = 1.0;
	}

	for( int i = 0; i < n_points; ++i) {
		bmr.AddElement(Y[i],X[i]);
	}
	
	const Matrix W1 = bmr.generate();
	W1.print(stdout);

	const Matrix W2 = bmr.generate();
	W2.print(stdout);

	const Matrix W3 = bmr.generate();
	W3.print(stdout);
	
	std::vector<Vector> R1(n_points); 
	std::vector<Vector> R2(n_points); 
	std::vector<Vector> R3(n_points); 
	for(int i = 0; i < n_points; ++i){
		R1[i] = W1 * X[i];
		R2[i] = W2 * X[i];
		R3[i] = W3 * X[i];
		printf("Input|");
		X[i].print(stdout);
		printf("Output|");
		Y[i].print(stdout);
		printf("Prediction1|");
		R1[i].print(stdout);
		printf("Prediction2|");
		R2[i].print(stdout);
		printf("Prediction3|");
		R3[i].print(stdout);
	}
}

#endif