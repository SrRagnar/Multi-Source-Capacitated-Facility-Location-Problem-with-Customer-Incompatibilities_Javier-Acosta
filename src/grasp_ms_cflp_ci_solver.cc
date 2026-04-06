// University of La Laguna
// School of Engineering and Technology
// Bachelor's Degree in Computer Engineering
// Subject: Design and Analysis of Algorithms
// Course: 3rd
// Practice 6: Multi-Source Capacitated Facility Location Problem with Customer Incompatibilities
// Author: Javier Acosta Portocarrero
// Date: 04/04/2026
// File grasp-ms-cflp-ci-solver.cc: implementation file.
// Contains the implementation of the GraspMsCflpCiSolver class.

#include <vector>
#include <stdexcept>
#include <algorithm>
#include <random>
#include <iostream>
#include <cmath>

#include "grasp_ms_cflp_ci_solver.h"
#include "ms_cflp_ci_solution.h"
#include "ms_cflp_ci_instance.h"

namespace {
  constexpr double kAmountTolerance = 1e-8;
  constexpr double kImprovementTolerance = 1e-8;
}  

/**
 * @brief Preprocesses the input instance.
 *
 * @param input The input instance to preprocess.
 */
void GraspMsCflpCiSolver::Preprocess(Instance* input) {
  return;
}

/**
 * @brief Constructs a solution for the input instance.
 *
 * @param input The input instance for which to construct a solution.
 * @return A pointer to the constructed solution.
 */
Solution* GraspMsCflpCiSolver::ConstructSolution(Instance* input) {
  MsCflpCiInstance* instance = dynamic_cast<MsCflpCiInstance*>(input);
  if (instance == nullptr) {
    throw std::invalid_argument("Input instance is not of type MsCflpCiInstance.");
  }
  MsCflpCiSolution* constructive_solution = new MsCflpCiSolution(*instance);
  const MsCflpCiInstance& original_instance = constructive_solution->GetInstance();
  
  // Fase 1, Facilities selection.
  const double total_demand = original_instance.GetTotalDemand();
  double accumulated_capacity = 0.0;
  while (accumulated_capacity + kAmountTolerance < total_demand) {
    std::vector<int> facilities_by_score = GetSortedFacilitiesByScore(original_instance, *constructive_solution);
    if (facilities_by_score.empty()) {
      break;
    }
    unsigned current_rcl_size = std::min(
        lcr_cardinality_,
        static_cast<unsigned>(facilities_by_score.size()));

    unsigned facility_index = static_cast<unsigned>(std::rand()) % current_rcl_size;
    int facility = facilities_by_score[facility_index];
    if (!constructive_solution->IsFacilityOpen(facility)) {
      constructive_solution->OpenFacility(facility);
      accumulated_capacity += original_instance.GetFacilityCapacity(facility);
    }
  }
  // Add a slack to treat possible incompatibilities.
  unsigned slack_parameter = FindSlackValue(original_instance);
  while (slack_parameter > 0) {
    std::vector<int> facilities_by_score = GetSortedFacilitiesByScore(original_instance, *constructive_solution);
    if (facilities_by_score.empty()) {
      break;
    }
    unsigned current_rcl_size = std::min(
        lcr_cardinality_,
        static_cast<unsigned>(facilities_by_score.size()));

    unsigned facility_index = static_cast<unsigned>(std::rand()) % current_rcl_size;
    int facility = facilities_by_score[facility_index];
    if (!constructive_solution->IsFacilityOpen(facility)) {
      constructive_solution->OpenFacility(facility);
      --slack_parameter;
    }
  }
  
  // Fase 2, Customer assignment.
  std::vector<double> customer_demands = original_instance.GetCustomerDemands();
  for (size_t customer_id = 0; customer_id < original_instance.GetCustomerCount(); ++customer_id) {
    // LRC for customer assignment.
    while (customer_demands[customer_id] > kAmountTolerance) {
      const std::vector<int>& facilities_by_current_cost = GetSortedFacilitiesByCostForCustomer(*constructive_solution, customer_id);
      if (facilities_by_current_cost.empty()) {
        // Not feasible to assign this customer, but we should have enough slack to handle this.
        delete constructive_solution;
        return nullptr;
      }
      unsigned rcl_size = std::min(
          lcr_cardinality_, 
          static_cast<unsigned>(facilities_by_current_cost.size()));
      unsigned factory_index = static_cast<unsigned>(std::rand()) % rcl_size;

      const int facility = facilities_by_current_cost[factory_index];
      const double amount_to_assign = std::min(
          constructive_solution->GetResidualCapacity(facility),
          customer_demands[customer_id]);

      if (!constructive_solution->AddFlow(customer_id, facility, amount_to_assign)) {
        delete constructive_solution;
        return nullptr;
      }
      customer_demands[customer_id] -= amount_to_assign;
    }
  }

  ++grasp_iterations_;
  //std::cout << " 11111" << std::endl;
  return constructive_solution;
}

