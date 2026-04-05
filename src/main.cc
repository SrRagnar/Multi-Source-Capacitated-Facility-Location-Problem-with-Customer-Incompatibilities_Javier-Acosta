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

#include <iostream>
#include <stdexcept>
#include <string>

#include "ms_cflp_ci_instance_csi_loader.h"
#include "ms_cflp_ci_general_solver.h"
#include "greedy_ms_cflp_ci_solver.h"

/**
 * @brief Main function of the program, responsible for loading an instance from a file,
 *        solving it using a selected strategy, and printing the total cost of the solution.
 *
 * @param argc Number of command-line arguments.
 * @param argv Array of command-line argument strings.
 * @return Exit code.
 */
int main(int argc, char* argv[]) {
  if (argc != 2) {
    std::cerr << "Usage: " << argv[0] << " <instance_file.dzn>" << std::endl;
    return 1;
  }
  try {
    MsCflpCiInstanceCsiLoader loader(argv[1]);
    MsCflpCiInstance* instance = loader.Load();
    MsCflpCiGeneralSolver solver(new GreedyMsCflpCiSolver());
    MsCflpCiSolution* solution = solver.SolveMsCflpCiInstance(instance);
    if (solution->IsFeasible()) {
      std::cout << "Solution is feasible." << std::endl;
    } else {
      std::cout << "Solution is not feasible." << std::endl;
      return 1;
    } 
    std::cout << "Total Cost: " << solution->GetTotalCost() << std::endl;
    delete solution;
    delete instance;
  } catch (const std::exception& e) {
    std::cerr << "Error: " << e.what() << std::endl;
    return 1;
  }

  return 0;
}