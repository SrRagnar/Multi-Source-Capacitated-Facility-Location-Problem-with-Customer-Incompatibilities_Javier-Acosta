// University of La Laguna
// School of Engineering and Technology
// Bachelor's Degree in Computer Engineering
// Subject: Design and Analysis of Algorithms
// Course: 3rd
// Practice 6: Multi-Source Capacitated Facility Location Problem with Customer
// Incompatibilities
// Author: Javier Acosta Portocarrero
// Date: 04/04/2026
// File ms_cflp_ci_solution.cc: implementation file.
// Contains the implementation of the MsCflpCiSolution class.

#include "ms_cflp_ci_solution.h"

#include <iostream>

#include <algorithm>
#include <cmath>
#include <stdexcept>

namespace {
  constexpr double kAmountTolerance = 1e-8;
  constexpr double kFractionTolerance = 1e-8;
  constexpr double kCostTolerance = 1e-8;

  double ClampNearZero(double value, double tolerance = kAmountTolerance) {
    if (std::fabs(value) <= tolerance) {
      return 0.0;
    }
    return value;
  }

  double ClampFraction(double value, double tolerance = kFractionTolerance) {
    if (std::fabs(value) <= tolerance) {
      return 0.0;
    }
    if (std::fabs(value - 1.0) <= tolerance) {
      return 1.0;
    }
    return value;
  }
}  

/**
 * @brief Builds an empty solution for a given instance.
 *
 * @param instance Problem instance associated with the solution.
 */
MsCflpCiSolution::MsCflpCiSolution(const MsCflpCiInstance& instance) : instance_(instance) {
  int customer_count = instance_.GetCustomerCount();
  int facility_count = instance_.GetFacilityCount();
  factory_open_.assign(facility_count, false);
  assignment_fraction_.assign(customer_count, std::vector<double>(facility_count, 0.0));
  assignment_.assign(customer_count, std::vector<bool>(facility_count, false));

  residual_capacity_.resize(facility_count);
  for (int j = 0; j < facility_count; ++j) {
    residual_capacity_[j] = instance_.GetFacilityCapacity(j);
  }
  clients_of_.assign(facility_count, std::vector<int>());
  facilities_of_.assign(customer_count, std::vector<int>());
  incompatibility_count_.assign(customer_count, std::vector<int>(facility_count, 0));
}

/**
 * @brief Returns the associated instance.
 *
 * @return Constant reference to the instance.
 */
const MsCflpCiInstance& MsCflpCiSolution::GetInstance() const {
  return instance_;
}

/**
 * @brief Returns the number of customers.
 *
 * @return Number of customers.
 */
int MsCflpCiSolution::GetCustomerCount() const {
  return instance_.GetCustomerCount();
}

/**
 * @brief Returns the number of facilities.
 *
 * @return Number of facilities.
 */
int MsCflpCiSolution::GetFacilityCount() const {
  return instance_.GetFacilityCount();
}

/**
 * @brief Returns the open facilities vector.
 *
 * @return Constant reference to the open facilities vector.
 */
const std::vector<bool>& MsCflpCiSolution::GetOpenFacilities() const {
  return factory_open_;
}

/**
 * @brief Returns the assignment fraction matrix.
 *
 * @return Constant reference to the fraction matrix.
 */
const std::vector<std::vector<double>>& MsCflpCiSolution::GetAssignmentFractions() const {
  return assignment_fraction_;
}

/**
 * @brief Returns the service matrix.
 *
 * @return Constant reference to the binary assignment matrix.
 */
const std::vector<std::vector<bool>>& MsCflpCiSolution::GetAssignments() const {
  return assignment_;
}

/**
 * @brief Returns the customers served by each facility.
 *
 * @return Constant reference to the structure.
 */
const std::vector<std::vector<int>>& MsCflpCiSolution::GetCustomersInFacility() const {
  return clients_of_;
}

/**
 * @brief Returns the facilities serving each customer.
 *
 * @return Constant reference to the structure.
 */
const std::vector<std::vector<int>>& MsCflpCiSolution::GetFacilitiesOf() const {
  return facilities_of_;
}

/**
 * @brief Returns the incompatibility counter matrix.
 *
 * @return Constant reference to the structure.
 */
