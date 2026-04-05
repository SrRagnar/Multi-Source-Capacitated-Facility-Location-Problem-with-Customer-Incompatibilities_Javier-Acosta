// // University of La Laguna
// School of Engineering and Technology
// Bachelor's Degree in Computer Engineering
// Subject: Design and Analysis of Algorithms
// Course: 3rd
// Practice 6: Multi-Source Capacitated Facility Location Problem with Customer Incompatibilities
// Author: Javier Acosta Portocarrero
// Date: 04/04/2026
// File greedy_ms_cflp_ci_solver.cc: implementation file.
// Contains the implementation of the GreedyMsCflpCiSolver class.

#include <stdexcept>

#include "greedy_ms_cflp_ci_solver.h"
#include "ms_cflp_ci_solution.h"
#include "ms_cflp_ci_instance.h"

Solution* GreedyMsCflpCiSolver::Solve(Instance* input) {
  MsCflpCiInstance* instance = dynamic_cast<MsCflpCiInstance*>(input);
  if (instance == nullptr) {
    throw std::invalid_argument("Input instance is not of type MsCflpCiInstance.");
  }
  MsCflpCiSolution* greedy_solution = new MsCflpCiSolution(*instance);
  const MsCflpCiInstance& original_instance = greedy_solution->GetInstance();
  
  // Fase 1, Facilities selection.
  const std::vector<int>& facilities_by_opening_cost = original_instance.GetFacilitiesSortedByOpeningCost();
  const double total_demand = original_instance.GetTotalDemand();
  double accumulated_capacity = 0.0;
  size_t facility_index = 0;
  for (; facility_index < facilities_by_opening_cost.size(); ++facility_index) {
    const int facility = facilities_by_opening_cost[facility_index];
    if (accumulated_capacity >= total_demand) {
      break;
    }
    greedy_solution->OpenFacility(facility);
    accumulated_capacity += original_instance.GetFacilityCapacity(facility);
  }
  // Add a slack to treat possible incompatibilities.
  const unsigned slack_parameter = 5;  // k
  for (size_t i = facility_index; i < (facility_index + slack_parameter) && i < facilities_by_opening_cost.size(); ++i) {
    const int facility = facilities_by_opening_cost[i];
    greedy_solution->OpenFacility(facility);
  }
  
  // Fase 2, Customer assignment.
  std::vector<double> customer_demands = original_instance.GetCustomerDemands();  // Its dynamic.
  for (size_t customer_id = 0; customer_id < original_instance.GetCustomerCount(); ++customer_id) {
    const std::vector<int>& facilities_by_current_cost = original_instance.GetFacilitiesSortedByCostForCustomer(customer_id);
    size_t factory_index = 0;
    while (customer_demands[customer_id] > 0 && factory_index < facilities_by_current_cost.size()) {
      const int facility = facilities_by_current_cost[factory_index];
      const double amount_to_assign = std::min(
          greedy_solution->GetResidualCapacity(facility),
          customer_demands[customer_id]);
          
      if (greedy_solution->CanAddFlow(customer_id, facility, amount_to_assign)) {
        greedy_solution->AddFlow(customer_id, facility, amount_to_assign);
        customer_demands[customer_id] -= amount_to_assign;
      }
      ++factory_index;
    }
  }
  return greedy_solution;
}