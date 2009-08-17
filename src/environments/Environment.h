// -*- Mode: c++ -*-
// copyright (c) 2008 by Christos Dimitrakakis <christos.dimitrakakis@gmail.com>
// $Revision$
/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef ENVIRONMENT_H
#define ENVIRONMENT_H

#include "MDP.h"
#include <cstdlib>
/**
   \defgroup EnvironmentGroup Environments
 */
/**
   \ingroup EnvironmentGroup
 */
/*@{*/
/// Template for environments
template <typename S, typename A>
class Environment
{
protected:
    S state; ///< The current state
    real reward; ///< The current reward
public:
    virtual ~Environment() 
    {
    }

    /// put the environment in its "natural: state
    virtual void Reset() = 0;

    /// returns true if the action succeeds, false if it does not.
    ///
    /// The usual of false is that the environment is in a terminal
    /// absorbing state.
    virtual bool Act(A action) = 0;
    
    /// Return a full MDP model of the environment. 
    /// This may not be possible for some environments
    virtual MDP<S, A>* getMDP() const
    {
        return NULL;
    }
    /// returns a (copy of) the current state
    S getState()
    {
        return state;
    }

    /// returns the current reward
    real getReward()
    {
        return reward;
    }

};

/// Default type for discrete environments
typedef Environment<int, int> DiscreteEnvironment;

/*@}*/

#endif