const std::vector<std::vector<int>>& MsCflpCiSolution::GetIncompatibilityCounter() const {
  return incompatibility_count_;
}

/**
 * @brief Returns the assigned fraction from a facility to a customer.
 *
 * @param customer_id Customer identifier.
 * @param facility_id Facility identifier.
 * @return Assigned fraction.
 */
double MsCflpCiSolution::GetCustomerFacilityFraction(
    int customer_id, int facility_id) const {
  if (!IsValidCustomerId(customer_id)) {
    throw std::out_of_range("Invalid customer id.");
  }
  if (!IsValidFacilityId(facility_id)) {
    throw std::out_of_range("Invalid facility id.");
  }

  return assignment_fraction_[customer_id][facility_id];
}

/**
 * @brief Returns the residual capacity of a facility.
 *
 * @param facility_id Facility identifier.
 * @return Residual capacity.
 */
double MsCflpCiSolution::GetResidualCapacity(int facility_id) const {
  if (!IsValidFacilityId(facility_id)) {
    throw std::out_of_range("Invalid facility id.");
  }

  return residual_capacity_[facility_id];
}

/**
 * @brief Returns the fixed opening cost.
 *
 * @return Fixed cost.
 */
double MsCflpCiSolution::GetFixedCost() const {
  return fixed_cost_;
}

/**
 * @brief Returns the transport cost.
 *
 * @return Transport cost.
 */
double MsCflpCiSolution::GetTransportCost() const {
  return transport_cost_;
}

/**
 * @brief Returns the total cost, which is the objective value of the solution.
 *
 * @return The total cost.
 */
double MsCflpCiSolution::GetTotalCost() const {
  return total_cost_;
}

/**
 * @brief Opens a facility.
 *
 * @param facility_id Facility identifier.
 * @return True if the operation is performed, false otherwise.
 */
bool MsCflpCiSolution::OpenFacility(int facility_id) {
  if (!IsValidFacilityId(facility_id)) {
    return false;
  }
  if (factory_open_[facility_id]) {
    return true;
  }
  factory_open_[facility_id] = true;
  fixed_cost_ += instance_.GetFacilityOpeningCost(facility_id);
  total_cost_ = fixed_cost_ + transport_cost_;

  return true;
}

/**
 * @brief Closes a facility if it has no assigned customers.
 *
 * @param facility_id Facility identifier.
 * @return True if the operation is performed, false otherwise.
 */
bool MsCflpCiSolution::CloseFacility(int facility_id) {
  if (!IsValidFacilityId(facility_id)) {
    return false;
  }
  if (!factory_open_[facility_id]) {
    return true;
  }
  if (!clients_of_[facility_id].empty()) {
    return false;
  }
  factory_open_[facility_id] = false;
  residual_capacity_[facility_id] = instance_.GetFacilityCapacity(facility_id);
  fixed_cost_ -= instance_.GetFacilityOpeningCost(facility_id);
  total_cost_ = fixed_cost_ + transport_cost_;

  return true;
}

/**
 * @brief Checks whether a facility is open.
 *
 * @param facility_id Facility identifier.
 * @return True if the facility is open, false otherwise.
 */
bool MsCflpCiSolution::IsFacilityOpen(int facility_id) const {
  if (!IsValidFacilityId(facility_id)) {
    throw std::out_of_range("Invalid facility id.");
  }

  return factory_open_[facility_id];
}

/**
 * @brief Checks whether a customer is served by a facility.
 *
 * @param customer_id Customer identifier.
 * @param facility_id Facility identifier.
 * @return True if the customer is served by the facility, false otherwise.
 */
bool MsCflpCiSolution::IsCustomerServedByFacility(int customer_id, int facility_id) const {
  if (!IsValidCustomerId(customer_id)) {
    throw std::out_of_range("Invalid customer id.");
  }
  if (!IsValidFacilityId(facility_id)) {
    throw std::out_of_range("Invalid facility id.");
  }

  return assignment_[customer_id][facility_id];
}

/**
 * @brief Checks whether a customer can be assigned to a facility.
 *
 * This method checks the incompatibility condition and whether the facility
 * is open.
 *
 * @param customer_id Customer identifier.
 * @param facility_id Facility identifier.
 * @return True if the customer can be assigned, false otherwise.
 */
