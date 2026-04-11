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
          if (!solution->CanSwapCustomersBetweenFacilities(customer_a, facility_a, customer_b, facility_b)) {
            continue;
          }
          const double swap_delta =
              solution->EvaluateSwapDelta(customer_a, facility_a, customer_b, facility_b);
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
