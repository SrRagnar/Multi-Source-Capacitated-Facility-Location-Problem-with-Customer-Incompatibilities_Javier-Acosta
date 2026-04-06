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

#include "grasp_ms_cflp_ci_solver.h"
#include "ms_cflp_ci_solution.h"
#include "ms_cflp_ci_instance.h"

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
  const unsigned cardinality_parameter = 2;  // K parameter for LRC.
  while (accumulated_capacity < total_demand) {
    std::vector<int> facilities_by_score = GetSortedFacilitiesByScore(original_instance, *constructive_solution);
    if (facilities_by_score.empty()) {
      break;
    }
    unsigned current_rcl_size = std::min(
        cardinality_parameter,
        static_cast<unsigned>(facilities_by_score.size()));

    unsigned facility_index = std::rand() % current_rcl_size;
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
        cardinality_parameter,
        static_cast<unsigned>(facilities_by_score.size()));

    unsigned facility_index = std::rand() % current_rcl_size;
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
    size_t factory_index = 0;
    while (customer_demands[customer_id] >= 1e-6) {
      const std::vector<int>& facilities_by_current_cost = GetSortedFacilitiesByCostForCustomer(*constructive_solution, customer_id);
      if (facilities_by_current_cost.empty()) {
        // Not feasible to assign this customer, but we should have enough slack to handle this.
        delete constructive_solution;
        return nullptr;
      }
      unsigned rcl_size = std::min(
          cardinality_parameter, 
          static_cast<unsigned>(facilities_by_current_cost.size()));
      factory_index = std::rand() % rcl_size;

      const int facility = facilities_by_current_cost[factory_index];
      const double amount_to_assign = std::min(
          constructive_solution->GetResidualCapacity(facility),
          customer_demands[customer_id]);
          
      constructive_solution->AddFlow(customer_id, facility, amount_to_assign);
      customer_demands[customer_id] -= amount_to_assign;
    }
  }

  ++grasp_iterations_;
  return constructive_solution;
}

/**
 * @brief Postprocesses the constructed solution by exploring its neighborhood.
 *
 * @param solution The solution to postprocess.
 * @return A pointer to the improved solution after postprocessing.
 */
