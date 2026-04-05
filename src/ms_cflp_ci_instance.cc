// University of La Laguna
// School of Engineering and Technology
// Bachelor's Degree in Computer Engineering
// Subject: Design and Analysis of Algorithms
// Course: 3rd
// Practice 6: Multi-Source Capacitated Facility Location Problem with Customer
// Incompatibilities
// Author: Javier Acosta Portocarrero
// Date: 04/04/2026
// File ms_cflp_ci_instance.cc: implementation file.
// Contains the implementation of the MsCflpCiInstance class.

#include "ms_cflp_ci_instance.h"

#include <algorithm>  // std::sort
#include <stdexcept>

/**
 * @brief Builds an instance from the essential input data.
 *
 * @param customer_count Number of customers.
 * @param facility_count Number of facilities.
 * @param customer_demands Demand of each customer.
 * @param facility_opening_costs Fixed opening cost of each facility.
 * @param facility_capacities Capacity of each facility.
 * @param assignment_costs Assignment cost matrix.
 * @param incompatibility_pairs Incompatibility pairs between customers.
 */
MsCflpCiInstance::MsCflpCiInstance(
    int customer_count, int facility_count,
    const std::vector<double>& customer_demands,
    const std::vector<double>& facility_opening_costs,
    const std::vector<double>& facility_capacities,
    const std::vector<std::vector<double> >& assignment_costs,
    const std::vector<std::pair<int, int> >& incompatibility_pairs)
    : customer_count_{customer_count},
      facility_count_{facility_count},
      customer_demands_{customer_demands},
      facility_opening_costs_{facility_opening_costs},
      facility_capacities_{facility_capacities},
      assignment_costs_{assignment_costs},
      incompatibility_pairs_{incompatibility_pairs} {

  BuildAuxiliaryStructures();
}

/**
 * @brief Returns the number of customers in the instance.
 *
 * @return Number of customers.
 */
int MsCflpCiInstance::GetCustomerCount() const {
  return customer_count_;
}

/**
 * @brief Returns the number of facilities in the instance.
 *
 * @return Number of facilities.
 */
int MsCflpCiInstance::GetFacilityCount() const {
  return facility_count_;
}

/**
 * @brief Returns the demand of a given customer.
 *
 * @param customer_id Customer identifier.
 * @return Demand of the customer.
 *
 * @throw std::out_of_range If the customer identifier is not valid.
 */
double MsCflpCiInstance::GetCustomerDemand(int customer_id) const {
  if (!IsValidCustomerId(customer_id)) {
    throw std::out_of_range("Invalid customer id.");
  }
  return customer_demands_[customer_id];
}

/**
 * @brief Returns the fixed opening cost of a given facility.
 *
 * @param facility_id Facility identifier.
 * @return Opening cost of the facility.
 *
 * @throw std::out_of_range If the facility identifier is not valid.
 */
double MsCflpCiInstance::GetFacilityOpeningCost(int facility_id) const {
  if (!IsValidFacilityId(facility_id)) {
    throw std::out_of_range("Invalid facility id.");
  }
  return facility_opening_costs_[facility_id];
}

/**
 * @brief Returns the capacity of a given facility.
 *
 * @param facility_id Facility identifier.
 * @return Capacity of the facility.
 *
 * @throw std::out_of_range If the facility identifier is not valid.
 */
double MsCflpCiInstance::GetFacilityCapacity(int facility_id) const {
  if (!IsValidFacilityId(facility_id)) {
    throw std::out_of_range("Invalid facility id.");
  }
  return facility_capacities_[facility_id];
}

/**
 * @brief Returns the assignment cost from a facility to a customer.
 *
 * @param customer_id Customer identifier.
 * @param facility_id Facility identifier.
 * @return Assignment cost c_ij.
 *
 * @throw std::out_of_range If any identifier is not valid.
 */
double MsCflpCiInstance::GetAssignmentCost(int customer_id, int facility_id) const {
  if (!IsValidCustomerId(customer_id)) {
    throw std::out_of_range("Invalid customer id.");
  }
  if (!IsValidFacilityId(facility_id)) {
    throw std::out_of_range("Invalid facility id.");
  }
  return assignment_costs_[customer_id][facility_id];
}

/**
 * @brief Returns the list of customers incompatible with a given customer.
 *
 * @param customer_id Customer identifier.
 * @return Constant reference to the list of incompatible customers.
 *
 * @throw std::out_of_range If the customer identifier is not valid.
 */
const std::vector<int>& MsCflpCiInstance::GetIncompatibleCustomers(int customer_id) const {
  if (!IsValidCustomerId(customer_id)) {
    throw std::out_of_range("Invalid customer id.");
  }
  return incompatible_customers_[customer_id];
}

/**
 * @brief Returns the list of incompatibility pairs of the instance.
 *
 * @return Constant reference to the vector of incompatibility pairs.
 */
const std::vector<std::pair<int, int> >& MsCflpCiInstance::GetIncompatibilityPairs() const {
  return incompatibility_pairs_;
}

