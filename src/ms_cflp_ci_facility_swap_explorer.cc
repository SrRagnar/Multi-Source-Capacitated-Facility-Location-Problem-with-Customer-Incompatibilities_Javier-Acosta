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
      if (!solution->CanSwapFacilities(open_facility, closed_facility)) {
        continue;
      }
      const double swap_delta = solution->EvaluateFacilitiesSwapDelta(open_facility, closed_facility);
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
  if (!best_solution->SwapFacilities(best_source_facility, best_target_facility)) {
    delete best_solution;
    std::cerr << "Error: failed to apply the best facilities swap move found." << std::endl;
    return nullptr;
  }
  return best_solution;
}