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
      const std::vector<int> target_facilities = GetTargetFacilitiesSortedByAssignmentCost(*solution, customer_id,
                                                                                   source_facility, amount, amount_tol);
      for (int target_facility : target_facilities) {
        if (!CanMoveCustomerToFacility(*solution, customer_id, source_facility, target_facility, amount, amount_tol)) {
          continue;
        }
        const double delta = EvaluateMoveCustomerDelta(*solution, customer_id, source_facility, target_facility, amount);
        if (delta < -improvement_tol) {
          MsCflpCiSolution* new_solution = new MsCflpCiSolution(*solution);
          if (!ApplyMoveCustomer(*new_solution, customer_id, source_facility, target_facility, amount)) {
            delete new_solution;
            continue;
            std::cerr << "ERROR: Incompabilities remove move cant be applied\n";
          }
          //std::cout << "Imcompabilties remove move. " << delta << std::endl;
          return new_solution;
        }
      }
    }
  }

  return nullptr;
}

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

int MsCflpCIncompabilitiesRemoverExplorer::CountCustomersUnblockedByRemovingCustomer(
    const MsCflpCiSolution& solution, int customer_id, int facility_id) const {
  int unblocked_customers = 0;
  const std::vector<int>& incompatible_customers =
      solution.GetInstance().GetIncompatibleCustomers(customer_id);

  for (int incompatible_customer : incompatible_customers) {
    if (solution.GetIncompatibilityCounter()[incompatible_customer][facility_id] == 1) {
      ++unblocked_customers;
    }
  }

  return unblocked_customers;
}

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