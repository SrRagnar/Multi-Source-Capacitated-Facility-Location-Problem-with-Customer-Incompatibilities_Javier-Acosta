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
        if (!solution->CanShiftFlow(customer_id, source_facility, target_facility, shift_amount)) {
          continue;
        }
        const double shift_delta = solution->EvaluateShiftDelta(customer_id, source_facility, target_facility, shift_amount);
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
  if (!best_solution->ShiftFlow(
          best_customer_id, best_source_facility, best_target_facility, best_shift_amount)) {
    delete best_solution;
    std::cerr << "ERROR: Failed to apply the best swap move found." << std::endl;
    return nullptr;
  }

  //std::cout << "Shift improvement " << best_delta << std::endl;
  return best_solution;
}

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
