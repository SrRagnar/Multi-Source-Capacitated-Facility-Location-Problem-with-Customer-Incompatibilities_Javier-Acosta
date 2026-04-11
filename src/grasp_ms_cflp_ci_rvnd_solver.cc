// University of La Laguna
// School of Engineering and Technology
// Bachelor's Degree in Computer Engineering
// Subject: Design and Analysis of Algorithms
// Course: 3rd
// Practice 6: Multi-Source Capacitated Facility Location Problem with Customer Incompatibilities
// Author: Javier Acosta Portocarrero
// Date: 12/04/2026
// File grasp-ms-cflp-ci-rvnd_solver.cc: implementation file.
// Contains the implementation of the GraspMsCflpCiRvndSolver class.

#include <stdexcept>
#include <vector>
#include <cstdlib>

#include "grasp_ms_cflp_ci_rvnd_solver.h"

/**
 * @brief Postprocesses a solution by applying a Randomized Variable Neighborhood Descent (RVND) local search.
 *
 * The method iteratively applies the neighborhood explorers in random order, 
 * restarting the sequence whenever an improving solution is found,
 * until no more improvements can be made and all explorers have been tried without success.
 *
 * @param solution The initial solution to postprocess.
 * @return A pointer to the locally optimal solution found after applying RVND.
 */
Solution* GraspMsCflpCiRvndSolver::Postprocess(Solution* solution) {
  MsCflpCiSolution* current = dynamic_cast<MsCflpCiSolution*>(solution);
  if (current == nullptr) {
    throw std::invalid_argument("Solution is not of type MsCflpCiSolution.");
  }
  const std::vector<MsCflpCiNeighboorhodExplorer*>& explorers = GetNeighborhoodExplorers();
  std::vector<size_t> active_explorers;
  for (size_t i = 0; i < explorers.size(); ++i) {
    active_explorers.push_back(i);
  }

  while (!active_explorers.empty()) {
    const size_t random_position = static_cast<size_t>(std::rand()) % active_explorers.size();
    const size_t explorer_index = active_explorers[random_position];
    MsCflpCiSolution* explored_solution =
        explorers[explorer_index]->Explore(current, GetAmountTolerance(), GetImprovementTolerance());

    if (explored_solution != nullptr) {
      delete current;
      current = explored_solution;
      active_explorers.clear();
      for (size_t i = 0; i < explorers.size(); ++i) {
        active_explorers.push_back(i);
      }
    } else {
      active_explorers.erase(active_explorers.begin() + static_cast<long>(random_position));
    }
  }

  Solution* final_solution = new MsCflpCiSolution(*current);
  delete current;
  return final_solution;
}