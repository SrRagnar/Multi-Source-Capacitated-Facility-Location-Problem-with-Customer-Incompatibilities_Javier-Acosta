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

#ifndef MS_CFLP_CI_SWAP_EXPLORER_H_
#define MS_CFLP_CI_SWAP_EXPLORER_H_

#include "ms_cflp_ci_neighboorhod_explorer.h"

class MsCflpCiSwapExplorer : public MsCflpCiNeighboorhodExplorer {
 public:
  MsCflpCiSwapExplorer() = default;
  ~MsCflpCiSwapExplorer() override = default;

  MsCflpCiSolution* Explore(const MsCflpCiSolution* solution, double amount_tol, double improvement_tol) const override;
 private:
  double EvaluateSwapDelta(const MsCflpCiSolution& solution, 
                           int customer_a, int facility_a, int customer_b, int facility_b,
                           double amount_tol, double improvement_tol) const;

  bool CanSwapCustomersBetweenFacilities(const MsCflpCiSolution& solution, 
                                         int customer_a, int facility_a, int customer_b, int facility_b,
                                         double amount_tol, double improvement_tol) const;
};

#endif