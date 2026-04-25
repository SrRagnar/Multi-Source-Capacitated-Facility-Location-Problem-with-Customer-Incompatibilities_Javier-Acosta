// University of La Laguna
// School of Engineering and Technology
// Bachelor's Degree in Computer Engineering
// Subject: Design and Analysis of Algorithms
// Course: 3rd
// Practice 6: Multi-Source Capacitated Facility Location Problem with Customer Incompatibilities
// Author: Javier Acosta Portocarrero
// Date: 25/04/2026
// File grasp-ms-cflp-ci-gvns_rl_solver.cc: implementation file.
// Contains the declaration of the GraspMsCflpCiRvndSolver class.

#include <stdexcept>
#include <random>
#include <vector>
#include <cstdlib>

#include "grasp_ms_cflp_ci_gvns_rl_solver.h"

/**
 * @brief Postprocesses a solution by applying a GVNS (General Variable Neighborhood Search),
 * with reinforcement learning.
 * 
 * @param solution The initial solution to postprocess.
 * @return A pointer to the locally optimal solution found after applying RVND.
 */
Solution* GraspMsCflpCiGvnsRl::Postprocess(Solution* solution) {
  MsCflpCiSolution* current_solution = dynamic_cast<MsCflpCiSolution*>(solution);
  if (current_solution == nullptr) {
    throw std::invalid_argument("Solution is not of type MsCflpCiSolution.");
  }
 
  size_t perturbator_index = 0;
  while (perturbator_index < perturbators_.size()) {
    MsCflpCiSolution* new_solution = perturbators_[perturbator_index++]->
        Perturbate(
            current_solution, 
            GetAmountTolerance(), 
            GetImprovementTolerance());
            
    new_solution = VndWithReinforcementLearning(new_solution);
    if (new_solution != nullptr) {
      delete current_solution;
      current_solution = new_solution;
      perturbator_index = 0;
      continue;
    }
    delete new_solution;
  }

  Solution* final_solution = new MsCflpCiSolution(*current_solution);
  delete current_solution;
  return final_solution;
}

MsCflpCiSolution* GraspMsCflpCiGvnsRl::VndWithReinforcementLearning(MsCflpCiSolution* Solution) const {
  unsigned iter_without_improvement = 0;
  unsigned total_iter = 0;
  const std::vector<MsCflpCiNeighboorhodExplorer*>& explorers = GetNeighborhoodExplorers();
  std::vector<double> neighborhoods_confidence(explorers.size(), initial_confidence_);
  size_t current_best_neighborhood_index = std::rand() % explorers.size();
  MsCflpCiSolution* current_solution = new MsCflpCiSolution(*Solution);

  while (iter_without_improvement < max_rl_vnd_iter_without_improvement &&
         total_iter < max_rl_vnd_iter_) {
    // Epislon greedy policy
    size_t current_neighborhood_index = 0;
    if ((std::rand() / RAND_MAX) < epsilon_) {
      current_neighborhood_index = current_best_neighborhood_index;
    } else {
      current_neighborhood_index = std::rand() % explorers.size();
    }
    MsCflpCiSolution* new_solution = current_solution;
    // Local Search
    while (true) {
      MsCflpCiSolution* tmp_solution = 
          explorers[current_neighborhood_index]->Explore(
              new_solution, 
              GetAmountTolerance(), 
              GetImprovementTolerance());
      
      if (tmp_solution == nullptr) {
        break;
      }
      delete new_solution;
      new_solution = tmp_solution;
    }
    const double reward = CalculateReward(current_solution, new_solution);
    if (reward == 0) {
      ++iter_without_improvement;
    } else {
      iter_without_improvement = 0;
    }
    ++ total_iter;
    neighborhoods_confidence[current_neighborhood_index] += learning_rate_ * 
                                                            (reward - neighborhoods_confidence[current_neighborhood_index]);
  }

  return current_solution;
}

double  GraspMsCflpCiGvnsRl::CalculateReward(MsCflpCiSolution* previus_solution, MsCflpCiSolution* new_solution) const {
  // Binary reward policy
  // If the new_solution is nullptr, it means no improvement was found
  if (new_solution == nullptr || new_solution->GetTotalCost() >= previus_solution->GetTotalCost())  {
    return 0;
  } 
  return 1;
}