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
  // Not implemented, because its not necessary for the current implementation.
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
  std::vector<int> facilities_by_score = GetSortedFacilitiesByScore(original_instance);  

  const double total_demand = original_instance.GetTotalDemand();
  double accumulated_capacity = 0.0;
  const unsigned cardinality_parameter = 5;  // k best elements, defines the LRC size.
  size_t facility_index = 0;
  // Randomly select a facility from the best k facilities from the sorted list until the demand is covered.
  while (accumulated_capacity < total_demand && !facilities_by_score.empty()) {
    facility_index = std::rand() % std::min(cardinality_parameter, static_cast<unsigned>(facilities_by_score.size()));
    const int facility = facilities_by_score[facility_index];
    greedy_solution->OpenFacility(facility);
    accumulated_capacity += original_instance.GetFacilityCapacity(facility);
    facilities_by_score.erase(facilities_by_score.begin() + facility_index);
  }
  // Add a slack to treat possible incompatibilities.
  unsigned slack_parameter = FindSlackValue(original_instance);
  while (!facilities_by_score.empty() && slack_parameter > 0) {
    facility_index = std::rand() % std::min(slack_parameter, static_cast<unsigned>(facilities_by_score.size()));
    const int facility = facilities_by_score[facility_index];
    greedy_solution->OpenFacility(facility);
    facilities_by_score.erase(facilities_by_score.begin() + facility_index);
    --slack_parameter;
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
  // Implementation for postprocessing
}

void GraspMsCflpCiSolver::UpdateBest(Solution* current, Solution*& best) {
  // Implementation for updating best solution
}

bool GraspMsCflpCiSolver::StopCriterion() const {
  // Implementation for stop criterion, not implemented yet.
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

std::vector<int> GraspMsCflpCiSolver::GetSortedFacilitiesByScore(const MsCflpCiInstance& instance) const {
  const double opening_cost_factor = 0.1;
  const double average_assignment_cost_factor = 0.9;
  std::vector<std::pair<int, double>> facilities_with_scores;

  // Compute a score for each facility based on the factors stablished, a lower scores is better.
  for (int facility_id = 0; facility_id < instance.GetFacilityCount(); ++facility_id) {
    double score = opening_cost_factor * instance.GetFacilityOpeningCost(facility_id) +
                   average_assignment_cost_factor * instance.GetAverageAssignmentCostByFacility(facility_id);
    facilities_with_scores.emplace_back(facility_id, score);
  }
  std::sort(facilities_with_scores.begin(), facilities_with_scores.end(),
            [](const std::pair<int, double>& a, const std::pair<int, double>& b) {
              return a.second < b.second;  // Ascending order by score
            });
  std::vector<int> facilities_by_score;
   for (const auto& pair : facilities_with_scores) {
    facilities_by_score.push_back(pair.first);
  }
  return facilities_by_score;
}