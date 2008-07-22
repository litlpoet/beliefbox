// -*- Mode: C++; -*-
// copyright (c) 2008 by Christos Dimitrakakis <christos.dimitrakakis@gmail.com>
/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifdef MAKE_MAIN
#include "PolicyEvaluation.h"
#include "BetaDistribution.h"
#include "Random.h"

#include <list>
#include <vector>
#include <set>

class SimpleBelief
{
protected:
    BetaDistribution prior;
    real r1;
    real r2;
public:
    /// Create a belief
    SimpleBelief(real alpha = 1.0, real beta=1.0, real r1_=0.0, real r2_=1.0)
        : prior(alpha, beta), r1(r1_), r2(r2_)
    {
    }

    void update(int state, int action, real reward, int next_state)
    {
        if (state==0 && action == 0) {
            if (reward == r1) {
                prior.calculatePosterior(0.0);
            } else if (reward == r2) {
                prior.calculatePosterior(1.0);
            } else {
                fprintf (stderr, "Reward of %f should not have been observed in this state\n", reward);
            }
        }
    }

    real getProbability(int state, int action, real reward, int next_state)
    {
        if (state==0 && action == 0) {
            real p = prior.getMean();
            if (reward == r1) {
                return 1.0 - p;
            } else if (reward == r2) {
                return p;
            } else {
                fprintf (stderr, "Reward of %f is illegal for s,a=0,0\n", reward);
            }
        }
        return 1.0;
    }
    
    real getGreedyReturn(int state, real gamma)
    {
        if (state == 0) {
            real p = prior.getMean();
            real R = p*r2 + (1.0 - p)*r1;
            real U = R / (1.0 - gamma);
            if (U > 0) {
                return U;
            } 
        } 
        return 0.0;
    }
};

template <typename B>
class BeliefTree
{
protected:
    std::vector<Distribution*> densities;
public:
    class Edge;
    class Node
    {
    public:
        B belief;
        int state;
        std::vector<Edge*> outs;
        int index;
    };
    
    class Edge
    {
    public:
        Node* src; ///< source node
        Node* dst; ///< destination node
        int a; ///< action taken
        real r; ///< reward received
        real p; ///< probability of path
        Edge (Node* src_, Node* dst_, int a_, real r_, real p_)
            : src(src_), dst(dst_), a(a_), r(r_), p(p_)
        {
        }
    };

    std::vector<Node*> nodes;
    std::vector<Edge*> edges;
    
    int n_states;
    int n_actions;
    BeliefTree(SimpleBelief prior,
               int state,
               int n_states_,
               int n_actions_)
        :
        n_states(n_states_),
        n_actions(n_actions_)
    {
        Node* root = new Node;
        root->belief = prior;
        root->state = state;
        root->index = 0;
        nodes.push_back(root);
    }

    ~BeliefTree()
    {
        std::cout << "D: " << densities.size() << std::endl;
        for (int i=densities.size(); i>=0; --i) {
            std::cout << i << std::endl;
        }
    }
    /// Return 
    Node* ExpandAction(int i, int a, real r, int s)
    {
        Node* next = new Node;
        next->belief = nodes[i]->belief;
        next->state = s;
        next->index = nodes.size();
        real p = nodes[i]->belief.getProbability(nodes[i]->state, a, r, s);
        next->belief.update(nodes[i]->state, a, r, s);

        
        edges.push_back(new Edge(nodes[i],
                             nodes[nodes.size()-1],
                             a, r, p));
        printf ("Added edge %d : %d --(%d %f %f)-> %d\n",
                (int) edges.size() - 1,
                i,
                a, r, p,
                (int) nodes.size() - 1);

        int k = edges.size() - 1;
        printf ("Added edge %d : %d --(%d %f %f)-> %d\n",
                k,
                edges[k]->src->index,
                edges[k]->a,
                edges[k]->r,
                edges[k]->p,
                edges[k]->dst->index);
                
        nodes[i]->outs.push_back(edges[k]);

        nodes.push_back(next);
        return nodes.back();
    }

    /// Return 
    //    std::vector<Node&> Expand(int i)
    void Expand(int i)
    {
        //B belief = nodes[i]->belief;
        int state = nodes[i]->state;
        //std::vector<Node*> node_set;
        if (state==1) { 
            // terminal state, all actions do nothing whatsoever.            
            ExpandAction(i, 0, 0.0, 1);
            ExpandAction(i, 1, 0.0, 1);
        } else if (state==0) {
            // If we play, there are two possibilities
            ExpandAction(i, 0, 1.0, 0);
            ExpandAction(i, 0, -1.0, 0); // VALGRIND
            // if we move to the terminal state nothing happens
            ExpandAction(i, 1, 0.0, 1);
        }
        //return node_set;
    }

    std::vector<Node*>& getNodes()
    {
        return nodes;
    }

    DiscreteMDP CreateMDP(real gamma)
    {
        int n_nodes = nodes.size();
        DiscreteMDP mdp(n_nodes, 2, NULL, NULL);
        for (int i=0; i<n_nodes; i++) {
            int n_edges = nodes[i]->outs.size();
            for (int j=0; j<n_edges; j++) {
                Edge* edge = nodes[i]->outs[j];
                Distribution* reward_density = 
                    new SingularDistribution(edge->r);
                
                densities.push_back(reward_density);
                mdp.setTransitionProbability(i,
                                             edge->a,
                                             edge->dst->index,
                                             edge->p);
                
                mdp.setRewardDistribution(i,
                                          edge->a,
                                          reward_density);
            }
            
            if (!n_edges) {
                real mean_reward = nodes[i]->belief.getGreedyReturn(nodes[i]->state, gamma);
                for (int a=0; a<n_actions; a++) {
                    mdp.setTransitionProbability(i, a, i, 1.0);

                    Distribution* reward_density = 
                        new SingularDistribution(mean_reward);
                    densities.push_back(reward_density);
                    mdp.setRewardDistribution(i, a, reward_density);
                }

            }
        }
        return mdp;
    }
};



/// A toy UCT stopping problem




//void EvaluateAlgorithm(BeliefExpansionAlgorithm& algorithm, real mean_r);


int main (int argc, char** argv)
{
    real alpha = 1.0;
    real beta = 1.0;
    real gamma = 0.9;

    real actual_probability = urandom(0, 1);

    SimpleBelief prior(alpha, beta, -1.0, 1.0);

    BeliefTree<SimpleBelief> tree(prior, 0, 2, 2);
    std::vector<BeliefTree<SimpleBelief>::Node*> node_set = tree.getNodes();

    for (int iter=0; iter<1; iter++) {
        int edgeless_nodes = 0;
        int edge_nodes = 0;
        int node_index = -1;
        for (uint i=0; i<node_set.size(); ++i) {
            if (node_set[i]->outs.size()==0) {
                edgeless_nodes++;
                if (node_index == -1) {
                    node_index = i;
                }
            } else {
                edge_nodes++;
            }
        }

        std::cout << edgeless_nodes << " leaf nodes, "
                  << edge_nodes << " non-leaf nodes, "
                  << "expanding node " << node_index
                  << std::endl;
        
        tree.Expand(node_index); // VALGRIND
    }

    DiscreteMDP mdp = tree.CreateMDP(gamma); // VALGRIND

    mdp.ShowModel();

    return 0;
}


#endif
