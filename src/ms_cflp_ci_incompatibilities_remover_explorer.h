// University of La Laguna
// School of Engineering and Technology
// Bachelor's Degree in Computer Engineering
// Subject: Design and Analysis of Algorithms
// Course: 3rd
// Practice 6: Multi-Source Capacitated Facility Location Problem with Customer
// Incompatibilities
// Author: Javier Acosta Portocarrero
// Date: 10/04/2026
// File ms_cflp_ci_incompabilities_remover_explorer.h: declaration file.
// Contains the declaration of the MsCflpCiIncompatibilitiesRemoverExplorer class.

#ifndef MS_CFLP_CI_INCOMPABILITIES_REMOVER_EXPLORER_H_
#define MS_CFLP_CI_INCOMPABILITIES_REMOVER_EXPLORER_H_

#include "ms_cflp_ci_neighboorhod_explorer.h"

class MsCflpCIncompabilitiesRemoverExplorer : public MsCflpCiNeighboorhodExplorer {
 public:
  MsCflpCIncompabilitiesRemoverExplorer() = default;
  ~MsCflpCIncompabilitiesRemoverExplorer() override = default;

  MsCflpCiSolution* Explore(const MsCflpCiSolution* solution, double amount_tol, double improvement_tol) const override;

 private:
  double GetFacilityTransportCost(const MsCflpCiSolution& solution, int facility_id) const;
  int CountCustomersUnblockedByRemovingCustomer(const MsCflpCiSolution& solution, int customer_id,
                                                int facility_id) const;
  bool CanMoveCustomerToFacility(const MsCflpCiSolution& solution, int customer_id, int source_facility,
                                 int target_facility, double amount, double amount_tol) const;
  double EvaluateMoveCustomerDelta(const MsCflpCiSolution& solution, int customer_id, int source_facility,
                                   int target_facility, double amount) const;
  bool ApplyMoveCustomer(MsCflpCiSolution& solution, int customer_id, int source_facility,
                         int target_facility, double amount) const;
  std::vector<int> GetOpenFacilitiesSortedByTransportCost(const MsCflpCiSolution& solution) const;
  std::vector<int> GetCustomersSortedByBlockingScore(const MsCflpCiSolution& solution, int facility_id,
                                                     double amount_tol) const;
  std::vector<int> GetTargetFacilitiesSortedByAssignmentCost(const MsCflpCiSolution& solution, int customer_id,
                                                             int source_facility, double amount,
                                                             double amount_tol) const;
};

#endif