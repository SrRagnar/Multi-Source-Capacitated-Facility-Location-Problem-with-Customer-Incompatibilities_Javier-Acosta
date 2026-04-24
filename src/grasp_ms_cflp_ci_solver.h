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
 public:
  GraspMsCflpCiSolver(unsigned cardinality = 1) : lcr_cardinality_{cardinality} {}
  ~GraspMsCflpCiSolver() override = default;
 protected:
  void Preprocess(Instance* input) override;
  Solution* ConstructSolution(Instance* input) override;
  Solution* Postprocess(Solution* solution) override;
  void UpdateBest(Solution* current, Solution*& best) override;
  bool StopCriterion() override;
 
 private:
  unsigned current_grasp_iteration_ = 0;
  unsigned max_grasp_iterations_ = 100;
  unsigned lcr_cardinality_ = 1;

  unsigned FindSlackValue(const MsCflpCiInstance& instance) const;
  std::vector<int> GetSortedFacilitiesByScore(const MsCflpCiInstance& instance, const MsCflpCiSolution& solution) const;
  std::vector<int> GetSortedFacilitiesByCostForCustomer(const MsCflpCiSolution& solution, int customer_id) const;

  MsCflpCiSolution* ExploreShiftNeighborhood(MsCflpCiSolution* solution) const;
  MsCflpCiSolution* ExploreClientSwapNeighborhood(MsCflpCiSolution* solution) const;
  MsCflpCiSolution* ExploreFacilitySwapNeighborhood(MsCflpCiSolution* solution) const;
  MsCflpCiSolution* ExploreIncompatibilitiesRemoveNeighborhood(MsCflpCiSolution* solution) const;
};

#endif