bool MsCflpCiSolution::CanAssignCustomerToFacility(int customer_id, int facility_id) const {
  if (!IsValidCustomerId(customer_id) || !IsValidFacilityId(facility_id) || 
      !factory_open_[facility_id] || assignment_[customer_id][facility_id]) {
    return false;
  }

  return incompatibility_count_[customer_id][facility_id] == 0;
}

/**
 * @brief Checks whether a flow amount can be added.
 *
 * The amount is expressed in demand units.
 *
 * @param customer_id Customer identifier.
 * @param facility_id Facility identifier.
 * @param amount Amount of demand units to add.
 * @return True if the flow can be added, false otherwise.
 */
bool MsCflpCiSolution::CanAddFlow(int customer_id, int facility_id, double amount) const {
  if (!IsValidCustomerId(customer_id) || !IsValidFacilityId(facility_id)) {
    return false;
  }
  if (!factory_open_[facility_id]) {
    return false;
  }
  if (amount <= kAmountTolerance) {
    return false;
  }

  const double residual_capacity = residual_capacity_[facility_id];
  if (amount - residual_capacity > kAmountTolerance) {
    return false;
  }

  const double demand = instance_.GetCustomerDemand(customer_id);
  const double assigned_amount = GetAssignedAmount(customer_id, facility_id);
  if (assigned_amount + amount - demand > kAmountTolerance) {
    return false;
  }

  const double assigned_fraction_sum = GetCustomerAssignedFractionSum(customer_id);
  if (assigned_fraction_sum + amount / demand - 1.0 > kFractionTolerance) {
    return false;
  }

  // Only check incompatibilities if the customer is not already assigned to the facility.
  if (!assignment_[customer_id][facility_id] &&
      incompatibility_count_[customer_id][facility_id] != 0) {
    return false;
  }

  return true;
}

/**
 * @brief Checks whether a flow amount can be removed.
 *
 * The amount is expressed in demand units.
 *
 * @param customer_id Customer identifier.
 * @param facility_id Facility identifier.
 * @param amount Amount of demand units to remove.
 * @return True if the flow can be removed, false otherwise.
 */
bool MsCflpCiSolution::CanRemoveFlow(int customer_id, int facility_id, double amount) const {
  if (!IsValidCustomerId(customer_id) || !IsValidFacilityId(facility_id)) {
    return false;
  }
  if (!assignment_[customer_id][facility_id]) {
    return false;
  }
  if (amount <= kAmountTolerance) {
    return false;
  }

  const double assigned_amount = GetAssignedAmount(customer_id, facility_id);
  if (amount - assigned_amount > kAmountTolerance) {
    return false;
  }

  return true;
}

/**
 * @brief Checks whether a shift move is feasible.
 *
 * This version avoids relying on CanAddFlow/CanRemoveFlow, which are too restrictive
 * for shift operations. Instead, it directly checks the true conditions of a shift:
 * - enough flow in source,
 * - enough capacity in target,
 * - incompatibilities,
 * - facilities open.
 */
bool MsCflpCiSolution::CanShiftFlow(
    int customer_id, int source_facility, int target_facility, double amount) const {
  if (!IsValidCustomerId(customer_id) ||
      !IsValidFacilityId(source_facility) ||
      !IsValidFacilityId(target_facility)) {
    return false;
  }
  if (source_facility == target_facility) {
    return false;
  }
  if (amount <= kAmountTolerance) {
    return false;
  }
  const double source_amount = GetAssignedAmount(customer_id, source_facility);
  if (!assignment_[customer_id][source_facility] ||
      amount - source_amount > kAmountTolerance) {
    return false;
  }
  if (!assignment_[customer_id][target_facility] &&
      incompatibility_count_[customer_id][target_facility] != 0) {
    return false;
  }
  if (amount - residual_capacity_[target_facility] > kAmountTolerance) {
    return false;
  }

  return true;
}

/**
 * @brief Adds a flow amount from a facility to a customer.
 *
 * The amount is expressed in demand units.
 *
 * @param customer_id Customer identifier.
 * @param facility_id Facility identifier.
 * @param amount Amount of demand units to add.
 * @return True if the operation is performed, false otherwise.
 */
