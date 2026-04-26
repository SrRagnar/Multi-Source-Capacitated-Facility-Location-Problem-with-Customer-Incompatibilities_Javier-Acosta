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
  if (perturbators_.empty()) {
    return current_solution;
  }

  unsigned iter = 0;
  unsigned iter_without_improvement = 0;

  while (iter < max_gvns_iter_ &&
         iter_without_improvement < max_gvns_iter_without_improvement_) {
    ++iter;
    unsigned perturbation_level = 1;
    while (perturbation_level <= num_perturbations_) {
      // Perturbation phase
      MsCflpCiSolution* new_solution = new MsCflpCiSolution(*current_solution);
      for (unsigned i = 0; i < perturbation_level; ++i) {
        const size_t perturbator_index = std::rand() % perturbators_.size();
        MsCflpCiSolution* tmp = perturbators_[perturbator_index]->Perturbate(
            new_solution, GetAmountTolerance(), GetImprovementTolerance());

        if (tmp != nullptr) {
          delete new_solution;
          new_solution = tmp;
        }
      }
      /// Apply VND with RL over the perturbed solution
      new_solution = VndWithReinforcementLearning(new_solution);
      /// Accept only if strictly better than current solution
      if (new_solution != nullptr &&
          new_solution->GetTotalCost() < current_solution->GetTotalCost() - GetImprovementTolerance()) {
        delete current_solution;
        current_solution = new_solution;
        iter_without_improvement = 0;
        perturbation_level = 1; 
      } else {
        delete new_solution;
        ++perturbation_level;
      }
    }

    ++iter_without_improvement;
  }

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
  const std::vector<MsCflpCiNeighboorhodExplorer*>& explorers = GetNeighborhoodExplorers();
  if (explorers.empty()) {
    return solution;
  }
  unsigned iter_without_improvement = 0;
  unsigned total_iter = 0;
  std::vector<double> neighborhoods_confidence(explorers.size(), initial_confidence_);
  size_t current_best_neighborhood_index = std::rand() % explorers.size();
  MsCflpCiSolution* current_solution = solution;

  while (iter_without_improvement < max_rl_vnd_iter_without_improvement_ &&
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
    /// Local search
    while (true) {
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
    current_best_neighborhood_index = 0;
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
  if (new_solution == nullptr || previous_cost <= 0.0) {
    return 0.0;
  }
  const double improvement = previous_cost - new_solution->GetTotalCost();
  if (improvement <= GetImprovementTolerance()) {
    return 0.0;
  }
  return improvement / previous_cost;
}

/**
 * @brief Applies a classic VND to a solution.
 *
 * @param solution Initial solution (ownership transferred).
 * @return Locally improved solution.
 */
MsCflpCiSolution* GraspMsCflpCiGvnsRl::ClassicVnd(MsCflpCiSolution* solution) const {
  MsCflpCiSolution* current = solution;
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

  return current;
}

/**
 * @brief Applies a random VND to a solution.
 *
 * @param solution Initial solution (ownership transferred).
 * @return Locally improved solution.
 */
MsCflpCiSolution* GraspMsCflpCiGvnsRl::RVnd(MsCflpCiSolution* solution) const {
  MsCflpCiSolution* current = solution;
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
    MsCflpCiSolution* explored_solution = explorers[explorer_index]->Explore(current, GetAmountTolerance(), GetImprovementTolerance());

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

  return current;
}