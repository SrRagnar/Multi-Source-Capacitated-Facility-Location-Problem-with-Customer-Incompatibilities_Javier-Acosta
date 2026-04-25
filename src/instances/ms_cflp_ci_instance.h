// University of La Laguna
// School of Engineering and Technology
// Bachelor's Degree in Computer Engineering
// Subject: Design and Analysis of Algorithms
// Course: 3rd
// Practice 6: Multi-Source Capacitated Facility Location Problem with Customer Incompatibilities
// Author: Javier Acosta Portocarrero
// Date: 04/04/2026
// File ms_cflp_ci_instance.h: declaration file.
// Contains the declaration of the MsCflpCiInstance class.

#ifndef MS_CFLP_CI_INSTANCE_H_
#define MS_CFLP_CI_INSTANCE_H_

#include <vector>

#include "instance.h"

/**
 * @class MsCflpCiInstance
 *
 * @brief Class that represents an instance of the Multi-Source Capacitated
 * Facility Location Problem with Customer Incompatibilities.
 * It also stores auxiliary structures to speed up greedy, GRASP and local
 * search algorithms.
 */
class MsCflpCiInstance : public Instance {
 public:
   MsCflpCiInstance(int customer_count, int facility_count,
                   const std::vector<double>& customer_demands,
                   const std::vector<double>& facility_opening_costs,
                   const std::vector<double>& facility_capacities,
                   const std::vector<std::vector<double> >& assignment_costs,
                   const std::vector<std::pair<int, int> >& incompatibility_pairs);
  ~MsCflpCiInstance() override = default;

  int GetCustomerCount() const;
  int GetFacilityCount() const;
  double GetCustomerDemand(int customer_id) const;
  double GetFacilityOpeningCost(int facility_id) const;
  double GetFacilityCapacity(int facility_id) const;
  double GetAssignmentCost(int customer_id, int facility_id) const;

  const std::vector<int>& GetIncompatibleCustomers(int customer_id) const;
  const std::vector<std::pair<int, int> >& GetIncompatibilityPairs() const;
  const std::vector<int>& GetFacilitiesSortedByOpeningCost() const;
  const std::vector<int>& GetFacilitiesSortedByCostForCustomer(int customer_id) const;
  const std::vector<double>& GetCustomerDemands() const { return customer_demands_;}
  double GetTotalDemand() const;
  double GetTotalCapacity() const;
  double GetAverageAssignmentCostByFacility(int facility_id) const;

  bool IsValid() const;
  bool AreCustomersIncompatible(int customer_1, int customer_2) const;

 private:
  void BuildAuxiliaryStructures();
  void BuildIncompatibilityStructures();
  void BuildFacilityOrderings();

  bool IsValidCustomerId(int customer_id) const;
  bool IsValidFacilityId(int facility_id) const;
  bool ValidateDimensions() const;
  bool ValidateCustomerData() const;
  bool ValidateFacilityData() const;
  bool ValidateAssignmentCosts() const;
  bool ValidateIncompatibilities() const;
  bool ValidateAuxiliaryStructures() const;
  // Problem size.
  int customer_count_ = 0;
  int facility_count_ = 0;
  std::vector<double> customer_demands_;
  std::vector<double> facility_opening_costs_;
  std::vector<double> facility_capacities_;
  // Assignment costs: assignment_costs_[i][j] = cost from facility j to customer i.
  std::vector<std::vector<double> > assignment_costs_;
  std::vector<std::pair<int, int> > incompatibility_pairs_;
  // incompatible_customers_[i] contains all customers incompatible with i.
  std::vector<std::vector<int> > incompatible_customers_;
  std::vector<std::vector<bool> > incompatible_matrix_;

  std::vector<int> facilities_sorted_by_opening_cost_;
  std::vector<std::vector<int> > facilities_sorted_by_customer_cost_;

  double total_demand_ = 0.0;
  double total_capacity_ = 0.0;
};

#endif  