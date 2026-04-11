// University of La Laguna
// School of Engineering and Technology
// Bachelor's Degree in Computer Engineering
// Subject: Design and Analysis of Algorithms
// Course: 3rd
// Practice 6: Multi-Source Capacitated Facility Location Problem with Customer
// Incompatibilities
// Author: Javier Acosta Portocarrero
// Date: 04/04/2026
// File ms_cflp_ci_solution.h: declaration file.
// Contains the declaration of the MsCflpCiSolution class.

#ifndef MS_CFLP_CI_SOLUTION_H_
#define MS_CFLP_CI_SOLUTION_H_

#include <vector>

#include "ms_cflp_ci_instance.h"
#include "solution.h"

/**
 * @class MsCflpCiSolution
 *
 * @brief Class that represents a solution of the MS-CFLP-CI problem.
 *
 * The class stores both the primary structures of the solution and the
 * auxiliary structures needed to evaluate and modify it efficiently.
 */
class MsCflpCiSolution : public Solution {
 public:
  explicit MsCflpCiSolution(const MsCflpCiInstance& instance);
  ~MsCflpCiSolution() override = default;

  const MsCflpCiInstance& GetInstance() const;
  int GetCustomerCount() const;
  int GetFacilityCount() const;
  const std::vector<bool>& GetOpenFacilities() const;
  const std::vector<std::vector<double>>& GetAssignmentFractions() const;
  const std::vector<std::vector<bool>>& GetAssignments() const;
  const std::vector<std::vector<int>>& GetCustomersInFacility() const;
  const std::vector<std::vector<int>>& GetFacilitiesOf() const;
  const std::vector<std::vector<int>>& GetIncompatibilityCounter() const;
  double GetCustomerFacilityFraction(int customer_id, int facility_id) const;
  double GetResidualCapacity(int facility_id) const;
  double GetFixedCost() const;
  double GetTransportCost() const;
  // Returns the objective value of the solution.
  double GetTotalCost() const;

  bool AddFlow(int customer_id, int facility_id, double amount);
  bool RemoveFlow(int customer_id, int facility_id, double amount);
  bool ShiftFlow(int customer_id, int source_facility, int target_facility, double amount);
  bool OpenFacility(int facility_id);
  bool CloseFacility(int facility_id);

  bool IsFacilityOpen(int facility_id) const;
  bool IsCustomerServedByFacility(int customer_id, int facility_id) const;
  bool CanAssignCustomerToFacility(int customer_id, int facility_id) const;
  bool CanAddFlow(int customer_id, int facility_id, double amount) const;
  bool CanRemoveFlow(int customer_id, int facility_id, double amount) const;
  bool CanShiftFlow(int customer_id, int source_facility, int target_facility, double amount) const;
  bool CanSwapCustomersBetweenFacilities(int customer_a, int facility_a, int customer_b, int facility_b) const;

  double EvaluateShiftDelta(int customer_id, int source_facility, int target_facility, double amount) const;
  double EvaluateSwapDelta(int customer_a, int facility_a, int customer_b, int facility_b) const;
  bool IsCustomerFullySatisfied(int customer_id, double tolerance = 1e-8) const;
  bool IsFeasible(double tolerance = 1e-8) const;
  // Used to check correct behavior of delta evaluations.
  double ComputeObjectiveFromScratch() const;
  bool CheckObjectiveConsistency(double tolerance = 1e-8) const;

  bool CanSwapFacilities(int source_facility, int target_facility) const;
  double EvaluateFacilitiesSwapDelta(int source_facility, int target_facility) const;
  bool SwapFacilities(int source_facility, int target_facility);

  int CountOpenFacilities() const;
  int CountIncompatibilityViolations() const;

 private:
  bool IsValidCustomerId(int customer_id) const;
  bool IsValidFacilityId(int facility_id) const;
  bool HasCustomerInFacility(int customer_id, int facility_id) const;

  void AddCustomerToFacilityLists(int customer_id, int facility_id);
  void RemoveCustomerFromFacilityLists(int customer_id, int facility_id);
  void IncreaseIncompatibilityCounters(int customer_id, int facility_id);
  void DecreaseIncompatibilityCounters(int customer_id, int facility_id);

  double GetAssignedAmount(int customer_id, int facility_id) const;
  double GetCustomerAssignedFractionSum(int customer_id) const;
  double GetFacilityUsedCapacity(int facility_id) const;

  const MsCflpCiInstance& instance_;
  // Primary structures.
  std::vector<bool> factory_open_;
  std::vector<std::vector<double>> assignment_fraction_;  // x
  std::vector<std::vector<bool>> assignment_;  // w
  // Auxiliary structures.
  std::vector<double> residual_capacity_;
  std::vector<std::vector<int>> clients_of_;
  std::vector<std::vector<int>> facilities_of_;
  std::vector<std::vector<int>> incompatibility_count_;

  double fixed_cost_ = 0.0;
  double transport_cost_ = 0.0;
  double total_cost_ = 0.0;
};

#endif