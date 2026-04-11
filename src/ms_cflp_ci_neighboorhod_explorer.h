// University of La Laguna
// School of Engineering and Technology
// Bachelor's Degree in Computer Engineering
// Subject: Design and Analysis of Algorithms
// Course: 3rd
// Practice 6: Multi-Source Capacitated Facility Location Problem with Customer
// Incompatibilities
// Author: Javier Acosta Portocarrero
// Date: 10/04/2026
// File ms_cflp_ci_neighboorhod_explorer.h: declaration file.
// Contains the declaration of the MsCflpCiNeighboorhodExplorer class.

#ifndef MS_CFLP_CI_NEIGHBOORHOOD_EXPLORER_H_
#define MS_CFLP_CI_NEIGHBOORHOOD_EXPLORER_H_

#include "ms_cflp_ci_solution.h"

class MsCflpCiNeighboorhodExplorer {
 public:
  virtual ~MsCflpCiNeighboorhodExplorer() = default;

  virtual MsCflpCiSolution* Explore(const MsCflpCiSolution* solution, double amount_tol, double improvement_tol) const = 0;
};

 #endif