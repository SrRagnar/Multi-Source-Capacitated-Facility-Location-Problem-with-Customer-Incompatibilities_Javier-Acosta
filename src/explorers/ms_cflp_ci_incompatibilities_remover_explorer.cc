// University of La Laguna
// School of Engineering and Technology
// Bachelor's Degree in Computer Engineering
// Subject: Design and Analysis of Algorithms
// Course: 3rd
// Practice 6: Multi-Source Capacitated Facility Location Problem with Customer
// Incompatibilities
// Author: Javier Acosta Portocarrero
// Date: 10/04/2026
// File ms_cflp_ci_incompabilities_remover_explorer.cc: implementation file.
// Contains the implementation of the MsCflpCiIncompatibilitiesRemoverExplorer class.

#include <algorithm>
#include <cmath>
#include <iostream>

#include "ms_cflp_ci_incompatibilities_remover_explorer.h"

/**
 * @brief Explores the incompatibility-removal neighborhood.
 *
 * The method first moves a blocking customer and then tries to move
 * unlocked incompatible customers to the released facility if this improves the cost.
 *
 * @param solution Current solution.
 * @param amount_tol Numerical tolerance for flow amounts.
 * @param improvement_tol Numerical tolerance for improvements.
 * @return A new improved solution, or nullptr if no improving move is found.
 */
MsCflpCiSolution* MsCflpCIncompabilitiesRemoverExplorer::Explore(const MsCflpCiSolution* solution,
                                                                 double amount_tol,
                                                                 double improvement_tol) const {
  const std::vector<int> open_facilities = GetOpenFacilitiesSortedByTransportCost(*solution);

  for (int source_facility : open_facilities) {
    const std::vector<int> customers = GetCustomersSortedByBlockingScore(*solution, source_facility, amount_tol);
    for (int customer_id : customers) {
      const double amount =
          solution->GetCustomerFacilityFraction(customer_id, source_facility) *
          solution->GetInstance().GetCustomerDemand(customer_id);
      if (amount <= amount_tol) {
        continue;
      }
      const std::vector<int> target_facilities = GetTargetFacilitiesSortedByAssignmentCost(*solution, customer_id, source_facility, amount, amount_tol);
      for (int target_facility : target_facilities) {
        if (!CanMoveCustomerToFacility(*solution, customer_id, source_facility, target_facility, amount, amount_tol)) {
          continue;
        }
        const double delta = EvaluateMoveCustomerDelta(*solution, customer_id, source_facility, target_facility, amount);
        if (delta < -improvement_tol) {
          MsCflpCiSolution* new_solution = new MsCflpCiSolution(*solution);
          if (!ApplyMoveCustomer(*new_solution, customer_id, source_facility, target_facility, amount)) {
            delete new_solution;
            std::cerr << "ERROR: Incompabilities remove move cant be applied\n";
            continue;
          }
          TryReassignUnlockedCustomers(*new_solution, customer_id, source_facility, amount_tol, improvement_tol);
          //std::cout << "Imcompabilties remove move. " << delta << std::endl;
          return new_solution;
        }
      }
    }
  }

  return nullptr;
}

/**
 * @brief Computes the transport cost currently induced by one facility.
 *
 * @param solution Current solution.
 * @param facility_id Facility identifier.
 * @return Transport cost of the facility.
 */
double MsCflpCIncompabilitiesRemoverExplorer::GetFacilityTransportCost(
    const MsCflpCiSolution& solution, int facility_id) const {
  double transport_cost = 0.0;
  const std::vector<int>& customers = solution.GetCustomersInFacility()[facility_id];

  for (int customer_id : customers) {
    const double amount =
        solution.GetCustomerFacilityFraction(customer_id, facility_id) *
        solution.GetInstance().GetCustomerDemand(customer_id);
    transport_cost += amount * solution.GetInstance().GetAssignmentCost(customer_id, facility_id);
  }

  return transport_cost;
}

