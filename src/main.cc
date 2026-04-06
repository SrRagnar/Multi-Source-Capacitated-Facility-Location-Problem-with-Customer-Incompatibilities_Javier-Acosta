// University of La Laguna
// School of Engineering and Technology
// Bachelor's Degree in Computer Engineering
// Subject: Design and Analysis of Algorithms
// Course: 3rd
// Practice 6: Multi-Source Capacitated Facility Location Problem with Customer Incompatibilities
// Author: Javier Acosta Portocarrero
// Date: 04/04/2026
// File main.cc: implementation file.
// Contains the implementation of the main function of the program.

#include <chrono>
#include <ctime>
#include <iomanip>
#include <iostream>
#include <stdexcept>
#include <string>
#include <vector>

#include "ms_cflp_ci_instance_csi_loader.h"
#include "ms_cflp_ci_general_solver.h"
#include "greedy_ms_cflp_ci_solver.h"
#include "grasp_ms_cflp_ci_solver.h"

/**
 * @brief Extracts the file name from a path.
 *
 * @param path Full path to the file.
 * @return File name only.
 */
std::string GetInstanceName(const std::string& path) {
  std::size_t pos = path.find_last_of("/\\");
  if (pos == std::string::npos) {
    return path;
  }
  return path.substr(pos + 1);
}

/**
 * @brief Prints the table header.
 */
void PrintTableHeader() {
  std::cout
      << std::left
      << std::setw(16) << "Instancia"
      << std::setw(8) << "|LRC|"
      << std::setw(8) << "Ejec."
      << std::setw(10) << "|J_open|"
      << std::setw(12) << "C. Fijo"
      << std::setw(12) << "C. Asig."
      << std::setw(12) << "C. Total"
      << std::setw(10) << "Incomp."
      << std::setw(12) << "CPU_Time"
      << std::endl;

  std::cout << std::string(100, '-') << std::endl;
}

/**
 * @brief Main function of the program, responsible for loading an instance from a file,
 *        solving it using a selected strategy, and printing a results table.
 *
 * @param argc Number of command-line arguments.
 * @param argv Array of command-line argument strings.
 * @return Exit code.
 */
int main(int argc, char* argv[]) {
  if (argc != 3) {
    std::cerr << "Usage: " << argv[0]
              << " <instance_file.dzn> <grasp|greedy>" << std::endl;
    return 1;
  }

  std::srand(static_cast<unsigned>(std::time(nullptr)));

  try {
    const std::string instance_path = argv[1];
    const std::string algorithm = argv[2];
    const std::string instance_name = GetInstanceName(instance_path);
    const std::vector<int> lrc_sizes = {2, 3, 5};
    const int executions = 3;

    PrintTableHeader();
    for (std::size_t lrc_index = 0; lrc_index < lrc_sizes.size(); ++lrc_index) {
      const int lrc_size = lrc_sizes[lrc_index];
      for (int execution = 1; execution <= executions; ++execution) {
        MsCflpCiInstanceCsiLoader loader(instance_path);
        MsCflpCiInstance* instance = loader.Load();
        auto start = std::chrono::high_resolution_clock::now();
        MsCflpCiGeneralSolver* solver = nullptr;
        if (algorithm == "grasp") {
          solver = new MsCflpCiGeneralSolver(new GraspMsCflpCiSolver(lrc_size));
        } else if (algorithm == "greedy") {
          solver = new MsCflpCiGeneralSolver(new GreedyMsCflpCiSolver());
        } else {
          delete instance;
          throw std::invalid_argument("Unknown algorithm: " + algorithm + " (use 'grasp' or 'greedy')");
        }
        MsCflpCiSolution* solution = solver->SolveMsCflpCiInstance(instance);
        auto end = std::chrono::high_resolution_clock::now();
        double cpu_time = std::chrono::duration<double>(end - start).count();
        if (solution == nullptr) {
          std::cout
              << std::left
              << std::setw(16) << instance_name
              << std::setw(8) << lrc_size
              << std::setw(8) << execution
              << std::setw(10) << "-"
              << std::setw(12) << "-"
              << std::setw(12) << "-"
              << std::setw(12) << "-"
              << std::setw(10) << "-"
              << std::setw(12) << std::fixed << std::setprecision(6) << cpu_time
              << std::endl;

          delete solver;
          delete instance;
          continue;
        }
        if (!solution->IsFeasible()) {
          std::cerr << "ERROR: SOLUTION NOT FEASIBLE." << std::endl;
        }
        std::cout
            << std::left
            << std::setw(16) << instance_name
            << std::setw(8) << lrc_size
            << std::setw(8) << execution
            << std::setw(10) << solution->CountOpenFacilities()
            << std::setw(12) << std::fixed << std::setprecision(2) << solution->GetFixedCost()
            << std::setw(12) << std::fixed << std::setprecision(2) << solution->GetTransportCost()
            << std::setw(12) << std::fixed << std::setprecision(2) << solution->GetTotalCost()
            << std::setw(10) << solution->CountIncompatibilityViolations()
            << std::setw(12) << std::fixed << std::setprecision(6) << cpu_time
            << std::endl;
        delete solution;
        delete solver;
        delete instance;
      }

      if (lrc_index + 1 < lrc_sizes.size()) {
        std::cout << std::string(100, '-') << std::endl;
      }
    }
  } catch (const std::exception& e) {
    std::cerr << "Error: " << e.what() << std::endl;
    return 1;
  }

  return 0;
}