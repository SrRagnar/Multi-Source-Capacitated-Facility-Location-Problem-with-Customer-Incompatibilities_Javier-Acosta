// University of La Laguna
// School of Engineering and Technology
// Bachelor's Degree in Computer Engineering
// Subject: Design and Analysis of Algorithms
// Course: 3rd
// Practice 6: Multi-Source Capacitated Facility Location Problem with Customer Incompatibilities
// Author: Javier Acosta Portocarrero
// Date: 12/04/2026
// File grasp-ms-cflp-ci-vnd_solver.cc: implementation file.
// Contains the dimplementation of the GraspMsCflpCiVndSolver class.

#include <stdexcept>

#include "grasp_ms_cflp_ci_vnd_solver.h"

/**
 * @brief Postprocesses a solution by applying a Variable Neighborhood Descent (VND) local search.
 *
 * The method iteratively applies the neighborhood explorers in the order they were added to the solver,
 * restarting the sequence whenever an improving solution is found, until no more improvements can be made.
 *
 * @param solution The initial solution to postprocess.
 * @return A pointer to the locally optimal solution found after applying VND.
 */
Solution* GraspMsCflpCiVndSolver::Postprocess(Solution* solution) {
  MsCflpCiSolution* current = dynamic_cast<MsCflpCiSolution*>(solution);
  if (current == nullptr) {
    throw std::invalid_argument("Solution is not of type MsCflpCiSolution.");
  }
  const std::vector<MsCflpCiNeighboorhodExplorer*>& explorers = GetNeighborhoodExplorers();
  size_t explorer_index = 0;

  while (explorer_index < explorers.size()) {
    MsCflpCiSolution* explored_solution = explorers[explorer_index]->Explore(current, 
                                                                             GetAmountTolerance(), 
                                                                             GetImprovementTolerance());
    if (explored_solution != nullptr) {
      delete current;
      current = explored_solution;
      explorer_index = 0;
    } else {
      ++explorer_index;
    }
  }

  Solution* final_solution = new MsCflpCiSolution(*current);
  delete current;
  return final_solution;
}