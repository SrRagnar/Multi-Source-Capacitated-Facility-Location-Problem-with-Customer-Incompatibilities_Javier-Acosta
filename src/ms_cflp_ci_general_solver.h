// University of La Laguna
// School of Engineering and Technology
// Bachelor's Degree in Computer Engineering
// Subject: Design and Analysis of Algorithms
// Course: 3rd
// Practice 6: Multi-Source Capacitated Facility Location Problem with Customer Incompatibilities
// Author: Javier Acosta Portocarrero
// Date: 04/04/2026
// File ms_cflp_ci_general_solver.h: declaration file.
// Contains the declaration of the MsCflpCiSolver class.

#ifndef MS_CFLP_CI_SOLVER_GENERAL_H_
#define MS_CFLP_CI_SOLVER_GENERAL_H_

#include "algorythm.h"
#include "ms_cflp_ci_instance.h"
#include "ms_cflp_ci_solution.h"

class MsCflpCiGeneralSolver {
 public:
  MsCflpCiGeneralSolver(Algorythm* strategy) : strategy_(strategy) {}
  ~MsCflpCiGeneralSolver() = default;

  MsCflpCiSolution* SolveMsCflpCiInstance(MsCflpCiInstance* input) const;
  void SetStrategy(Algorythm* strategy) { strategy_ = strategy; }
 private: 
  Algorythm* strategy_;
};

#endif