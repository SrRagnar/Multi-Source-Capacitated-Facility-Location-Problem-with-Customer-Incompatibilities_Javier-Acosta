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

#ifndef MS_CFLP_CI_FACILITIES_SWAP_EXPLORER_H_
#define MS_CFLP_CI_FACILITIES_SWAP_EXPLORER_H_

#include "ms_cflp_ci_neighboorhod_explorer.h"

class MsCflpCiFacilitiesSwapExplorer : public MsCflpCiNeighboorhodExplorer {
 public:
  MsCflpCiFacilitiesSwapExplorer() = default;
  ~MsCflpCiFacilitiesSwapExplorer() override = default;

  MsCflpCiSolution* Explore(const MsCflpCiSolution* solution, double amount_tol, double improvement_tol) const override;
 private:
  bool CanSwapFacilities(const MsCflpCiSolution& solution, int source_facility, int target_facility,
                         double amount_tol, double improvement_tol) const;
  double EvaluateFacilitiesSwapDelta(const MsCflpCiSolution& solution, int source_facility, int target_facility,
                                     double amount_tol, double improvement_tol) const;
  bool SwapFacilities(MsCflpCiSolution& solution, int source_facility, int target_facility,
                      double amount_tol, double improvement_tol) const;
};

#endif