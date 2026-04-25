#include <stdexcept>
#include <vector>
#include <cstdlib>

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
  while (perturbator_index < perturbators_.size()) {
    /// Generate a perturbed solution (may return nullptr if no move found)
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
    const double reward = CalculateReward(previous_cost, current_solution);
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
double GraspMsCflpCiGvnsRl::CalculateReward(double previous_cost, MsCflpCiSolution* new_solution) const {
  if (new_solution == nullptr ||
      new_solution->GetTotalCost() >= previous_cost - GetImprovementTolerance()) {
    return 0.0;
  } 
  return 1.0;
}