/**
 * @brief Counts how many customers would be unblocked by removing one customer.
 *
 * @param solution Current solution.
 * @param customer_id Customer to remove.
 * @param facility_id Facility from which the customer is removed.
 * @return Number of newly unblocked customers.
 */
int MsCflpCIncompabilitiesRemoverExplorer::CountCustomersUnblockedByRemovingCustomer(
    const MsCflpCiSolution& solution, int customer_id, int facility_id) const {
  int unblocked_customers = 0;
  const std::vector<int>& incompatible_customers = solution.GetInstance().GetIncompatibleCustomers(customer_id);

  for (int incompatible_customer : incompatible_customers) {
    if (solution.GetIncompatibilityCounter()[incompatible_customer][facility_id] == 1) {
      ++unblocked_customers;
    }
  }

  return unblocked_customers;
}

/**
 * @brief Checks whether a customer can be moved to a target facility.
 *
 * @param solution Current solution.
 * @param customer_id Customer identifier.
 * @param source_facility Source facility identifier.
 * @param target_facility Target facility identifier.
 * @param amount Amount to move.
 * @param amount_tol Numerical tolerance for flow amounts.
 * @return True if the move is feasible, false otherwise.
 */
bool MsCflpCIncompabilitiesRemoverExplorer::CanMoveCustomerToFacility(
    const MsCflpCiSolution& solution, int customer_id, int source_facility, int target_facility,
    double amount, double amount_tol) const {
  if (source_facility == target_facility) {
    return false;
  }
  if (amount <= amount_tol) {
    return false;
  }
  if (solution.GetResidualCapacity(target_facility) + amount_tol < amount) {
    return false;
  }
  if (solution.IsFacilityOpen(target_facility)) {
    return solution.IsCustomerServedByFacility(customer_id, target_facility) ||
           solution.CanAssignCustomerToFacility(customer_id, target_facility);
  }

  return solution.GetIncompatibilityCounter()[customer_id][target_facility] == 0;
}

/**
 * @brief Evaluates the delta of moving one customer to another facility.
 *
 * @param solution Current solution.
 * @param customer_id Customer identifier.
 * @param source_facility Source facility identifier.
 * @param target_facility Target facility identifier.
 * @param amount Amount to move.
 * @return Objective delta of the move.
 */
double MsCflpCIncompabilitiesRemoverExplorer::EvaluateMoveCustomerDelta(
    const MsCflpCiSolution& solution, int customer_id, int source_facility,
    int target_facility, double amount) const {
  double delta =
      amount * (solution.GetInstance().GetAssignmentCost(customer_id, target_facility) -
                solution.GetInstance().GetAssignmentCost(customer_id, source_facility));

  if (!solution.IsFacilityOpen(target_facility)) {
    delta += solution.GetInstance().GetFacilityOpeningCost(target_facility);
  }
  const double source_amount =
      solution.GetCustomerFacilityFraction(customer_id, source_facility) *
      solution.GetInstance().GetCustomerDemand(customer_id);
  if (std::fabs(source_amount - amount) <= 1e-8 &&
      solution.GetCustomersInFacility()[source_facility].size() == 1) {
    delta -= solution.GetInstance().GetFacilityOpeningCost(source_facility);
  }

  return delta;
}

/**
 * @brief Applies a customer move between two facilities.
 *
 * @param solution Solution to modify.
 * @param customer_id Customer identifier.
 * @param source_facility Source facility identifier.
 * @param target_facility Target facility identifier.
 * @param amount Amount to move.
 * @return True if the move is successfully applied, false otherwise.
 */
bool MsCflpCIncompabilitiesRemoverExplorer::ApplyMoveCustomer(
    MsCflpCiSolution& solution, int customer_id, int source_facility,
    int target_facility, double amount) const {
  if (!solution.IsFacilityOpen(target_facility)) {
    if (!solution.OpenFacility(target_facility)) {
      return false;
    }
  }
  if (!solution.RemoveFlow(customer_id, source_facility, amount)) {
    return false;
  }
  if (!solution.AddFlow(customer_id, target_facility, amount)) {
    return false;
  }
  if (solution.GetCustomersInFacility()[source_facility].empty()) {
    if (!solution.CloseFacility(source_facility)) {
      return false;
    }
  }

  return true;
}

