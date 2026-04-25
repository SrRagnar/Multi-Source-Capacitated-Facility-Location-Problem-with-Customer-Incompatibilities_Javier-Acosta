// University of La Laguna
// School of Engineering and Technology
// Bachelor's Degree in Computer Engineering
// Subject: Design and Analysis of Algorithms
// Course: 3rd
// Practice 6: Multi-Source Capacitated Facility Location Problem with Customer
// Incompatibilities
// Author: Javier Acosta Portocarrero
// Date: 24/04/2026
// File ms_cflp_ci_perturbation_strategy.h: declaration file.
// Contains the declaration of the MsCflpCiPerturbationStrategy class.

#ifndef MS_CFLP_CI_PERTURBATION_STRATEGY_H_
#define MS_CFLP_CI_PERTURBATION_STRATEGY_H_

#include "../solutions/ms_cflp_ci_solution.h"

class MsCflpCiPerturbationStrategy {
 public:
  virtual ~MsCflpCiPerturbationStrategy() = default;

  virtual MsCflpCiSolution* Perturbate(const MsCflpCiSolution* solution, double amount_tol, double improvement_tol) const = 0;
};

 #endif