/**
 * @brief Returns the facilities sorted by increasing opening cost.
 *
 * @return Constant reference to the sorted facilities.
 */
const std::vector<int>& MsCflpCiInstance::GetFacilitiesSortedByOpeningCost() const {
  return facilities_sorted_by_opening_cost_;
}

/**
 * @brief Returns the facilities sorted by assignment cost for a customer.
 *
 * @param customer_id Customer identifier.
 * @return Constant reference to the sorted facilities for that customer.
 *
 * @throw std::out_of_range If the customer identifier is not valid.
 */
const std::vector<int>& MsCflpCiInstance::GetFacilitiesSortedByCostForCustomer(int customer_id) const {
  if (!IsValidCustomerId(customer_id)) {
    throw std::out_of_range("Invalid customer id.");
  }
  return facilities_sorted_by_customer_cost_[customer_id];
}

/**
 * @brief Returns the total demand of the instance.
 *
 * @return Sum of all customer demands.
 */
double MsCflpCiInstance::GetTotalDemand() const {
  return total_demand_;
}

/**
 * @brief Returns the total capacity of the instance.
 *
 * @return Sum of all facility capacities.
 */
double MsCflpCiInstance::GetTotalCapacity() const {
  return total_capacity_;
}

/**
 * @brief Checks whether two customers are incompatible.
 *
 * @param customer_1 First customer.
 * @param customer_2 Second customer.
 * @return True if both customers are incompatible, false otherwise.
 *
 * @throw std::out_of_range If any customer identifier is not valid.
 */
bool MsCflpCiInstance::AreCustomersIncompatible(int customer_1, int customer_2) const {
  if (!IsValidCustomerId(customer_1) || !IsValidCustomerId(customer_2)) {
    throw std::out_of_range("Invalid customer id.");
  }
  return incompatible_matrix_[customer_1][customer_2];
}

/**
 * @brief Validates the consistency of the instance data.
 *
 * @return True if the instance is valid, false otherwise.
 */
bool MsCflpCiInstance::IsValid() const {
  return ValidateDimensions() &&
         ValidateCustomerData() &&
         ValidateFacilityData() &&
         ValidateAssignmentCosts() &&
         ValidateIncompatibilities() &&
         ValidateAuxiliaryStructures();
}

/**
 * @brief Validates basic dimensions of the instance.
 */
bool MsCflpCiInstance::ValidateDimensions() const {
  if (customer_count_ < 0 || facility_count_ < 0) {
    return false;
  }
  if (static_cast<int>(customer_demands_.size()) != customer_count_) {
    return false;
  }
  if (static_cast<int>(facility_opening_costs_.size()) != facility_count_) {
    return false;
  }
  if (static_cast<int>(facility_capacities_.size()) != facility_count_) {
    return false;
  }
  if (static_cast<int>(assignment_costs_.size()) != customer_count_) {
    return false;
  }
  return true;
}

/**
 * @brief Validates customer data.
 */
bool MsCflpCiInstance::ValidateCustomerData() const {
  for (int i = 0; i < customer_count_; ++i) {
    if (customer_demands_[i] < 0.0) {
      return false;
    }
  }
  return true;
}

/**
 * @brief Validates facility data.
 */
bool MsCflpCiInstance::ValidateFacilityData() const {
  for (int j = 0; j < facility_count_; ++j) {
    if (facility_opening_costs_[j] < 0.0) {
      return false;
    }
    if (facility_capacities_[j] < 0.0) {
      return false;
    }
  }
  return true;
}

/**
 * @brief Validates assignment cost matrix.
 */
bool MsCflpCiInstance::ValidateAssignmentCosts() const {
  for (int i = 0; i < customer_count_; ++i) {
    if (static_cast<int>(assignment_costs_[i].size()) != facility_count_) {
      return false;
    }

    for (int j = 0; j < facility_count_; ++j) {
      if (assignment_costs_[i][j] < 0.0) {
        return false;
      }
    }
  }

  return true;
}

/**
 * @brief Validates incompatibility pairs and structures.
 */
bool MsCflpCiInstance::ValidateIncompatibilities() const {
  for (std::size_t k = 0; k < incompatibility_pairs_.size(); ++k) {
    int c1 = incompatibility_pairs_[k].first;
    int c2 = incompatibility_pairs_[k].second;
    if (!IsValidCustomerId(c1) || !IsValidCustomerId(c2)) {
      return false;
    }
    if (c1 == c2) {
      return false;
    }
  }
  if (static_cast<int>(incompatible_customers_.size()) != customer_count_) {
    return false;
  }
  if (static_cast<int>(incompatible_matrix_.size()) != customer_count_) {
    return false;
  }
  for (int i = 0; i < customer_count_; ++i) {
    if (static_cast<int>(incompatible_matrix_[i].size()) != customer_count_) {
      return false;
    }
  }
  return true;
}

/**
 * @brief Validates auxiliary preprocessed structures.
 */
