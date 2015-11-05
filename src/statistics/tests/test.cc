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

#ifdef MAKE_MAIN
#include "ContextTreeKDTree.h"
#include <vector>
#include "BetaDistribution.h"
#include "ContextTreeRealLine.h"
#include "EasyClock.h"
#include "MultivariateNormal.h"
#include "MultivariateNormalUnknownMeanPrecision.h"
#include "NormalDistribution.h"
#include "Random.h"
#include "ranlib.h"

int main() {
  long seed = time(NULL);

  setall(seed, seed / 100);
  for (int i = 0; i <= 10; ++i) {
    printf("%f\n", genchi(1 + i));
  }
  return 0;
}

#endif
