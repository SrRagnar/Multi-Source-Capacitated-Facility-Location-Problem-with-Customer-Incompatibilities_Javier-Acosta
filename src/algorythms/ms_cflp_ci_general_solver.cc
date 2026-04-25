// University of La Laguna
// School of Engineering and Technology
// Bachelor's Degree in Computer Engineering
// Subject: Design and Analysis of Algorithms
// Course: 3rd
// Practice 6: Multi-Source Capacitated Facility Location Problem with Customer Incompatibilities
// Author: Javier Acosta Portocarrero
// Date: 04/04/2026
// File ms_cflp_ci_general_solver.cc: implementation file.
// Contains the implementation of the MsCflpCiSolver class.

#include <stdexcept>

#include "ms_cflp_ci_general_solver.h"

MsCflpCiSolution* MsCflpCiGeneralSolver::SolveMsCflpCiInstance(MsCflpCiInstance* input) const {
  if (input == nullptr) {
    throw std::invalid_argument("Null instance given.");
  }
  Solution* solution = strategy_->Solve(input);
  MsCflpCiSolution* ms_cflp_ci_solution = dynamic_cast<MsCflpCiSolution*>(solution);
  if (ms_cflp_ci_solution == nullptr) {
    delete solution;
    throw std::runtime_error("The strategy is not compatible with the MsCflpCiInstance.");
  }
  return ms_cflp_ci_solution;
}