bool MsCflpCiInstance::ValidateAuxiliaryStructures() const {
  if (static_cast<int>(facilities_sorted_by_opening_cost_.size()) != facility_count_ 
      || static_cast<int>(facilities_sorted_by_customer_cost_.size()) != customer_count_) {
    return false;
  }
  for (int i = 0; i < customer_count_; ++i) {
    if (static_cast<int>(facilities_sorted_by_customer_cost_[i].size()) != facility_count_) {
      return false;
    }
  }

  return true;
}

/**
 * @brief Builds all auxiliary structures of the instance.
 *
 * This method should be called once the raw data of the instance has already
 * been loaded.
 */
void MsCflpCiInstance::BuildAuxiliaryStructures() {
  total_demand_ = 0.0;
  total_capacity_ = 0.0;
  for (int i = 0; i < customer_count_; ++i) {
    total_demand_ += customer_demands_[i];
  }
  for (int j = 0; j < facility_count_; ++j) {
    total_capacity_ += facility_capacities_[j];
  }
  BuildIncompatibilityStructures();
  BuildFacilityOrderings();
}

/**
 * @brief Builds the incompatibility adjacency list and matrix.
 *
 * The adjacency list is useful to iterate over incompatible customers of a
 * given customer. The matrix allows constant-time compatibility checks.
 */
void MsCflpCiInstance::BuildIncompatibilityStructures() {
  incompatible_customers_.clear();
  incompatible_matrix_.clear();
  incompatible_customers_.resize(customer_count_);
  incompatible_matrix_.resize(customer_count_,
                              std::vector<bool>(customer_count_, false));

  for (std::size_t k = 0; k < incompatibility_pairs_.size(); ++k) {
    int customer_1 = incompatibility_pairs_[k].first;
    int customer_2 = incompatibility_pairs_[k].second;
    if (!IsValidCustomerId(customer_1) || !IsValidCustomerId(customer_2)) {
      continue;
    }
    if (customer_1 == customer_2) {
      continue;
    }
    if (!incompatible_matrix_[customer_1][customer_2]) {
      incompatible_matrix_[customer_1][customer_2] = true;
      incompatible_matrix_[customer_2][customer_1] = true;
      incompatible_customers_[customer_1].push_back(customer_2);
      incompatible_customers_[customer_2].push_back(customer_1);
    }
  }
}

/**
 * @brief Builds facility orderings used by constructive algorithms.
 * Two orderings are generated:
 * - facilities sorted by increasing opening cost,
 * - for each customer, facilities sorted by increasing assignment cost.
 */
void MsCflpCiInstance::BuildFacilityOrderings() {
  facilities_sorted_by_opening_cost_.clear();
  facilities_sorted_by_customer_cost_.clear();
  facilities_sorted_by_opening_cost_.resize(facility_count_);
  for (int j = 0; j < facility_count_; ++j) {
    facilities_sorted_by_opening_cost_[j] = j;
  }
  std::sort(facilities_sorted_by_opening_cost_.begin(),
            facilities_sorted_by_opening_cost_.end(),
            [this](int facility_1, int facility_2) {
              return facility_opening_costs_[facility_1] <
                     facility_opening_costs_[facility_2];
            });
  facilities_sorted_by_customer_cost_.resize(customer_count_);
  for (int i = 0; i < customer_count_; ++i) {
    facilities_sorted_by_customer_cost_[i].resize(facility_count_);
    for (int j = 0; j < facility_count_; ++j) {
      facilities_sorted_by_customer_cost_[i][j] = j;
    }
    std::sort(facilities_sorted_by_customer_cost_[i].begin(),
              facilities_sorted_by_customer_cost_[i].end(),
              [this, i](int facility_1, int facility_2) {
                return assignment_costs_[i][facility_1] <
                       assignment_costs_[i][facility_2];
              });
  }
}

/**
 * @brief Checks whether a customer identifier is valid.
 *
 * @param customer_id Customer identifier.
 * @return True if the identifier is valid, false otherwise.
 */
bool MsCflpCiInstance::IsValidCustomerId(int customer_id) const {
  return customer_id >= 0 && customer_id < customer_count_;
}

/**
 * @brief Checks whether a facility identifier is valid.
 *
 * @param facility_id Facility identifier.
 * @return True if the identifier is valid, false otherwise.
 */
bool MsCflpCiInstance::IsValidFacilityId(int facility_id) const {
  return facility_id >= 0 && facility_id < facility_count_;
}

/**
 * @brief Computes the average assignment cost of a facility.
 *
 * @param facility_id Facility identifier.
 * @return Average assignment cost of the facility.
 *
 * @throw std::out_of_range If the facility identifier is not valid.
 */
double MsCflpCiInstance::GetAverageAssignmentCostByFacility(int facility_id) const {
  if (!IsValidFacilityId(facility_id)) {
    throw std::out_of_range("Invalid facility identifier.");
  }
  double total_cost = 0.0;
  for (int i = 0; i < customer_count_; ++i) {
    total_cost += assignment_costs_[i][facility_id];
  }
  return total_cost / customer_count_;
}