/**
 * @brief Tries to reassign unlocked incompatible customers to the released facility.
 *
 * @param solution Solution to modify.
 * @param moved_blocker_customer Customer that has been removed from the released facility.
 * @param released_facility Facility that has been released.
 * @param amount_tol Numerical tolerance for flow amounts.
 * @param improvement_tol Numerical tolerance for improvements.
 * @return True if at least one reassignment is performed, false otherwise.
 */
bool MsCflpCIncompabilitiesRemoverExplorer::TryReassignUnlockedCustomers(MsCflpCiSolution& solution, int moved_blocker_customer, int released_facility,
                                                                         double amount_tol, double improvement_tol) const {
  bool moved_any_customer = false;
  const std::vector<int>& incompatible_customers = solution.GetInstance().GetIncompatibleCustomers(moved_blocker_customer);
  for (int incompatible_customer : incompatible_customers) {
    if (solution.GetIncompatibilityCounter()[incompatible_customer][released_facility] != 0) {
      continue;
    }
    if (TryMoveCustomerToReleasedFacility(solution, incompatible_customer, released_facility,
                                          amount_tol, improvement_tol)) {
      moved_any_customer = true;
    }
  }

  return moved_any_customer;
}

/**
 * @brief Tries to move one customer to the released facility if the move improves the cost.
 *
 * @param solution Solution to modify.
 * @param customer_id Customer identifier.
 * @param target_facility Released facility identifier.
 * @param amount_tol Numerical tolerance for flow amounts.
 * @param improvement_tol Numerical tolerance for improvements.
 * @return True if the move is applied, false otherwise.
 */
bool MsCflpCIncompabilitiesRemoverExplorer::TryMoveCustomerToReleasedFacility(
    MsCflpCiSolution& solution, int customer_id, int target_facility,
    double amount_tol, double improvement_tol) const {
  const std::vector<std::vector<int>>& facilities_of = solution.GetFacilitiesOf();
  int best_source_facility = -1;
  double best_amount = 0.0;
  double best_delta = 0.0;
  for (int source_facility : facilities_of[customer_id]) {
    if (source_facility == target_facility) {
      continue;
    }
    const double amount =
        solution.GetCustomerFacilityFraction(customer_id, source_facility) *
        solution.GetInstance().GetCustomerDemand(customer_id);
    if (amount <= amount_tol) {
      continue;
    }
    if (!CanMoveCustomerToFacility(solution, customer_id, source_facility, target_facility,
                                   amount, amount_tol)) {
      continue;
    }
    const double delta = EvaluateMoveCustomerDelta(solution, customer_id, source_facility, target_facility, amount);
    if (delta < best_delta - improvement_tol) {
      best_source_facility = source_facility;
      best_amount = amount;
      best_delta = delta;
    }
  }

  if (best_source_facility == -1) {
    return false;
  }
  return ApplyMoveCustomer(solution, customer_id, best_source_facility, target_facility, best_amount);
}

/**
 * @brief Sorts open facilities by decreasing transport cost.
 *
 * @param solution Current solution.
 * @return Open facilities sorted by transport cost.
 */
std::vector<int> MsCflpCIncompabilitiesRemoverExplorer::GetOpenFacilitiesSortedByTransportCost(
    const MsCflpCiSolution& solution) const {
  std::vector<std::pair<int, double>> facilities_with_costs;
  const int facility_count = solution.GetFacilityCount();

  for (int facility_id = 0; facility_id < facility_count; ++facility_id) {
    if (!solution.IsFacilityOpen(facility_id)) {
      continue;
    }
    if (solution.GetCustomersInFacility()[facility_id].empty()) {
      continue;
    }
    facilities_with_costs.emplace_back(facility_id, GetFacilityTransportCost(solution, facility_id));
  }

  std::sort(facilities_with_costs.begin(), facilities_with_costs.end(),
            [](const std::pair<int, double>& first_facility,
               const std::pair<int, double>& second_facility) {
              return first_facility.second > second_facility.second;
            });

  std::vector<int> facilities;
  for (const auto& facility_with_cost : facilities_with_costs) {
    facilities.push_back(facility_with_cost.first);
  }

  return facilities;
}

