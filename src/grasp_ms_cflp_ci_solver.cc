// University of La Laguna
// School of Engineering and Technology
// Bachelor's Degree in Computer Engineering
// Subject: Design and Analysis of Algorithms
// Course: 3rd
// Practice 6: Multi-Source Capacitated Facility Location Problem with Customer Incompatibilities
// Author: Javier Acosta Portocarrero
// Date: 04/04/2026
// File grasp-ms-cflp-ci-solver.cc: implementation file.
// Contains the implementation of the GraspMsCflpCiSolver class.

#include <vector>
#include <stdexcept>
#include <algorithm>
#include <random>

#include "grasp_ms_cflp_ci_solver.h"
#include "ms_cflp_ci_solution.h"
#include "ms_cflp_ci_instance.h"

void GraspMsCflpCiSolver::Preprocess(Instance* input) {
  return;
}

Solution* GraspMsCflpCiSolver::ConstructSolution(Instance* input) {
  MsCflpCiInstance* instance = dynamic_cast<MsCflpCiInstance*>(input);
  if (instance == nullptr) {
    throw std::invalid_argument("Input instance is not of type MsCflpCiInstance.");
  }
  MsCflpCiSolution* greedy_solution = new MsCflpCiSolution(*instance);
  const MsCflpCiInstance& original_instance = greedy_solution->GetInstance();
  
  // Fase 1, Facilities selection.
  const double total_demand = original_instance.GetTotalDemand();
  double accumulated_capacity = 0.0;
  const unsigned cardinality_parameter = 5;
  while (accumulated_capacity < total_demand) {
    std::vector<int> facilities_by_score = GetSortedFacilitiesByScore(original_instance, *greedy_solution);
    if (facilities_by_score.empty()) {
      break;
    }
    unsigned current_rcl_size = std::min(
        cardinality_parameter,
        static_cast<unsigned>(facilities_by_score.size()));

    unsigned facility_index = std::rand() % current_rcl_size;
    int facility = facilities_by_score[facility_index];
    if (!greedy_solution->IsFacilityOpen(facility)) {
      greedy_solution->OpenFacility(facility);
      accumulated_capacity += original_instance.GetFacilityCapacity(facility);
    }
  }
  // Add a slack to treat possible incompatibilities.
  unsigned slack_parameter = FindSlackValue(original_instance);
  while (slack_parameter > 0) {
    std::vector<int> facilities_by_score = GetSortedFacilitiesByScore(original_instance, *greedy_solution);
    if (facilities_by_score.empty()) {
      break;
    }
    unsigned current_rcl_size = std::min(
        slack_parameter,
        static_cast<unsigned>(facilities_by_score.size()));

    unsigned facility_index = std::rand() % current_rcl_size;
    int facility = facilities_by_score[facility_index];
    if (!greedy_solution->IsFacilityOpen(facility)) {
      greedy_solution->OpenFacility(facility);
      --slack_parameter;
    }
  }
  
  // Fase 2, Customer assignment.
  std::vector<double> customer_demands = original_instance.GetCustomerDemands();
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

void GraspMsCflpCiSolver::Postprocess(Solution* solution) {
  // Not implemented yet
}

void GraspMsCflpCiSolver::UpdateBest(Solution* current, Solution*& best) {
  // Not implemented yet
}

bool GraspMsCflpCiSolver::StopCriterion() const {
  return true;
}

unsigned GraspMsCflpCiSolver::FindSlackValue(const MsCflpCiInstance& instance) const {
  const unsigned total_incompatibilities = instance.GetIncompatibilityPairs().size();
  if (total_incompatibilities == 0) {
    return 0;  // No slack needed if there are no incompatibilities.
  }
  // Heuristic: set slack as a small percentage of the total incompatibilities, with a maximum cap.
  const unsigned slack = static_cast<unsigned>(total_incompatibilities * 0.1);  // 10% of total incompatibilities
  return std::min(slack, static_cast<unsigned>(instance.GetFacilityCount() * 0.2));  // Cap at 20% of total facilities
}

std::vector<int> GraspMsCflpCiSolver::GetSortedFacilitiesByScore(const MsCflpCiInstance& instance, const MsCflpCiSolution& solution) const {

  const double opening_cost_factor = 0.1;
  const double average_assignment_cost_factor = 0.8;
  const double incompatibility_penalty_factor = 0.1;

  std::vector<std::pair<int, double>> facilities_with_scores;
  // Penalty based on incompatibility density, to prefer facilities that may cause fewer issues.
  const double incompatibility_density = static_cast<double>(instance.GetIncompatibilityPairs().size()) /
                                         (instance.GetCustomerCount() + 1.0);

  for (int facility_id = 0; facility_id < instance.GetFacilityCount(); ++facility_id) {
    // Don't consider already open facilities.
    if (solution.IsFacilityOpen(facility_id)) {
      continue;
    }
    double score = opening_cost_factor * instance.GetFacilityOpeningCost(facility_id) +
        average_assignment_cost_factor * instance.GetAverageAssignmentCostByFacility(facility_id) +
        incompatibility_penalty_factor * incompatibility_density;

    facilities_with_scores.emplace_back(facility_id, score);
  }
  std::sort(facilities_with_scores.begin(),
            facilities_with_scores.end(),
            [](const std::pair<int, double>& a,
               const std::pair<int, double>& b) {
              return a.second < b.second;
            });

  std::vector<int> facilities_by_score;
  for (size_t i = 0; i < facilities_with_scores.size(); ++i) {
    facilities_by_score.push_back(facilities_with_scores[i].first);
  }

  return facilities_by_score;
}