bool MsCflpCiSolution::AddFlow(int customer_id, int facility_id, double amount) {
  if (!CanAddFlow(customer_id, facility_id, amount)) {
    return false;
  }

  const double demand = instance_.GetCustomerDemand(customer_id);
  const double fraction = amount / demand;
  const bool was_assigned = assignment_[customer_id][facility_id];

  assignment_fraction_[customer_id][facility_id] += fraction;
  assignment_fraction_[customer_id][facility_id] =
      ClampFraction(assignment_fraction_[customer_id][facility_id]);

  residual_capacity_[facility_id] -= amount;
  residual_capacity_[facility_id] = ClampNearZero(residual_capacity_[facility_id]);
  if (residual_capacity_[facility_id] < 0.0 &&
      std::fabs(residual_capacity_[facility_id]) <= kAmountTolerance) {
    residual_capacity_[facility_id] = 0.0;
  }

  transport_cost_ += instance_.GetAssignmentCost(customer_id, facility_id) * amount;

  if (!was_assigned) {
    assignment_[customer_id][facility_id] = true;
    AddCustomerToFacilityLists(customer_id, facility_id);
    IncreaseIncompatibilityCounters(customer_id, facility_id);
  }

  total_cost_ = fixed_cost_ + transport_cost_;
  return true;
}

/**
 * @brief Removes a flow amount from a facility to a customer.
 *
 * The amount is expressed in demand units.
 *
 * @param customer_id Customer identifier.
 * @param facility_id Facility identifier.
 * @param amount Amount of demand units to remove.
 * @return True if the operation is performed, false otherwise.
 */
bool MsCflpCiSolution::RemoveFlow(int customer_id, int facility_id, double amount) {
  if (!CanRemoveFlow(customer_id, facility_id, amount)) {
    return false;
  }

  const double demand = instance_.GetCustomerDemand(customer_id);
  const double fraction = amount / demand;

  assignment_fraction_[customer_id][facility_id] -= fraction;
  assignment_fraction_[customer_id][facility_id] = ClampFraction(assignment_fraction_[customer_id][facility_id]);

  if (assignment_fraction_[customer_id][facility_id] < 0.0 &&
      std::fabs(assignment_fraction_[customer_id][facility_id]) <= kFractionTolerance) {
    assignment_fraction_[customer_id][facility_id] = 0.0;
  }

  residual_capacity_[facility_id] += amount;
  residual_capacity_[facility_id] = ClampNearZero(residual_capacity_[facility_id]);

  transport_cost_ -= instance_.GetAssignmentCost(customer_id, facility_id) * amount;

  if (assignment_fraction_[customer_id][facility_id] <= kFractionTolerance &&
      assignment_[customer_id][facility_id]) {
    assignment_fraction_[customer_id][facility_id] = 0.0;
    assignment_[customer_id][facility_id] = false;
    DecreaseIncompatibilityCounters(customer_id, facility_id);
    RemoveCustomerFromFacilityLists(customer_id, facility_id);
  }

  total_cost_ = fixed_cost_ + transport_cost_;
  return true;
}

/**
 * @brief Applies a shift move between two facilities.
 *
 * @param customer_id Customer identifier.
 * @param source_facility Source facility.
 * @param target_facility Target facility.
 * @param amount Amount of demand units to shift.
 * @return True if the move is applied, false otherwise.
 */
bool MsCflpCiSolution::ShiftFlow(
    int customer_id, int source_facility, int target_facility, double amount) {
  if (!CanShiftFlow(customer_id, source_facility, target_facility, amount)) {
    return false;
  }

  const bool target_was_closed = !factory_open_[target_facility];
  if (target_was_closed && !OpenFacility(target_facility)) {
    return false;
  }
  if (!RemoveFlow(customer_id, source_facility, amount)) {
    if (target_was_closed) {
      CloseFacility(target_facility);
    }
    return false;
  }
  if (!AddFlow(customer_id, target_facility, amount)) {
    AddFlow(customer_id, source_facility, amount);
    if (target_was_closed) {
      CloseFacility(target_facility);
    }
    return false;
  }
  if (clients_of_[source_facility].empty()) {
    if (!CloseFacility(source_facility)) {
      return false;
    }
  }

  return true;
}

/**
 * @brief Evaluates the objective delta of a shift move.
 *
 * This method only evaluates the variation in transport cost.
 *
 * @param customer_id Customer identifier.
 * @param source_facility Source facility.
 * @param target_facility Target facility.
 * @param amount Amount of demand units to shift.
 * @return Delta value of the move.
 */
