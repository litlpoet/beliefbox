#include "InventoryManagement.h"
#include <vector>
#include "DiscreteMDP.h"
#include "Gridworld.h"
#include "Matrix.h"
#include "MatrixNorm.h"
#include "MultiMDPValueIteration.h"
#include "ValueIteration.h"

#define ACCURACY_THRESHOLD 10e-6

Matrix GetQValues(const DiscreteMDP* mdp, real gamma) {
  ValueIteration value_iteration(mdp, gamma);
  value_iteration.ComputeStateActionValues(ACCURACY_THRESHOLD);

  Matrix Q(mdp->getNStates(), mdp->getNActions());
  for (int s = 0; s < mdp->getNStates(); ++s) {
    for (int a = 0; a < mdp->getNActions(); ++a) {
      Q(s, a) = value_iteration.getValue(s, a);
    }
  }
  return Q;
}

Matrix GetQValues(std::vector<const DiscreteMDP*>& M, Vector& w, real gamma) {
  MultiMDPValueIteration value_iteration(w, M, gamma);
  value_iteration.ComputeStateActionValues(ACCURACY_THRESHOLD);

  Matrix Q(M[0]->getNStates(), M[0]->getNActions());
  for (int s = 0; s < M[0]->getNStates(); ++s) {
    for (int a = 0; a < M[0]->getNActions(); ++a) {
      Q(s, a) = value_iteration.getValue(s, a);
    }
  }
  return Q;
}

int main(void) {
  real gamma = 0.9;
  InventoryManagement inventory_management_A(10, 25, 0.1, 1.3);
  InventoryManagement inventory_management_B(15, 25, 0.2, 1.1);

  std::vector<const DiscreteMDP*> mdp_list;
  std::vector<Matrix> Q;
  mdp_list.push_back(inventory_management_A.getMDP());
  mdp_list.push_back(inventory_management_B.getMDP());
  Q.push_back(GetQValues(mdp_list[0], gamma));
  Q.push_back(GetQValues(mdp_list[1], gamma));

  for (real weight = 0; weight <= 1; weight += 0.01) {
    Vector w(2);
    w(0) = weight;
    w(1) = 1 - weight;
    Matrix Qw = GetQValues(mdp_list, w, gamma);
    printf("%f %f %f # F NORM\n", weight, FrobeniusNorm(Qw - Q[0]),
           FrobeniusNorm(Qw - Q[1]));
  }

  for (real weight = 0; weight <= 1; weight += 0.01) {
    Vector w(2);
    w(0) = weight;
    w(1) = 1 - weight;
    Matrix Qw = GetQValues(mdp_list, w, gamma);
    Vector V = Qw.ColumnMax();
    printf("%f ", weight);
    for (int s = 0; s < V.Size(); ++s) {
      printf("%f ", V(s));
    }
    printf(" # w | Vs\n");
  }
  return 0;
}
