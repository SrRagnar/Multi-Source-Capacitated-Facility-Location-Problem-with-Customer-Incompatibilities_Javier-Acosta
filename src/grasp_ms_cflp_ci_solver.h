// University of La Laguna
// School of Engineering and Technology
// Bachelor's Degree in Computer Engineering
// Subject: Design and Analysis of Algorithms
// Course: 3rd
// Practice 6: Multi-Source Capacitated Facility Location Problem with Customer Incompatibilities
// Author: Javier Acosta Portocarrero
// Date: 04/04/2026
// File grasp-ms-cflp-ci-solver.h: declaration file.
// Contains the declaration of the GraspMsCflpCiSolver class.

#ifndef GRAASP_MS_CFLP_CI_SOLVER_H_
#define GRAASP_MS_CFLP_CI_SOLVER_H_

#include "grasp_algorythm.h"
#include "ms_cflp_ci_instance.h"
#include "ms_cflp_ci_solution.h"

/**
 * @class GraspMsCflpCiSolver
 *
 * @brief Class that implements the GRASP metaheuristic to solve the MS-CFLP-CI problem.
 */
class GraspMsCflpCiSolver : public GraspAlgorythm {
 protected:
  void Preprocess(Instance* input) override;
  Solution* ConstructSolution(Instance* input) override;
  void Postprocess(Solution* solution) override;
  void UpdateBest(Solution* current, Solution*& best) override;
  bool StopCriterion() const override;
 
 private:
  unsigned FindSlackValue(const MsCflpCiInstance& instance) const;
  std::vector<int> GetSortedFacilitiesByScore(const MsCflpCiInstance& instance, const MsCflpCiSolution& solution) const;

  MsCflpCiSolution* LocalSeachByShifting(MsCflpCiSolution* solution) const;
  MsCflpCiSolution* LocalSearchBySwapingClients(MsCflpCiSolution* solution) const;
  MsCflpCiSolution* LocalSearchBySwapingFacilities(MsCflpCiSolution* solution) const;
  MsCflpCiSolution* LocalSearchByRemovingIncompatibilities(MsCflpCiSolution* solution) const;
};

#endif