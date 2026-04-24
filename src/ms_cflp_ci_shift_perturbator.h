// University of La Laguna
// School of Engineering and Technology
// Bachelor's Degree in Computer Engineering
// Subject: Design and Analysis of Algorithms
// Course: 3rd
// Practice 6: Multi-Source Capacitated Facility Location Problem with Customer
// Incompatibilities
// Author: Javier Acosta Portocarrero
// Date: 24/04/2026
// File ms_cflp_ci_shift_perturbator.h: declaration file.
// Contains the declaration of the MsCflpCiShiftPerturbator class.

#ifndef MS_CFLP_CI_SHIFT_PERTURBATOR_H_
#define MS_CFLP_CI_SHIFT_PERTURBATOR_H_

#include "ms_cflp_ci_solution.h"
#include "ms_cflp_ci_perturbation_strategy.h"

class MsCflpCiShiftPerturbator : MsCflpCiPerturbationStrategy  {
 public:
  ~MsCflpCiShiftPerturbator() = default;

  MsCflpCiSolution* Perturbate(const MsCflpCiSolution* solution, double amount_tol, double improvement_tol) const override;
  
 private:
  bool CanShiftFlow(const MsCflpCiSolution& solution, 
                    int customer_id, int source_facility, int target_facility, double amount,
                    double amount_tol, double improvement_tol) const;

  bool ShiftFlow(MsCflpCiSolution& solution, 
                 int customer_id, int source_facility, int target_facility, double amount,
                 double amount_tol, double improvement_tol) const;
};

#endif