double MsCflpCiSolution::EvaluateShiftDelta(int customer_id, int source_facility, int target_facility, double amount) const {
 if (!CanShiftFlow(customer_id, source_facility, target_facility, amount)) {
    throw std::invalid_argument("Infeasible shift move.");
  }

  double delta = amount * (instance_.GetAssignmentCost(customer_id, target_facility) -
                 instance_.GetAssignmentCost(customer_id, source_facility));

  if (!factory_open_[target_facility]) {
    delta += instance_.GetFacilityOpeningCost(target_facility);
  }
  const double source_amount = GetAssignedAmount(customer_id, source_facility);
  if (std::fabs(source_amount - amount) <= kAmountTolerance && clients_of_[source_facility].size() == 1) {
    delta -= instance_.GetFacilityOpeningCost(source_facility);
  }

  return delta;
}

/**
 * @brief Checks whether a customer has all its demand satisfied.
 *
 * @param customer_id Customer identifier.
 * @param tolerance Numerical tolerance.
 * @return True if the customer is fully satisfied, false otherwise.
 */
bool MsCflpCiSolution::IsCustomerFullySatisfied(int customer_id, double tolerance) const {
  if (!IsValidCustomerId(customer_id)) {
    return false;
  }

  return std::fabs(GetCustomerAssignedFractionSum(customer_id) - 1.0) <= tolerance;
}

/**
 * @brief Checks whether the whole solution is feasible.
 *
 * The method verifies:
 * - complete demand satisfaction,
 * - capacity respect,
 * - incompatibility respect,
 * - consistency between fractions and binary assignments,
 * - consistency of residual capacities.
 *
 * @param tolerance Numerical tolerance.
 * @return True if the solution is feasible, false otherwise.
 */
bool MsCflpCiSolution::IsFeasible(double tolerance) const {
  int customer_count = GetCustomerCount();
  int facility_count = GetFacilityCount();

  for (int i = 0; i < customer_count; ++i) {
    if (!IsCustomerFullySatisfied(i, tolerance * 10)) {
      std::cerr << "Customer " << i << " is not fully satisfied by " << GetCustomerAssignedFractionSum(i) << "." << std::endl;  
      return false;
    }
  }
  for (int j = 0; j < facility_count; ++j) {
    double used_capacity = GetFacilityUsedCapacity(j);
    double expected_residual = instance_.GetFacilityCapacity(j) - used_capacity;
    if (used_capacity - instance_.GetFacilityCapacity(j) > tolerance) {
      std::cerr << "Facility " << j << " exceeds capacity." << std::endl;
      return false;
    }
    if (std::fabs(residual_capacity_[j] - expected_residual) > tolerance * 10) {
      std::cerr << "Facility " << j << " has inconsistent residual capacity " << std::fabs(residual_capacity_[j] - expected_residual) << std::endl;
      return false;
    }
    if (!factory_open_[j] && used_capacity > tolerance) {
      std::cerr << "Facility " << j << " is closed but has assigned customers." << std::endl;
      return false;
    }
  }
  for (int i = 0; i < customer_count; ++i) {
    for (int j = 0; j < facility_count; ++j) {
      bool should_be_assigned = assignment_fraction_[i][j] > tolerance;
      if (assignment_[i][j] != should_be_assigned) {
        std::cerr << "Inconsistency between fraction and binary assignment for customer " << i
                  << " and facility " << j << "." << std::endl;
        return false;
      }
      if (assignment_fraction_[i][j] < -tolerance ||
          assignment_fraction_[i][j] > 1.0 + tolerance) {
        std::cerr << "Invalid assignment fraction for customer " << i
                  << " and facility " << j << "." << std::endl;
        return false;
      }
    }
  }
  for (int j = 0; j < facility_count; ++j) {
    const std::vector<int>& customers = clients_of_[j];
    for (int a = 0; a < static_cast<int>(customers.size()); ++a) {
      for (int b = a + 1; b < static_cast<int>(customers.size()); ++b) {
        if (instance_.AreCustomersIncompatible(customers[a], customers[b])) {
          std::cerr << "Incompatibility violation between customers " << customers[a]
                    << " and " << customers[b] << " in facility " << j << "." << std::endl;
          return false;
        }
      }
    }
  }

  return true;
}

