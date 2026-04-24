// University of La Laguna
// School of Engineering and Technology
// Bachelor's Degree in Computer Engineering
// Subject: Design and Analysis of Algorithms
// Course: 3rd
// Practice 6: Multi-Source Capacitated Facility Location Problem with Customer
// Incompatibilities
// Author: Javier Acosta Portocarrero
// Date: 24/04/2026
// File ms_cflp_ci_shift_perturbator.h: declaration file.
// Contains the declaration of the MsCflpCiShiftPerturbator class.

#include <random>

#include "ms_cflp_ci_shift_perturbator.h"

MsCflpCiSolution* MsCflpCiShiftPerturbator::Perturbate(const MsCflpCiSolution* solution, double amount_tol, double improvement_tol) const {
  const int customers_amount = solution->GetCustomerCount();
  const int facilities_amount = solution->GetFacilityCount();
  const unsigned max_iter = customers_amount * facilities_amount;
  if (customers_amount <= 0 || facilities_amount <= 1) {
    return new MsCflpCiSolution(*solution);
  }

  for (unsigned i = 0; i < max_iter; ++i) {
    const int customer_id = std::rand() % customers_amount;
    const std::vector<std::vector<int>>& facilities_of = solution->GetFacilitiesOf();
    if (facilities_of[customer_id].empty()) {
      continue;
    }
    const int source_index = std::rand() % facilities_of[customer_id].size();
    const int source_facility = facilities_of[customer_id][source_index];
    const int target_facility = std::rand() % facilities_amount;
    if (source_facility == target_facility) {
      continue;
    }
    const double source_amount = solution->GetAssignedAmount(customer_id, source_facility);
    const double residual_capacity = solution->GetResidualCapacity(target_facility);
    const double max_amount = std::min(source_amount, residual_capacity);
    if (max_amount <= amount_tol) {
      continue;
    }
    const double random_fraction = static_cast<double>(std::rand()) / static_cast<double>(RAND_MAX);
    const double amount = amount_tol + random_fraction * (max_amount - amount_tol);
    if (!CanShiftFlow(*solution, customer_id, source_facility, target_facility,
                      amount, amount_tol, improvement_tol)) {
      continue;
    }
    MsCflpCiSolution* perturbed_solution = new MsCflpCiSolution(*solution);
    ShiftFlow(*perturbed_solution, customer_id, source_facility, target_facility,
              amount, amount_tol, improvement_tol);

    return perturbed_solution;
  }

  return new MsCflpCiSolution(*solution);
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
bool MsCflpCiShiftPerturbator::CanShiftFlow(const MsCflpCiSolution& solution, int customer_id, 
                                            int source_facility, int target_facility, double amount,
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
bool MsCflpCiShiftPerturbator::ShiftFlow(MsCflpCiSolution& solution, int customer_id, 
                                        int source_facility, int target_facility, double amount,
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