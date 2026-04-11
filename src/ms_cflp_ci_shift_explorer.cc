// University of La Laguna
// School of Engineering and Technology
// Bachelor's Degree in Computer Engineering
// Subject: Design and Analysis of Algorithms
// Course: 3rd
// Practice 6: Multi-Source Capacitated Facility Location Problem with Customer
// Incompatibilities
// Author: Javier Acosta Portocarrero
// Date: 10/04/2026
// File ms_cflp_ci_shift_explorer.cc: implementation file.
// Contains the implementation of the MsCflpCiShiftExplorer class.

#include <algorithm>
#include <iostream>
#include <stdexcept>
#include <cmath>

#include "ms_cflp_ci_shift_explorer.h"
 
MsCflpCiSolution* MsCflpCiShiftExplorer::Explore(const MsCflpCiSolution* solution, 
                                                 double amount_tol, double improvement_tol) const {
  const int customer_count = solution->GetCustomerCount();
  const double no_improving_delta = 0.0;
  int best_customer_id = -1;
  int best_source_facility = -1;
  int best_target_facility = -1;
  double best_shift_amount = 0.0;
  double best_delta = no_improving_delta;

  for (int customer_id = 0; customer_id < customer_count; ++customer_id) {
    const std::vector<int> source_facilities = solution->GetFacilitiesOf()[customer_id];
    const std::vector<int> target_facilities = GetSortedFacilitiesByCostForCustomer(*solution, customer_id, 
                                                                                    amount_tol, improvement_tol);
    for (int source_facility : source_facilities) {
      const double assigned_amount =
          solution->GetCustomerFacilityFraction(customer_id, source_facility) *
          solution->GetInstance().GetCustomerDemand(customer_id);
      if (assigned_amount <= amount_tol) {
        continue;
      }
      // Use only feasible facilities in the current solution.
      for (int target_facility : target_facilities) {
        if (source_facility == target_facility) {
          continue;
        }
        // Shift the whole possible amount: all current flow or all remaining target capacity.
        const double shift_amount = std::min(assigned_amount, solution->GetResidualCapacity(target_facility));
        if (shift_amount <= amount_tol) {
          continue;
        }
        if (!CanShiftFlow(*solution, customer_id, source_facility, target_facility, shift_amount, 
                                    amount_tol, improvement_tol)) {
          continue;
        }
        const double shift_delta = EvaluateShiftDelta(*solution, customer_id, source_facility, 
                                                               target_facility, shift_amount, amount_tol, 
                                                               improvement_tol);
        if (shift_delta < best_delta - improvement_tol) {
          best_customer_id = customer_id;
          best_source_facility = source_facility;
          best_target_facility = target_facility;
          best_shift_amount = shift_amount;
          best_delta = shift_delta;
        }
      }
    }
  }
  if (best_customer_id == -1) {
    return nullptr;
  }
  MsCflpCiSolution* best_solution = new MsCflpCiSolution(*solution);
  if (!ShiftFlow(*best_solution, best_customer_id, best_source_facility, best_target_facility, best_shift_amount,
                  amount_tol, improvement_tol)) {
    delete best_solution;
    std::cerr << "ERROR: Failed to apply the best swap move found." << std::endl;
    return nullptr;
  }

  //std::cout << "Shift improvement " << best_delta << std::endl;
  return best_solution;
}

/**
 * @brief Gets the facilities sorted by their assignment cost for a given customer.
 *
 * @param solution The solution for which to evaluate costs.
 * @param customer_id Customer identifier.
 * @param amount_tol Numerical tolerance for flow amounts.
 * @param improvement_tol Numerical tolerance for improvement acceptance.
 * @return A vector containing the facility IDs sorted by their assignment cost for the given customer.
 */