/**
 * @brief Computes the objective value from scratch.
 *
 * @return Recomputed objective value.
 */
double MsCflpCiSolution::ComputeObjectiveFromScratch() const {
  double fixed_cost = 0.0;
  double transport_cost = 0.0;
  for (int j = 0; j < GetFacilityCount(); ++j) {
    if (factory_open_[j]) {
      fixed_cost += instance_.GetFacilityOpeningCost(j);
    }
  }
  for (int i = 0; i < GetCustomerCount(); ++i) {
    double demand = instance_.GetCustomerDemand(i);
    for (int j = 0; j < GetFacilityCount(); ++j) {
      transport_cost += instance_.GetAssignmentCost(i, j) * demand * assignment_fraction_[i][j];
    }
  }

  return fixed_cost + transport_cost;
}

/**
 * @brief Checks whether the incremental objective matches the recomputed one.
 *
 * @param tolerance Numerical tolerance.
 * @return True if both values match, false otherwise.
 */
bool MsCflpCiSolution::CheckObjectiveConsistency(double tolerance) const {
  return std::fabs(ComputeObjectiveFromScratch() - total_cost_) <= tolerance;
}

/**
 * @brief Counts the number of open facilities.
 *
 * @return Number of open facilities.
 */
int MsCflpCiSolution::CountOpenFacilities() const {
  int count = 0;
  for (int j = 0; j < GetFacilityCount(); ++j) {
    if (factory_open_[j]) {
      ++count;
    }
  }

  return count;
}

/**
 * @brief Counts incompatibility violations in the current solution.
 *
 * @return Number of violating pairs inside facilities.
 */
int MsCflpCiSolution::CountIncompatibilityViolations() const {
  int violations = 0;
  for (int j = 0; j < GetFacilityCount(); ++j) {
    const std::vector<int>& customers = clients_of_[j];
    for (int a = 0; a < static_cast<int>(customers.size()); ++a) {
      for (int b = a + 1; b < static_cast<int>(customers.size()); ++b) {
        if (instance_.AreCustomersIncompatible(customers[a], customers[b])) {
          ++violations;
        }
      }
    }
  }

  return violations;
}

/**
 * @brief Checks whether a customer identifier is valid.
 *
 * @param customer_id Customer identifier.
 * @return True if the identifier is valid, false otherwise.
 */
bool MsCflpCiSolution::IsValidCustomerId(int customer_id) const {
  return customer_id >= 0 && customer_id < GetCustomerCount();
}

/**
 * @brief Checks whether a facility identifier is valid.
 *
 * @param facility_id Facility identifier.
 * @return True if the identifier is valid, false otherwise.
 */
bool MsCflpCiSolution::IsValidFacilityId(int facility_id) const {
  return facility_id >= 0 && facility_id < GetFacilityCount();
}

/**
 * @brief Checks whether a customer appears in the list of a facility.
 *
 * @param customer_id Customer identifier.
 * @param facility_id Facility identifier.
 * @return True if the customer is already in the facility list, false otherwise.
 */
bool MsCflpCiSolution::HasCustomerInFacility(int customer_id, int facility_id) const {
  const std::vector<int>& customers = clients_of_[facility_id];
  return std::find(customers.begin(), customers.end(), customer_id) != customers.end();
}

/**
 * @brief Adds the bidirectional links between a customer and a facility.
 *
 * @param customer_id Customer identifier.
 * @param facility_id Facility identifier.
 */
void MsCflpCiSolution::AddCustomerToFacilityLists(int customer_id, int facility_id) {
  if (!HasCustomerInFacility(customer_id, facility_id)) {
    clients_of_[facility_id].push_back(customer_id);
  }
  std::vector<int>& facilities = facilities_of_[customer_id];
  if (std::find(facilities.begin(), facilities.end(), facility_id) ==
      facilities.end()) {
    facilities.push_back(facility_id);
  }
}

/**
 * @brief Removes the bidirectional links between a customer and a facility.
 *
 * @param customer_id Customer identifier.
 * @param facility_id Facility identifier.
 */
