// University of La Laguna
// School of Engineering and Technology
// Bachelor's Degree in Computer Engineering
// Subject: Design and Analysis of Algorithms
// Course: 3rd
// Practice 6: Multi-Source Capacitated Facility Location Problem with Customer
// Incompatibilities
// Author: Javier Acosta Portocarrero
// Date: 10/04/2026
// File ms_cflp_ci_facilities_swap_explorer.h: declaration file.
// Contains the declaration of the MsCflpCiFacilitiesSwapExplorer class.

#include <iostream>

#include "ms_cflp_ci_facility_swap_explorer.h"

MsCflpCiSolution* MsCflpCiFacilitiesSwapExplorer::Explore(const MsCflpCiSolution* solution, double amount_tol,
                                                          double improvement_tol) const {
  int best_source_facility = -1;
  int best_target_facility = -1;
  const double no_improving_delta = 0.0;
  double best_delta = no_improving_delta;
  const int facility_count = solution->GetFacilityCount();
  std::vector<int> open_facilities;
  std::vector<int> closed_facilities;
  for (int facility_id = 0; facility_id < facility_count; ++facility_id) {
    if (solution->IsFacilityOpen(facility_id)) {
      open_facilities.push_back(facility_id);
    } else {
      closed_facilities.push_back(facility_id);
    }
  }
  
  for (int open_facility : open_facilities) {
    for (int closed_facility : closed_facilities) {
      if (!CanSwapFacilities(*solution, open_facility, closed_facility, amount_tol, improvement_tol)) {
        continue;
      }
      const double swap_delta = EvaluateFacilitiesSwapDelta(*solution, open_facility, closed_facility, 
                                                             amount_tol, improvement_tol);
      if (swap_delta < best_delta - improvement_tol) {
        best_source_facility = open_facility;
        best_target_facility = closed_facility;
        best_delta = swap_delta;
      }
    }
  }

  if (best_source_facility == -1) {
    return nullptr;
  }
  MsCflpCiSolution* best_solution = new MsCflpCiSolution(*solution);
  if (!SwapFacilities(*best_solution, best_source_facility, best_target_facility, amount_tol, improvement_tol)) {
    delete best_solution;
    std::cerr << "Error: failed to apply the best facilities swap move found." << std::endl;
    return nullptr;
  }
  //std::cout << "Facility swap move: " << best_delta << std::endl;
  return best_solution;
}

/**
 * @brief Checks whether two facilities can be swapped.
 *
 * This move is interpreted as:
 * - closing the currently open facility,
 * - opening the currently closed facility,
 * - reassigning all customers served by the open facility to the closed one.
 *
 * @param source_facility First facility identifier.
 * @param target_facility Second facility identifier.
 * @return True if the swap is feasible, false otherwise.
 */
bool MsCflpCiFacilitiesSwapExplorer::CanSwapFacilities(const MsCflpCiSolution& solution, 
                                              int source_facility, int target_facility,
                                              double amount_tol, double improvement_tol) const {
  if (!solution.IsValidFacilityId(source_facility) || !solution.IsValidFacilityId(target_facility)) {
    return false;
  }
  if (source_facility == target_facility) {
    return false;
  }

  int open_facility = -1;
  int closed_facility = -1;
  const std::vector<bool>& factory_open = solution.GetOpenFacilities();
  if (factory_open[source_facility] && !factory_open[target_facility]) {
    open_facility = source_facility;
    closed_facility = target_facility;
  } else if (!factory_open[source_facility] && factory_open[target_facility]) {
    open_facility = target_facility;
    closed_facility = source_facility;
  } else {
    return false;
  }

  const MsCflpCiInstance& instance = solution.GetInstance();
  const double residual_capacity_open = solution.GetResidualCapacity(open_facility);
  const double used_capacity = instance.GetFacilityCapacity(open_facility) - residual_capacity_open;
  if (used_capacity - instance.GetFacilityCapacity(closed_facility) > amount_tol) {
    return false;
  }

  return true;
}

/**
 * @brief Evaluates the objective delta of a facility swap move.
 *
 * This move is interpreted as:
 * - closing the currently open facility,
 * - opening the currently closed facility,
 * - reassigning to the new facility all flow currently served by the old one.
 *
 * @param source_facility First facility identifier.
 * @param target_facility Second facility identifier.
 * @return Delta value of the move.
 */
double MsCflpCiFacilitiesSwapExplorer::EvaluateFacilitiesSwapDelta(const MsCflpCiSolution& solution, 
                                                                   int source_facility, int target_facility,
                                                                   double amount_tol, double improvement_tol) const {
  int open_facility = -1;
  int closed_facility = -1;
  const std::vector<bool>& factory_open = solution.GetOpenFacilities();
  if (factory_open[source_facility] && !factory_open[target_facility]) {
    open_facility = source_facility;
    closed_facility = target_facility;
  } else {
    open_facility = target_facility;
    closed_facility = source_facility;
  }

  double delta = 0.0;
  const MsCflpCiInstance& instance = solution.GetInstance();
  delta += instance.GetFacilityOpeningCost(closed_facility);
  delta -= instance.GetFacilityOpeningCost(open_facility);

  const std::vector<std::vector<int>>& clients_of = solution.GetCustomersInFacility();
  const std::vector<int>& customers_in_open_facility = clients_of[open_facility];
  for (int customer_id : customers_in_open_facility) {
    const double amount = solution.GetAssignedAmount(customer_id, open_facility);
    delta += amount * (instance.GetAssignmentCost(customer_id, closed_facility) -
                       instance.GetAssignmentCost(customer_id, open_facility));
  }

  return delta;
}

/**
 * @brief Applies a facility swap move and returns the resulting solution.
 *
 * This move is interpreted as:
 * - closing the currently open facility,
 * - opening the currently closed facility,
 * - reassigning to the new facility all flow currently served by the old one.
 *
 * @param source_facility First facility identifier.
 * @param target_facility Second facility identifier.
 * @return True if successfully applied, false otherwise.
 */
bool MsCflpCiFacilitiesSwapExplorer::SwapFacilities(MsCflpCiSolution& solution, 
                                                    int source_facility, int target_facility,
                                                    double amount_tol, double improvement_tol) const {
  if (!CanSwapFacilities(solution, source_facility, target_facility, amount_tol, improvement_tol)) {
    return false;
  }
  int open_facility = -1;
  int closed_facility = -1;
  const std::vector<bool>& factory_open = solution.GetOpenFacilities();
  if (factory_open[source_facility] && !factory_open[target_facility]) {
    open_facility = source_facility;
    closed_facility = target_facility;
  } else {
    open_facility = target_facility;
    closed_facility = source_facility;
  }
  if (!solution.OpenFacility(closed_facility)) {
    return false;
  }

  const std::vector<int> customers_in_open_facility = solution.GetCustomersInFacility()[open_facility];
  for (int customer_id : customers_in_open_facility) {
    const double amount =
        solution.GetCustomerFacilityFraction(customer_id, open_facility) *
        solution.GetInstance().GetCustomerDemand(customer_id);
    if (amount <= amount_tol) {
      continue;
    }
    if (!solution.RemoveFlow(customer_id, open_facility, amount) ||
        !solution.AddFlow(customer_id, closed_facility, amount)) {
      return false;
    }
  }
  if (!solution.CloseFacility(open_facility)) {
    return false;
  }

  return true;
}