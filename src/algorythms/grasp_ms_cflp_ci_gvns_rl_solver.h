// University of La Laguna
// School of Engineering and Technology
// Bachelor's Degree in Computer Engineering
// Subject: Design and Analysis of Algorithms
// Course: 3rd
// Practice 6: Multi-Source Capacitated Facility Location Problem with Customer Incompatibilities
// Author: Javier Acosta Portocarrero
// Date: 25/04/2026
// File grasp-ms-cflp-ci-gvns_rl_solver.h: declaration file.
// Contains the declaration of the GraspMsCflpCiGvnsRlSolver class.

#ifndef GRAASP_MS_CFLP_CI_GVNS_RL_SOLVER_H_
#define GRAASP_MS_CFLP_CI_GVNS_RL_SOLVER_H_

#include "grasp_ms_cflp_ci_solver.h"
#include "../perturbators/ms_cflp_ci_perturbation_strategy.h"

class GraspMsCflpCiGvnsRl : public GraspMsCflpCiSolver {
 public:
  GraspMsCflpCiGvnsRl(unsigned cardinality = 1,
                      std::vector<MsCflpCiNeighboorhodExplorer*> explorers = {},
                      std::vector<MsCflpCiPerturbationStrategy*> perturbators = {})  :
      GraspMsCflpCiSolver(cardinality, explorers), perturbators_{perturbators} {}
  ~GraspMsCflpCiGvnsRl() override = default;

 protected:
  Solution* Postprocess(Solution* solution) override;
 
 private:
  std::vector<MsCflpCiPerturbationStrategy*> perturbators_;
  const unsigned max_rl_vnd_iter_ = 10000;
  const unsigned max_rl_vnd_iter_without_improvement_ = 25;
  const unsigned max_gvns_iter_ = 1000;
  const unsigned max_gvns_iter_without_improvement_ = 100;
  // Used for the epsilon-greedy policy used to decide to explote or explore.
  const double epsilon_ = 0.2;
  const double learning_rate_ = 0.2;
  const double initial_confidence_ = 0.5;

  MsCflpCiSolution* VndWithReinforcementLearning(MsCflpCiSolution* Solution) const;
  double CalculateBinaryReward(double previus_cost, MsCflpCiSolution* new_solution) const;
  double CalculateProportionalReward(double previus_cost, MsCflpCiSolution* new_solution) const;
};

#endif