/**
 * @brief Postprocesses the constructed solution by exploring its neighborhood.
 *
 * @param solution The solution to postprocess.
 * @return A pointer to the improved solution after postprocessing.
 */
Solution* GraspMsCflpCiSolver::Postprocess(Solution* solution) {
  //return new MsCflpCiSolution(*dynamic_cast<MsCflpCiSolution*>(solution));
  MsCflpCiSolution* current = dynamic_cast<MsCflpCiSolution*>(solution);
  if (current == nullptr) {
    throw std::invalid_argument("Solution is not of type MsCflpCiSolution.");
  }
  //const int kMaxLocalIterations = 1000;
  //int iteration = 0;
  while (true) {
    //std::cout << "2222" << std::endl;
    //++iteration;
    MsCflpCiSolution* shift_neighbor = ExploreShiftNeighborhood(current);
    if (shift_neighbor != nullptr) {
      delete current;
      current = shift_neighbor;
      continue;
    }
    delete shift_neighbor;
    MsCflpCiSolution* swap_neighbor = ExploreClientSwapNeighborhood(current);
    if (swap_neighbor != nullptr) {
      delete current;
      current = swap_neighbor;
      continue;
    }
    delete swap_neighbor;
    break;
  }

  Solution* final_solution = new MsCflpCiSolution(*current);
  delete current;
  return final_solution;
}

/**
 * @brief Updates the best solution found so far.
 *
 * @param current The current solution to compare against the best.
 * @param best A reference to the best solution found so far.
 */
void GraspMsCflpCiSolver::UpdateBest(Solution* current, Solution*& best) {
  MsCflpCiSolution* current_solution = dynamic_cast<MsCflpCiSolution*>(current);
  MsCflpCiSolution* best_solution = dynamic_cast<MsCflpCiSolution*>(best);
  if (current_solution == nullptr) {
    throw std::invalid_argument("Current solution is not of type MsCflpCiSolution.");
  }
  if (best_solution == nullptr || current_solution->GetTotalCost() < best_solution->GetTotalCost()) {
    delete best;
    best = new MsCflpCiSolution(*current_solution);
  }
}

/**
 * @brief Checks if the stopping criterion for the GRASP algorithm is met.
 *
 * @return true if the stopping criterion is met, false otherwise.
 */
bool GraspMsCflpCiSolver::StopCriterion() {
  if (grasp_iterations_ >= max_grasp_iterations_) {
    grasp_iterations_ = 0;
    return true;
  }
  return false;
}

unsigned GraspMsCflpCiSolver::FindSlackValue(const MsCflpCiInstance& instance) const {
  const unsigned total_incompatibilities = instance.GetIncompatibilityPairs().size();
  if (total_incompatibilities == 0) {
    return 0;  // No slack needed if there are no incompatibilities.
  }
  // Simple and conservative heuristic:
  // open at least one extra facility when incompatibilities exist,
  // and increase slack smoothly for denser incompatible instances.
  unsigned slack = 1 + total_incompatibilities / instance.GetCustomerCount();
  // Cap the slack so that it does not open too many extra facilities.
  const unsigned facility_count = instance.GetFacilityCount();
  const unsigned max_slack = std::max(1u, facility_count / 5);
  return std::min(slack, max_slack);
}

/**
 * @brief Gets the facilities sorted by their scores.
 *
 * @param instance The instance for which to sort facilities.
 * @param solution The solution for which to evaluate facility scores.
 * @return A vector containing the facility IDs sorted by their scores.
 */