std::vector<int> MsCflpCiShiftExplorer::GetSortedFacilitiesByCostForCustomer(const MsCflpCiSolution& solution, 
                                                                             int customer_id, double amount_tol, 
                                                                             double improvement_tol) const {
  std::vector<std::pair<int, double>> facilities_with_costs;
  for (int facility_id = 0; facility_id < solution.GetInstance().GetFacilityCount(); ++facility_id) {
    if (solution.GetResidualCapacity(facility_id) <= amount_tol) {
      continue;
    }
    if (solution.IsFacilityOpen(facility_id)) {
      if (solution.CanAssignCustomerToFacility(customer_id, facility_id) ||
          solution.IsCustomerServedByFacility(customer_id, facility_id)) {
        facilities_with_costs.emplace_back(facility_id, solution.GetInstance().GetAssignmentCost(customer_id, facility_id));
      }
      continue;
    }
    if (solution.GetIncompatibilityCounter()[customer_id][facility_id] == 0) {
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
 * @brief Checks whether a shift move is feasible.
 *
 * @param solution The solution for which to check feasibility.
 * @param customer_id Customer identifier.
 * @param source_facility Source facility.
 * @param target_facility Target facility.
 * @param amount Amount of demand units to shift.
 * @param amount_tol Numerical tolerance for flow amounts.
 * @param improvement_tol Numerical tolerance for improvement acceptance.
 * 
 * @return True if the move is feasible, false otherwise.
 */

bool MsCflpCiShiftExplorer::CanShiftFlow(const MsCflpCiSolution& solution, 
                    int customer_id, int source_facility, int target_facility, double amount,
                    double amount_tol, double improvement_tol) const {
  if (!solution.IsValidCustomerId(customer_id) ||
      !solution.IsValidFacilityId(source_facility) ||
      !solution.IsValidFacilityId(target_facility)) {
    return false;
  }
  if (source_facility == target_facility) {
    return false;
  }
  if (amount <= amount_tol) {
    return false;
  }
  const double source_amount = solution.GetAssignedAmount(customer_id, source_facility);
  const std::vector<std::vector<bool>>& assignment = solution.GetAssignments();
  if (!assignment[customer_id][source_facility] ||
      amount - source_amount > amount_tol) {
    return false;
  }
  const std::vector<std::vector<int>>& incompatibility_count = solution.GetIncompatibilityCounter();
  if (!assignment[customer_id][target_facility] &&
      incompatibility_count[customer_id][target_facility] != 0) {
    return false;
  }
  if (amount - solution.GetResidualCapacity(target_facility) > amount_tol) {
    return false;
  }

  return true;
}


/**
 * @brief Evaluates the objective delta of a shift move.
 *
 * This method only evaluates the variation in transport cost.
 *
 * @param solution The solution for which to evaluate the move.
 * @param customer_id Customer identifier.
 * @param source_facility Source facility.
 * @param target_facility Target facility.
 * @param amount Amount of demand units to shift.
 * @param amount_tol Numerical tolerance for flow amounts.
 * @param improvement_tol Numerical tolerance for improvement acceptance.
 * 
 * @return Delta value of the move.
 */
double MsCflpCiShiftExplorer::EvaluateShiftDelta(const MsCflpCiSolution& solution, 
                            int customer_id, int source_facility, int target_facility, double amount,
                            double amount_tol, double improvement_tol) const {
  if (!CanShiftFlow(solution, customer_id, source_facility, target_facility, amount, amount_tol, improvement_tol)) {
    throw std::invalid_argument("Infeasible shift move.");
  }

  const MsCflpCiInstance& instance = solution.GetInstance();
  double delta = amount * (instance.GetAssignmentCost(customer_id, target_facility) -
                 instance.GetAssignmentCost(customer_id, source_facility));

  const std::vector<bool>& factory_open = solution.GetOpenFacilities();
  if (!factory_open[target_facility]) {
    delta += instance.GetFacilityOpeningCost(target_facility);
  }
  const double source_amount = solution.GetAssignedAmount(customer_id, source_facility);
  const std::vector<std::vector<int>>& clients_of = solution.GetCustomersInFacility();
  if (std::fabs(source_amount - amount) <= amount_tol && clients_of[source_facility].size() == 1) {
    delta -= instance.GetFacilityOpeningCost(source_facility);
  }

  return delta;
}

/**
 * @brief Applies a shift move between two facilities.
 *
 * @param solution The solution to which to apply the move.
 * @param customer_id Customer identifier.
 * @param source_facility Source facility.
 * @param target_facility Target facility.
 * @param amount Amount of demand units to shift.
 * @param amount_tol Numerical tolerance for flow amounts.
 * @param improvement_tol Numerical tolerance for improvement acceptance.
 *
 * @return True if the move is applied, false otherwise.
 */
bool MsCflpCiShiftExplorer::ShiftFlow(MsCflpCiSolution& solution, 
                 int customer_id, int source_facility, int target_facility, double amount,
                 double amount_tol, double improvement_tol) const {
  if (!CanShiftFlow(solution, customer_id, source_facility, target_facility, amount, amount_tol, improvement_tol)) {
    return false;
  }

  const std::vector<bool>& factory_open = solution.GetOpenFacilities();
  const bool target_was_closed = !factory_open[target_facility];
  if (target_was_closed && !solution.OpenFacility(target_facility)) {
    return false;
  }
  if (!solution.RemoveFlow(customer_id, source_facility, amount)) {
    if (target_was_closed) {
      solution.CloseFacility(target_facility);
    }
    return false;
  }
  if (!solution.AddFlow(customer_id, target_facility, amount)) {
    solution.AddFlow(customer_id, source_facility, amount);
    if (target_was_closed) {
      solution.CloseFacility(target_facility);
    }
    return false;
  }
  const std::vector<std::vector<int>>& clients_of = solution.GetCustomersInFacility();
  if (clients_of[source_facility].empty()) {
    if (!solution.CloseFacility(source_facility)) {
      return false;
    }
  }

  return true;
}