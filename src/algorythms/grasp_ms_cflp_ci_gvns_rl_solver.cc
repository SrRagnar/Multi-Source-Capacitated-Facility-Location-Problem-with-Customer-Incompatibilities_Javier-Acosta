// University of La Laguna
// School of Engineering and Technology
// Bachelor's Degree in Computer Engineering
// Subject: Design and Analysis of Algorithms
// Course: 3rd
// Practice 6: Multi-Source Capacitated Facility Location Problem with Customer Incompatibilities
// Author: Javier Acosta Portocarrero
// Date: 25/04/2026
// File grasp-ms-cflp-ci-gvns_rl_solver.cc: implementation file.
// Contains the implementation of the GraspMsCflpCiGvnsRlSolver class.

#include <stdexcept>
#include <vector>
#include <cstdlib>
#include <iostream>

#include "grasp_ms_cflp_ci_gvns_rl_solver.h"

/**
 * @brief Applies a GVNS with RL to improve an initial solution.
 *
 * Uses a set of perturbators and a VND with reinforcement learning.
 * Only accepts new solutions if they improve the incumbent.
 *
 * @param solution Initial solution.
 * @return Improved solution (ownership transferred to caller).
 */
Solution* GraspMsCflpCiGvnsRl::Postprocess(Solution* solution) {
  MsCflpCiSolution* current_solution = dynamic_cast<MsCflpCiSolution*>(solution);
  if (current_solution == nullptr) {
    throw std::invalid_argument("Solution is not of type MsCflpCiSolution.");
  }
 
  size_t perturbator_index = 0;
  //unsigned iter = 0;
  while (perturbator_index < perturbators_.size()) {
    /// Generate a perturbed solution (may return nullptr if no move found)
    //std::cout << "GVNS iteration: " << iter++ << std::endl;
    MsCflpCiSolution* new_solution = perturbators_[perturbator_index++]->
        Perturbate(current_solution, GetAmountTolerance(), GetImprovementTolerance());
    if (new_solution == nullptr) {
      continue;
    }
    /// Apply VND with RL over the perturbed solution
    new_solution = VndWithReinforcementLearning(new_solution);
    /// Accept only if strictly better than current solution
    if (new_solution != nullptr &&
        new_solution->GetTotalCost() < current_solution->GetTotalCost() - GetImprovementTolerance()) {
      delete current_solution;
      current_solution = new_solution;
      perturbator_index = 0;
      continue;
    }
    delete new_solution;
  }

  /// Return final solution without extra copy
  return current_solution;
}

/**
 * @brief Applies a VND guided by reinforcement learning.
 *
 * Uses an epsilon-greedy policy to select neighborhoods and updates
 * confidence values based on observed improvements.
 *
 * @param solution Initial solution (ownership transferred).
 * @return Locally improved solution.
 */
MsCflpCiSolution* GraspMsCflpCiGvnsRl::VndWithReinforcementLearning(MsCflpCiSolution* solution) const {
  unsigned iter_without_improvement = 0;
  unsigned total_iter = 0;
  const std::vector<MsCflpCiNeighboorhodExplorer*>& explorers = GetNeighborhoodExplorers();
  std::vector<double> neighborhoods_confidence(explorers.size(), initial_confidence_);
  size_t current_best_neighborhood_index = std::rand() % explorers.size();
  MsCflpCiSolution* current_solution = new MsCflpCiSolution(*solution);

  while (iter_without_improvement < max_rl_vnd_iter_without_improvement &&
         total_iter < max_rl_vnd_iter_) {
    /// Epsilon-greedy selection
    size_t current_neighborhood_index = 0;
    const double random_value = static_cast<double>(std::rand()) / RAND_MAX;
    if (random_value < epsilon_) {
      current_neighborhood_index = std::rand() % explorers.size();
    } else {
      current_neighborhood_index = current_best_neighborhood_index;
    }
    const double previous_cost = current_solution->GetTotalCost();
    //unsigned iter = 0;
    /// Local search
    while (true) {
      //std::cout << "iter: " << iter++ << ", neighborhood: " << current_neighborhood_index << std::endl;
      MsCflpCiSolution* tmp_solution = explorers[current_neighborhood_index]->Explore(
          current_solution, GetAmountTolerance(), GetImprovementTolerance());
      if (tmp_solution == nullptr) {
        break;
      }
      delete current_solution;
      current_solution = tmp_solution;
    }
    /// Compute reward based on improvement
    const double reward = CalculateProportionalReward(previous_cost, current_solution);
    if (reward == 0.0) {
      ++iter_without_improvement;
    } else {
      iter_without_improvement = 0;
    }
    /// Update confidence of the selected neighborhood
    neighborhoods_confidence[current_neighborhood_index] += learning_rate_ * 
        (reward - neighborhoods_confidence[current_neighborhood_index]);
    /// Update best neighborhood index
    for (size_t i = 0; i < neighborhoods_confidence.size(); ++i) {
      if (neighborhoods_confidence[i] > neighborhoods_confidence[current_best_neighborhood_index]) {
        current_best_neighborhood_index = i;
      }
    }

    ++total_iter;
  }

  return current_solution;
}

/**
 * @brief Computes reward for RL policy.
 *
 * Binary reward: 1 if improvement, 0 otherwise.
 *
 * @param previous_cost Cost before applying neighborhood.
 * @param new_solution Resulting solution.
 * @return Reward value (0 or 1).
 */
double GraspMsCflpCiGvnsRl::CalculateBinaryReward(double previous_cost, MsCflpCiSolution* new_solution) const {
  if (new_solution == nullptr ||
      new_solution->GetTotalCost() >= previous_cost - GetImprovementTolerance()) {
    return 0.0;
  } 
  return 1.0;
}

/**
 * @brief Computes proportional reward for RL policy.
 *
 * Proportional reward: (previous_cost - new_cost) / previous_cost.
 *
 * @param previous_cost Cost before applying neighborhood.
 * @param new_solution Resulting solution.
 * @return Reward value in [0, 1].
 */
double GraspMsCflpCiGvnsRl::CalculateProportionalReward(double previous_cost, MsCflpCiSolution* new_solution) const {
  if (new_solution == nullptr) {
    return 0.0;
  }
  return (previous_cost  - new_solution->GetTotalCost()) / previous_cost;
}