void MsCflpCiSolution::RemoveCustomerFromFacilityLists(int customer_id, int facility_id) {
  std::vector<int>& customers = clients_of_[facility_id];
  customers.erase(std::remove(customers.begin(), customers.end(), customer_id), customers.end());

  std::vector<int>& facilities = facilities_of_[customer_id];
  facilities.erase(std::remove(facilities.begin(), facilities.end(), facility_id), facilities.end());
}

/**
 * @brief Increases incompatibility counters when a customer starts being served
 * by a facility.
 *
 * @param customer_id Customer identifier.
 * @param facility_id Facility identifier.
 */
void MsCflpCiSolution::IncreaseIncompatibilityCounters(int customer_id, int facility_id) {
  const std::vector<int>& incompatible_customers = instance_.GetIncompatibleCustomers(customer_id);
  for (int k = 0; k < static_cast<int>(incompatible_customers.size()); ++k) {
    int incompatible_customer = incompatible_customers[k];
    ++incompatibility_count_[incompatible_customer][facility_id];
  }
}

/**
 * @brief Decreases incompatibility counters when a customer stops being served
 * by a facility.
 *
 * @param customer_id Customer identifier.
 * @param facility_id Facility identifier.
 */
void MsCflpCiSolution::DecreaseIncompatibilityCounters(int customer_id, int facility_id) {
  const std::vector<int>& incompatible_customers = instance_.GetIncompatibleCustomers(customer_id);
  for (int k = 0; k < static_cast<int>(incompatible_customers.size()); ++k) {
    int incompatible_customer = incompatible_customers[k];
    --incompatibility_count_[incompatible_customer][facility_id];
  }
}

/**
 * @brief Returns the assigned amount from a facility to a customer.
 *
 * @param customer_id Customer identifier.
 * @param facility_id Facility identifier.
 * @return Assigned amount in demand units.
 */
double MsCflpCiSolution::GetAssignedAmount(int customer_id, int facility_id) const {
  return assignment_fraction_[customer_id][facility_id] * instance_.GetCustomerDemand(customer_id);
}

/**
 * @brief Returns the sum of assigned fractions of a customer.
 *
 * @param customer_id Customer identifier.
 * @return Sum of assigned fractions.
 */
double MsCflpCiSolution::GetCustomerAssignedFractionSum(int customer_id) const {
  double sum = 0.0;
  for (int j = 0; j < GetFacilityCount(); ++j) {
    sum += assignment_fraction_[customer_id][j];
  }

  return ClampFraction(sum);
}

/**
 * @brief Returns the used capacity of a facility.
 *
 * @param facility_id Facility identifier.
 * @return Used capacity in demand units.
 */