std::vector<int> GraspMsCflpCiSolver::GetSortedFacilitiesByScore(const MsCflpCiInstance& instance, const MsCflpCiSolution& solution) const {
  const double opening_cost_factor = 2.0;
  const double assignment_cost_factor = 20.0;
  const double redundancy_penalty_factor = 0.1;
  const double incompatibility_penalty_factor = 20.0;
  std::vector<std::pair<int, double>> facilities_with_scores;
  const double customer_count = static_cast<double>(instance.GetCustomerCount());

  for (int facility_id = 0; facility_id < instance.GetFacilityCount(); ++facility_id) {
    // Don't consider already open facilities.
    if (solution.IsFacilityOpen(facility_id)) {
      continue;
    }
    // Base score: opening cost relative to capacity.
    const double facility_capacity = instance.GetFacilityCapacity(facility_id);
    double score = opening_cost_factor * instance.GetFacilityOpeningCost(facility_id);
    double assignment_score = 0.0;
    double redundancy_penalty = 0.0;
    double incompatibility_penalty = 0.0;
    // Evaluate all customers.
    for (int customer_id = 0; customer_id < instance.GetCustomerCount(); ++customer_id) {
      const double demand = instance.GetCustomerDemand(customer_id);
      const double cost = instance.GetAssignmentCost(customer_id, facility_id);
      const double weighted_cost = cost * demand;
      const std::vector<int>& sorted = instance.GetFacilitiesSortedByCostForCustomer(customer_id);
      bool already_covered = false;
      // Check if customer already has a good open facility.
      for (int k = 0; k < static_cast<int>(std::min<size_t>(2, sorted.size())); ++k) {
        if (solution.IsFacilityOpen(sorted[k])) {
          already_covered = true;
          break;
        }
      }
      // Assignment contribution.
      if (!already_covered) {
        assignment_score += weighted_cost;  
      } else {
        // Penalize redundant facilities.
        redundancy_penalty += weighted_cost;
      }
      // Penalize incompatible customers not yet covered.
      if (!already_covered) {
        incompatibility_penalty += demand * instance.GetIncompatibleCustomers(customer_id).size();
      }
    }
    // Normalize to keep scales comparable.
    assignment_score /= (customer_count + 1.0);
    redundancy_penalty /= (customer_count + 1.0);
    incompatibility_penalty /= (customer_count + 1.0);
    score += assignment_cost_factor * assignment_score;
    score += redundancy_penalty_factor * redundancy_penalty;
    score += incompatibility_penalty_factor * incompatibility_penalty;

    facilities_with_scores.emplace_back(facility_id, score);
  }
  std::sort(facilities_with_scores.begin(), facilities_with_scores.end(),
            [](const std::pair<int, double>& a, const std::pair<int, double>& b) {
              return a.second < b.second;
            });

  std::vector<int> facilities_by_score;
  for (const auto& pair : facilities_with_scores) {
    facilities_by_score.push_back(pair.first);
  }

  return facilities_by_score;
}

/**
 * @brief Gets the facilities sorted by their costs for a given customer.
 *
 * @param solution The solution for which to sort facilities.
 * @param customer_id The ID of the customer for which to sort facilities.
 * @return A vector containing the facility IDs sorted by their costs.
 */
std::vector<int> GraspMsCflpCiSolver::GetSortedFacilitiesByCostForCustomer(const MsCflpCiSolution& solution, int customer_id) const {
  std::vector<std::pair<int, double>> facilities_with_costs;
  for (int facility_id = 0; facility_id < solution.GetInstance().GetFacilityCount(); ++facility_id) {
    if (solution.IsFacilityOpen(facility_id) 
        && solution.GetResidualCapacity(facility_id) > kAmountTolerance
        && solution.CanAssignCustomerToFacility(customer_id, facility_id)) {
      facilities_with_costs.emplace_back(facility_id, solution.GetInstance().GetAssignmentCost(customer_id, facility_id));
    }
  }
  std::sort(facilities_with_costs.begin(), facilities_with_costs.end(),
            [](const std::pair<int, double>& a, const std::pair<int, double>& b) {
              return a.second < b.second;
            });
  std::vector<int> facilities_by_cost;
  for (const auto& pair : facilities_with_costs) {
    facilities_by_cost.push_back(pair.first);
  }

  return facilities_by_cost;
}

/**
 * @brief Explores the neighborhood by shifting flow between facilities.
 *
 * This version:
 * - Uses only feasible target facilities (open, compatible, with capacity).
 * - Tries partial shifts (not only full reassignment).
 */
