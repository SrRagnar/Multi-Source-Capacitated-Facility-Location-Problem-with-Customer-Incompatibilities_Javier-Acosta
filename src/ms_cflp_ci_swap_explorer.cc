// University of La Laguna
// School of Engineering and Technology
// Bachelor's Degree in Computer Engineering
// Subject: Design and Analysis of Algorithms
// Course: 3rd
// Practice 6: Multi-Source Capacitated Facility Location Problem with Customer
// Incompatibilities
// Author: Javier Acosta Portocarrero
// Date: 10/04/2026
// File ms_cflp_ci_swap_explorer.h: declaration file.
// Contains the declaration of the MsCflpCiSwapExplorer class.

#include <iostream>
#include <cmath>

#include "ms_cflp_ci_swap_explorer.h"

MsCflpCiSolution* MsCflpCiSwapExplorer::Explore(const MsCflpCiSolution* solution, 
                                                 double amount_tol, double improvement_tol) const {
  const int customer_count = solution->GetCustomerCount();
  const double no_improving_delta = 0.0;
  int best_customer_a = -1;
  int best_customer_b = -1;
  int best_facility_a = -1;
  int best_facility_b = -1;
  double best_delta = no_improving_delta;

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
          if (!CanSwapCustomersBetweenFacilities(*solution, customer_a, facility_a, customer_b, facility_b,
                                                  amount_tol, improvement_tol)) {
            continue;
          }
          const double swap_delta = EvaluateSwapDelta(*solution, customer_a, facility_a, customer_b, facility_b,
                                                      amount_tol, improvement_tol);
          // Only consider improving moves, without calculating the whole new objective value.
          if (swap_delta < best_delta - improvement_tol) {
            best_customer_a = customer_a;
            best_customer_b = customer_b;
            best_facility_a = facility_a;
            best_facility_b = facility_b;
            best_delta = swap_delta;
          }
        }
      }
    }
  }
  if (best_customer_a == -1) {
    return nullptr;
  }
  MsCflpCiSolution* best_solution = new MsCflpCiSolution(*solution);
  const double amount_a =
      solution->GetCustomerFacilityFraction(best_customer_a, best_facility_a) *
      solution->GetInstance().GetCustomerDemand(best_customer_a);
  const double amount_b =
      solution->GetCustomerFacilityFraction(best_customer_b, best_facility_b) *
      solution->GetInstance().GetCustomerDemand(best_customer_b);
  if (!best_solution->RemoveFlow(best_customer_a, best_facility_a, amount_a) ||
      !best_solution->RemoveFlow(best_customer_b, best_facility_b, amount_b) ||
      !best_solution->AddFlow(best_customer_a, best_facility_b, amount_a) ||
      !best_solution->AddFlow(best_customer_b, best_facility_a, amount_b)) {
    // This should never happen.
    delete best_solution;
    std::cerr << "ERROR: Failed to apply the best swap move found." << std::endl;
    return nullptr;
  }

  //std::cout << "Swap improvement " << best_delta << std::endl;
  return best_solution;
}


/**
 * @brief Checks whether two customers can be swapped between two facilities.
 *
 * @param customer_a First customer identifier.
 * @param facility_a Facility serving the first customer.
 * @param customer_b Second customer identifier.
 * @param facility_b Facility serving the second customer.
 * @return True if the swap is feasible, false otherwise.
 */
bool MsCflpCiSwapExplorer::CanSwapCustomersBetweenFacilities(const MsCflpCiSolution& solution, 
                                         int customer_a, int facility_a, int customer_b, int facility_b,
                                         double amount_tol, double improvement_tol) const {
  if (!solution.IsValidCustomerId(customer_a) || !solution.IsValidCustomerId(customer_b) ||
      !solution.IsValidFacilityId(facility_a) || !solution.IsValidFacilityId(facility_b)) {
    return false;
  }
  if (customer_a == customer_b || facility_a == facility_b) {
    return false;
  }
  if (!solution.IsCustomerFullySatisfied(customer_a) || !solution.IsCustomerFullySatisfied(customer_b)) {
    return false;
  }
  const std::vector<std::vector<int>>& facilities_of = solution.GetFacilitiesOf();
  if (facilities_of[customer_a].size() != 1 || facilities_of[customer_b].size() != 1) {
    return false;
  }
  const std::vector<std::vector<bool>>& assignment = solution.GetAssignments();
  if (!assignment[customer_a][facility_a] || !assignment[customer_b][facility_b]) {
    return false;
  }

  const double amount_a = solution.GetAssignedAmount(customer_a, facility_a);
  const double amount_b = solution.GetAssignedAmount(customer_b, facility_b);
  const MsCflpCiInstance& instance = solution.GetInstance();
  const double demand_a = instance.GetCustomerDemand(customer_a);
  const double demand_b = instance.GetCustomerDemand(customer_b);

  if (std::fabs(amount_a - demand_a) > amount_tol ||
      std::fabs(amount_b - demand_b) > amount_tol) {
    return false;
  }
  if (amount_b - (solution.GetResidualCapacity(facility_a) + amount_a) > amount_tol) {
    return false;
  }
  if (amount_a - (solution.GetResidualCapacity(facility_b) + amount_b) > amount_tol) {
    return false;
  }
  const std::vector<std::vector<int>>& incompatibility_count = solution.GetIncompatibilityCounter();
  if (incompatibility_count[customer_a][facility_b] != 0) {
    return false;
  }
  if (incompatibility_count[customer_b][facility_a] != 0) {
    return false;
  }

  return true;    
}

/**
 * @brief Evaluates the cost delta of swapping two customers between two facilities.
 *
 * @param customer_a First customer identifier.
 * @param facility_a Facility serving the first customer.
 * @param customer_b Second customer identifier.
 * @param facility_b Facility serving the second customer.
 * @return Cost delta of performing the swap move.
 */
double MsCflpCiSwapExplorer::EvaluateSwapDelta(const MsCflpCiSolution& solution, 
                           int customer_a, int facility_a, int customer_b, int facility_b,
                           double amount_tol, double improvement_tol) const {
  const MsCflpCiInstance& instance_ = solution.GetInstance();
  const double amount_a = instance_.GetCustomerDemand(customer_a);
  const double amount_b = instance_.GetCustomerDemand(customer_b);

  const double cost_a_old = instance_.GetAssignmentCost(customer_a, facility_a);
  const double cost_a_new = instance_.GetAssignmentCost(customer_a, facility_b);
  const double cost_b_old = instance_.GetAssignmentCost(customer_b, facility_b);
  const double cost_b_new = instance_.GetAssignmentCost(customer_b, facility_a);

  double delta = 0.0;
  delta += (cost_a_new - cost_a_old) * amount_a;
  delta += (cost_b_new - cost_b_old) * amount_b;

  return delta;
}