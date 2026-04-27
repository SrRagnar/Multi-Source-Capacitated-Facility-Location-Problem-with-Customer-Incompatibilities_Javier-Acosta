// University of La Laguna
// School of Engineering and Technology
// Bachelor's Degree in Computer Engineering
// Subject: Design and Analysis of Algorithms
// Course: 3rd
// Practice 6: Multi-Source Capacitated Facility Location Problem with Customer Incompatibilities
// Author: Javier Acosta Portocarrero
// Date: 27/04/2026
// File ms_cflp_ci_solution_saver.cc: implementation file.
// Contains the implementation of the MsCflpCiSolutionSaver class.

#include <fstream>
#include <iomanip>
#include <stdexcept>
#include <vector>

#include "ms_cflp_ci_solution_saver.h"

/**
 * @brief Saves a solution into the configured file.
 *
 * The method opens the file in append mode, so the new solution is added at the
 * end of the file without deleting previous solutions.
 *
 * @param solution Solution to save.
 * @param instance_name Name of the instance solved.
 * @param algorithm Algorithm used to obtain the solution.
 * @param lrc_size Cardinality of the restricted candidate list.
 * @param execution Execution number.
 * @param cpu_time CPU time used to obtain the solution.
 *
 * @throw std::runtime_error If the output file cannot be opened.
 */
void MsCflpCiSolutionSaver::Save(const MsCflpCiSolution& solution,
                                 const std::string& instance_name,
                                 const std::string& algorithm,
                                 int lrc_size,
                                 int execution,
                                 double cpu_time) const {
  std::ofstream output_file(file_path_, std::ios::app);
  if (!output_file) {
    throw std::runtime_error("Could not open solution output file: " + file_path_);
  }

  output_file << "============================================================\n";
  output_file << "Instance: " << instance_name << "\n";
  output_file << "Algorithm: " << algorithm << "\n";
  output_file << "LRC size: " << lrc_size << "\n";
  output_file << "Execution: " << execution << "\n";

  output_file << std::fixed << std::setprecision(6);
  output_file << "CPU time: " << cpu_time << "\n";

  output_file << std::fixed << std::setprecision(2);
  output_file << "Fixed cost: " << solution.GetFixedCost() << "\n";
  output_file << "Transport cost: " << solution.GetTransportCost() << "\n";
  output_file << "Total cost: " << solution.GetTotalCost() << "\n";
  output_file << "Objective from scratch: " << solution.ComputeObjectiveFromScratch() << "\n";
  output_file << "Open facilities: " << solution.CountOpenFacilities() << "\n";
  output_file << "Incompatibility violations: " << solution.CountIncompatibilityViolations() << "\n";
  output_file << "Feasible: " << (solution.IsFeasible() ? "yes" : "no") << "\n\n";

  output_file << "Open facilities detail:\n";
  for (int facility_id = 0; facility_id < solution.GetFacilityCount(); ++facility_id) {
    if (!solution.IsFacilityOpen(facility_id)) {
      continue;
    }
    output_file << "  Facility " << facility_id
                << " | residual capacity: " << solution.GetResidualCapacity(facility_id)
                << " | used capacity: "
                << solution.GetInstance().GetFacilityCapacity(facility_id) - solution.GetResidualCapacity(facility_id)
                << "\n";
  }

  output_file << "\nCustomer assignments:\n";
  for (int customer_id = 0; customer_id < solution.GetCustomerCount(); ++customer_id) {
    output_file << "  Customer " << customer_id
                << " | demand: " << solution.GetInstance().GetCustomerDemand(customer_id)
                << "\n";

    const std::vector<int>& facilities = solution.GetFacilitiesOf()[customer_id];
    for (int facility_id : facilities) {
      const double amount = solution.GetAssignedAmount(customer_id, facility_id);
      const double fraction = solution.GetCustomerFacilityFraction(customer_id, facility_id);
      const double unit_cost = solution.GetInstance().GetAssignmentCost(customer_id, facility_id);

      output_file << "    -> Facility " << facility_id
                  << " | amount: " << amount
                  << " | fraction: " << fraction
                  << " | unit cost: " << unit_cost
                  << " | cost: " << amount * unit_cost
                  << "\n";
    }
  }

  output_file << "\n";
}