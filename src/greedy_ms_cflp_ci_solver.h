// University of La Laguna
// School of Engineering and Technology
// Bachelor's Degree in Computer Engineering
// Subject: Design and Analysis of Algorithms
// Course: 3rd
// Practice 6: Multi-Source Capacitated Facility Location Problem with Customer Incompatibilities
// Author: Javier Acosta Portocarrero
// Date: 04/04/2026
// File greedy_ms_cflp_ci_solver.h: declaration file.
// Contains the declaration of the GreedyMsCflpCiSolver class.

#ifndef GREEDY_MS_CFLP_CI_SOLVER_H_
#define GREEDY_MS_CFLP_CI_SOLVER_H_

#include "algorythm.h"

class GreedyMsCflpCiSolver : public Algorythm {
 public:
  GreedyMsCflpCiSolver() = default;
  ~GreedyMsCflpCiSolver() = default;

  Solution* Solve(Instance* input) override;
};

#endif