MsCflpCiSolution* GraspMsCflpCiSolver::ExploreShiftNeighborhood(MsCflpCiSolution* solution) const {
  MsCflpCiSolution* best_solution = nullptr;
  const int customer_count = solution->GetCustomerCount();
  // Fractions of flow to test (partial shifts).
  const std::vector<double> shift_fractions = {0.25, 0.50, 0.75, 1.00};

  for (int customer_id = 0; customer_id < customer_count; ++customer_id) {
    const std::vector<int>& current_facilities = solution->GetFacilitiesOf()[customer_id];
    for (int source_facility : current_facilities) {
      const double current_amount =
          solution->GetCustomerFacilityFraction(customer_id, source_facility) *
          solution->GetInstance().GetCustomerDemand(customer_id);
      if (current_amount <= kAmountTolerance) {
        continue;
      }
      // Use only feasible facilities in the current solution.
      const std::vector<int> target_facilities = GetSortedFacilitiesByCostForCustomer(*solution, customer_id);
      for (int target_facility : target_facilities) {
        if (source_facility == target_facility) {
          continue;
        }
        // Maximum shift limited by source flow and target capacity.
        const double max_shift_amount = std::min(
            current_amount,
            solution->GetResidualCapacity(target_facility));
        if (max_shift_amount <= kAmountTolerance) {
          continue;
        }
        for (double shift_fraction : shift_fractions) {
          const double amount = max_shift_amount * shift_fraction;
          if (amount <= kAmountTolerance) {
            continue;
          }
          if (!solution->CanShiftFlow(customer_id, source_facility, target_facility, amount)) {
            continue;
          }
          const double delta = solution->EvaluateShiftDelta(customer_id, source_facility, target_facility, amount);
          if (delta < -kImprovementTolerance) {
            MsCflpCiSolution* new_solution = new MsCflpCiSolution(*solution);
            if (!new_solution->ShiftFlow(customer_id, source_facility, target_facility, amount)) {
              delete new_solution;
              continue;
            }
            const double real_improvement = solution->GetTotalCost() - new_solution->GetTotalCost();
            if (real_improvement > kImprovementTolerance) {
              //std::cout << "Found improving shift " << -real_improvement << std::endl;
              delete best_solution;
              best_solution = new_solution;
              return best_solution; 
            } else {
              delete new_solution;
            }
          }
        }
      }
    }
  }

  return best_solution;
}

/**
 * @brief Explores the neighborhood of the current solution by swapping the assignments of two customers.
 *
 * @param solution The current solution to explore.
 * @return A pointer to the best neighboring solution found, or nullptr if no improving neighbor exists.
 */
MsCflpCiSolution* GraspMsCflpCiSolver::ExploreClientSwapNeighborhood(MsCflpCiSolution* solution) const {
  MsCflpCiSolution* best_neighbor = nullptr;
  const int customer_count = solution->GetCustomerCount();
  for (int customer_a = 0; customer_a < customer_count; ++customer_a) {
    // Avoid duplicate pairs and self-swapping.
    for (int customer_b = customer_a + 1; customer_b < customer_count; ++customer_b) {
      const std::vector<int>& facilities_a = solution->GetFacilitiesOf()[customer_a];
      const std::vector<int>& facilities_b = solution->GetFacilitiesOf()[customer_b];
      for (int facility_a : facilities_a) {
        for (int facility_b : facilities_b) {
          if (facility_a == facility_b) {
            continue;
          }
          // Check feasibility of the swap.
          if (!solution->CanSwapCustomersBetweenFacilities(customer_a, facility_a, customer_b, facility_b)) {
            continue;
          }
          double delta = solution->EvaluateSwapDelta(customer_a, facility_a, customer_b, facility_b);
          // Only consider improving moves, without calculating the whole new objective value.
          if (delta < -kImprovementTolerance) {
            MsCflpCiSolution* new_solution = new MsCflpCiSolution(*solution);
            double amount_a = solution->GetCustomerFacilityFraction(customer_a, facility_a) * 
                              solution->GetInstance().GetCustomerDemand(customer_a);
            double amount_b = solution->GetCustomerFacilityFraction(customer_b, facility_b) * 
                              solution->GetInstance().GetCustomerDemand(customer_b);
            if (!new_solution->RemoveFlow(customer_a, facility_a, amount_a) ||
                !new_solution->RemoveFlow(customer_b, facility_b, amount_b) ||
                !new_solution->AddFlow(customer_a, facility_b, amount_a) ||
                !new_solution->AddFlow(customer_b, facility_a, amount_b)) {
              // This should never happen.
              delete new_solution;
              continue;
            }
            const double real_improvement = solution->GetTotalCost() - new_solution->GetTotalCost();
            // Compare against best neighbor found so far if there is one.
            if (real_improvement > kImprovementTolerance) {
              //std::cout << "Found improving swap " << -real_improvement << std::endl;
              delete best_neighbor;
              best_neighbor = new_solution;
              return best_neighbor;
            } else {
              delete new_solution;
            }
          }
        }
      }
    }
  }
  return best_neighbor;
}