/**
 * @brief Sorts customers by blocking score inside one facility.
 *
 * @param solution Current solution.
 * @param facility_id Facility identifier.
 * @param amount_tol Numerical tolerance for flow amounts.
 * @return Customers sorted by blocking score.
 */
std::vector<int> MsCflpCIncompabilitiesRemoverExplorer::GetCustomersSortedByBlockingScore(
    const MsCflpCiSolution& solution, int facility_id, double amount_tol) const {
  std::vector<std::pair<int, std::pair<int, double>>> customers_with_scores;
  const std::vector<int>& customers = solution.GetCustomersInFacility()[facility_id];

  for (int customer_id : customers) {
    const double amount =
        solution.GetCustomerFacilityFraction(customer_id, facility_id) *
        solution.GetInstance().GetCustomerDemand(customer_id);
    if (amount <= amount_tol) {
      continue;
    }
    const int blocking_score =
        CountCustomersUnblockedByRemovingCustomer(solution, customer_id, facility_id);
    const double transport_cost =
        amount * solution.GetInstance().GetAssignmentCost(customer_id, facility_id);
    customers_with_scores.emplace_back(customer_id, std::make_pair(blocking_score, transport_cost));
  }

  std::sort(customers_with_scores.begin(), customers_with_scores.end(),
            [](const std::pair<int, std::pair<int, double>>& first_customer,
               const std::pair<int, std::pair<int, double>>& second_customer) {
              if (first_customer.second.first != second_customer.second.first) {
                return first_customer.second.first > second_customer.second.first;
              }
              return first_customer.second.second > second_customer.second.second;
            });

  std::vector<int> customers_sorted;
  for (const auto& customer_with_score : customers_with_scores) {
    customers_sorted.push_back(customer_with_score.first);
  }

  return customers_sorted;
}

/**
 * @brief Sorts feasible target facilities by increasing assignment cost.
 *
 * @param solution Current solution.
 * @param customer_id Customer identifier.
 * @param source_facility Source facility identifier.
 * @param amount Amount to move.
 * @param amount_tol Numerical tolerance for flow amounts.
 * @return Target facilities sorted by assignment cost.
 */
std::vector<int> MsCflpCIncompabilitiesRemoverExplorer::GetTargetFacilitiesSortedByAssignmentCost(
    const MsCflpCiSolution& solution, int customer_id, int source_facility, double amount,
    double amount_tol) const {
  std::vector<std::pair<int, double>> facilities_with_costs;
  const int facility_count = solution.GetFacilityCount();

  for (int facility_id = 0; facility_id < facility_count; ++facility_id) {
    if (facility_id == source_facility) {
      continue;
    }
    if (solution.GetResidualCapacity(facility_id) + amount_tol < amount) {
      continue;
    }
    if (solution.IsFacilityOpen(facility_id)) {
      if (!solution.IsCustomerServedByFacility(customer_id, facility_id) &&
          !solution.CanAssignCustomerToFacility(customer_id, facility_id)) {
        continue;
      }
    } else if (solution.GetIncompatibilityCounter()[customer_id][facility_id] != 0) {
      continue;
    }
    facilities_with_costs.emplace_back(
        facility_id, solution.GetInstance().GetAssignmentCost(customer_id, facility_id));
  }

  std::sort(facilities_with_costs.begin(), facilities_with_costs.end(),
            [](const std::pair<int, double>& first_facility,
               const std::pair<int, double>& second_facility) {
              return first_facility.second < second_facility.second;
            });

  std::vector<int> facilities_sorted;
  for (const auto& facility_with_cost : facilities_with_costs) {
    facilities_sorted.push_back(facility_with_cost.first);
  }

  return facilities_sorted;
}