#include <iostream>
Solution* GraspMsCflpCiSolver::Postprocess(Solution* solution) {
  MsCflpCiSolution* current = dynamic_cast<MsCflpCiSolution*>(solution);
  if (current == nullptr) {
    throw std::invalid_argument("Solution is not of type MsCflpCiSolution.");
  }
  while (true) {
    MsCflpCiSolution* shift_neighbor = ExploreShiftNeighborhood(current);
    if (shift_neighbor != nullptr) {
      std::cout << shift_neighbor->GetTotalCost() << std::endl;
      delete current;
      current = shift_neighbor;
      continue;
    }
    delete shift_neighbor;
    MsCflpCiSolution* swap_neighbor = ExploreClientSwapNeighborhood(current);
    if (swap_neighbor != nullptr) {
      std::cout << swap_neighbor->GetTotalCost() << std::endl;
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

/**
 * @brief Finds the slack value for the instance.
 *
 * @param instance The instance for which to find the slack value.
 * @return The slack value.
 */
unsigned GraspMsCflpCiSolver::FindSlackValue(const MsCflpCiInstance& instance) const {
  const unsigned total_incompatibilities = instance.GetIncompatibilityPairs().size();
  if (total_incompatibilities == 0) {
    return 0;  // No slack needed if there are no incompatibilities.
  }
  // Heuristic: set slack as a small percentage of the total incompatibilities, with a maximum cap.
  const unsigned slack = static_cast<unsigned>(total_incompatibilities * 0.1);  // 10% of total incompatibilities
  return std::min(slack, static_cast<unsigned>(instance.GetFacilityCount() * 0.2));  // Cap at 20% of total facilities
}

/**
 * @brief Gets the facilities sorted by their scores.
 *
 * @param instance The instance for which to sort facilities.
 * @param solution The solution for which to evaluate facility scores.
 * @return A vector containing the facility IDs sorted by their scores.
 */
std::vector<int> GraspMsCflpCiSolver::GetSortedFacilitiesByScore(const MsCflpCiInstance& instance, const MsCflpCiSolution& solution) const {
  const double opening_cost_factor = 0.1;
  const double assignment_cost_factor = 0.6;
  const double redundancy_penalty_factor = 0.2;
  const double incompatibility_penalty_factor = 0.1;
  std::vector<std::pair<int, double>> facilities_with_scores;
  const double customer_count = static_cast<double>(instance.GetCustomerCount());

  for (int facility_id = 0; facility_id < instance.GetFacilityCount(); ++facility_id) {
    // Don't consider already open facilities.
    if (solution.IsFacilityOpen(facility_id)) {
      continue;
    }
    // Base score: opening cost relative to capacity.
    const double facility_capacity = instance.GetFacilityCapacity(facility_id);
    double score = opening_cost_factor * (instance.GetFacilityOpeningCost(facility_id) / (facility_capacity + 1e-9));
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
        redundancy_penalty += weighted_cost * 0.5;  // Half penalty for redundancy.
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
        && solution.GetResidualCapacity(facility_id) > 0 
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
 * @brief Explores the neighborhood of the current solution by shifting the assignment of a customer from one facility to another.
 *
 * @param solution The current solution to explore.
 * @return A pointer to the best neighboring solution found, or nullptr if no improving neighbor exists.
 */
MsCflpCiSolution* GraspMsCflpCiSolver::ExploreShiftNeighborhood(MsCflpCiSolution* solution) const {
  MsCflpCiSolution* best_solution = nullptr;
  const int customer_count = solution->GetCustomerCount();

  for (int customer_id = 0; customer_id < customer_count; ++customer_id) {
    const std::vector<int>& current_facilities = solution->GetFacilitiesOf()[customer_id];
    for (int source_facility : current_facilities) {
      const std::vector<int>& other_facilities = solution->GetInstance().GetFacilitiesSortedByCostForCustomer(customer_id);
      for (int target_facility : other_facilities) {
        if (source_facility != target_facility) {
          double amount = solution->GetCustomerFacilityFraction(customer_id, source_facility) * solution->GetInstance().GetCustomerDemand(customer_id);
          // Checks if the new solution would be feasible.
          if (solution->CanShiftFlow(customer_id, source_facility, target_facility, amount)) {
            double delta = solution->EvaluateShiftDelta(customer_id, source_facility, target_facility, amount);
            // Only consider improving moves, without calculating the whole new objective value.
            if (delta < -1e-6) {
              std::cout << "Found improving shift: Customer " << customer_id << " from Facility " << source_facility
                        << " to Facility " << target_facility
                        << " (Delta: " << delta << ")" << std::endl;
              MsCflpCiSolution* new_solution = new MsCflpCiSolution(*solution);
              new_solution->ShiftFlow(customer_id, source_facility, target_facility, amount);
               // Compare against best neighbor found so far if there is one.
              if (best_solution == nullptr || new_solution->GetTotalCost() < best_solution->GetTotalCost()) {
                delete best_solution;
                best_solution = new_solution;
              } else {
                delete new_solution;
              }
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
          if (delta < -1e-6) {
            std::cout << "Found improving swap: Customer " << customer_a << " from Facility " << facility_a
                      << " with Customer " << customer_b << " from Facility " << facility_b
                      << " (Delta: " << delta << ")" << std::endl;
            MsCflpCiSolution* new_solution = new MsCflpCiSolution(*solution);
            double amount_a = solution->GetCustomerFacilityFraction(customer_a, facility_a) * 
                              solution->GetInstance().GetCustomerDemand(customer_a);
            double amount_b = solution->GetCustomerFacilityFraction(customer_b, facility_b) * 
                              solution->GetInstance().GetCustomerDemand(customer_b);
            new_solution->RemoveFlow(customer_a, facility_a, amount_a);
            new_solution->RemoveFlow(customer_b, facility_b, amount_b);
            new_solution->AddFlow(customer_a, facility_b, amount_a);
            new_solution->AddFlow(customer_b, facility_a, amount_b);
            // Compare against best neighbor found so far if there is one.
            if (best_neighbor == nullptr || new_solution->GetTotalCost() < best_neighbor->GetTotalCost()) {
              delete best_neighbor;
              best_neighbor = new_solution;
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