double MsCflpCiSolution::GetFacilityUsedCapacity(int facility_id) const {
  double used_capacity = 0.0;
  for (int i = 0; i < GetCustomerCount(); ++i) {
    used_capacity += GetAssignedAmount(i, facility_id);
  }

  return ClampNearZero(used_capacity);
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
bool MsCflpCiSolution::CanSwapCustomersBetweenFacilities(
    int customer_a, int facility_a, int customer_b, int facility_b) const {
  if (facility_a == facility_b) {
    return false;
  }

  if (!IsCustomerFullySatisfied(customer_a) || !IsCustomerFullySatisfied(customer_b)) {
    return false;
  }

  if (facilities_of_[customer_a].size() != 1 || facilities_of_[customer_b].size() != 1) {
    return false;
  }

  if (!assignment_[customer_a][facility_a] || !assignment_[customer_b][facility_b]) {
    return false;
  }

  const double amount_a = GetAssignedAmount(customer_a, facility_a);
  const double amount_b = GetAssignedAmount(customer_b, facility_b);
  const double demand_a = instance_.GetCustomerDemand(customer_a);
  const double demand_b = instance_.GetCustomerDemand(customer_b);

  if (std::fabs(amount_a - demand_a) > kAmountTolerance) {
    return false;
  }
  if (std::fabs(amount_b - demand_b) > kAmountTolerance) {
    return false;
  }

  MsCflpCiSolution tmp(*this);

  if (!tmp.RemoveFlow(customer_a, facility_a, amount_a)) return false;
  if (!tmp.RemoveFlow(customer_b, facility_b, amount_b)) return false;
  if (!tmp.AddFlow(customer_a, facility_b, amount_a)) return false;
  if (!tmp.AddFlow(customer_b, facility_a, amount_b)) return false;

  return tmp.IsFeasible();
}

double MsCflpCiSolution::EvaluateSwapDelta(int customer_a, int facility_a, int customer_b, int facility_b) const {
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

/**
 * @brief Checks whether two facilities can be swapped.
 *
 * This move is interpreted as:
 * - closing the currently open facility,
 * - opening the currently closed facility,
 * - reassigning all customers served by the open facility to the closed one.
 *
 * @param source_facility First facility identifier.
 * @param target_facility Second facility identifier.
 * @return True if the swap is feasible, false otherwise.
 */
bool MsCflpCiSolution::CanSwapFacilities(int source_facility, int target_facility) const {
  if (!IsValidFacilityId(source_facility) || !IsValidFacilityId(target_facility)) {
    return false;
  }
  if (source_facility == target_facility) {
    return false;
  }

  int open_facility = -1;
  int closed_facility = -1;
  if (factory_open_[source_facility] && !factory_open_[target_facility]) {
    open_facility = source_facility;
    closed_facility = target_facility;
  } else if (!factory_open_[source_facility] && factory_open_[target_facility]) {
    open_facility = target_facility;
    closed_facility = source_facility;
  } else {
    return false;
  }

  const double used_capacity =
      instance_.GetFacilityCapacity(open_facility) - residual_capacity_[open_facility];
  if (used_capacity - instance_.GetFacilityCapacity(closed_facility) > kAmountTolerance) {
    return false;
  }

  return true;
}

/**
 * @brief Evaluates the objective delta of a facility swap move.
 *
 * This move is interpreted as:
 * - closing the currently open facility,
 * - opening the currently closed facility,
 * - reassigning to the new facility all flow currently served by the old one.
 *
 * @param source_facility First facility identifier.
 * @param target_facility Second facility identifier.
 * @return Delta value of the move.
 */
double MsCflpCiSolution::EvaluateFacilitiesSwapDelta(int source_facility, int target_facility) const {
  int open_facility = -1;
  int closed_facility = -1;
  if (factory_open_[source_facility] && !factory_open_[target_facility]) {
    open_facility = source_facility;
    closed_facility = target_facility;
  } else {
    open_facility = target_facility;
    closed_facility = source_facility;
  }

  double delta = 0.0;
  delta += instance_.GetFacilityOpeningCost(closed_facility);
  delta -= instance_.GetFacilityOpeningCost(open_facility);

  const std::vector<int>& customers_in_open_facility = clients_of_[open_facility];
  for (int customer_id : customers_in_open_facility) {
    const double amount = GetAssignedAmount(customer_id, open_facility);
    delta += amount * (instance_.GetAssignmentCost(customer_id, closed_facility) -
                       instance_.GetAssignmentCost(customer_id, open_facility));
  }

  return delta;
}

/**
 * @brief Applies a facility swap move and returns the resulting solution.
 *
 * This move is interpreted as:
 * - closing the currently open facility,
 * - opening the currently closed facility,
 * - reassigning to the new facility all flow currently served by the old one.
 *
 * @param source_facility First facility identifier.
 * @param target_facility Second facility identifier.
 * @return True if successfully applied, false otherwise.
 */
bool MsCflpCiSolution::SwapFacilities(int source_facility, int target_facility) {
  if (!CanSwapFacilities(source_facility, target_facility)) {
    return false;
  }

  int open_facility = -1;
  int closed_facility = -1;
  if (factory_open_[source_facility] && !factory_open_[target_facility]) {
    open_facility = source_facility;
    closed_facility = target_facility;
  } else {
    open_facility = target_facility;
    closed_facility = source_facility;
  }
  if (!OpenFacility(closed_facility)) {
    return false;
  }

  const std::vector<int> customers_in_open_facility = GetCustomersInFacility()[open_facility];
  for (int customer_id : customers_in_open_facility) {
    const double amount =
        GetCustomerFacilityFraction(customer_id, open_facility) *
        GetInstance().GetCustomerDemand(customer_id);
    if (amount <= kAmountTolerance) {
      continue;
    }
    if (!RemoveFlow(customer_id, open_facility, amount) ||
        !AddFlow(customer_id, closed_facility, amount)) {
      return false;
    }
  }
  if (!CloseFacility(open_facility)) {
    return false;
  }

  return true;
}