// University of La Laguna
// School of Engineering and Technology
// Bachelor's Degree in Computer Engineering
// Subject: Design and Analysis of Algorithms
// Course: 3rd
// Practice 6: Multi-Source Capacitated Facility Location Problem with Customer Incompatibilities
// Author: Javier Acosta Portocarrero
// Date: 12/04/2026
// File grasp-ms-cflp-ci-vnd_solver.h: declaration file.
// Contains the declaration of the GraspMsCflpCiVndSolver class.

#ifndef GRAASP_MS_CFLP_CI_VND_SOLVER_H_
#define GRAASP_MS_CFLP_CI_VND_SOLVER_H_

#include "grasp_ms_cflp_ci_solver.h"

class GraspMsCflpCiVndSolver : public GraspMsCflpCiSolver {
 public:
  GraspMsCflpCiVndSolver(unsigned cardinality = 1,
                         std::vector<MsCflpCiNeighboorhodExplorer*> explorers = {}) :
      GraspMsCflpCiSolver(cardinality, explorers) {}
      
  ~GraspMsCflpCiVndSolver() override = default;

 protected:
  Solution* Postprocess(Solution* solution) override;
};

#endif