// University of La Laguna
// School of Engineering and Technology
// Bachelor's Degree in Computer Engineering
// Subject: Design and Analysis of Algorithms
// Course: 3rd
// Practice 6: Multi-Source Capacitated Facility Location Problem with Customer Incompatibilities
// Author: Javier Acosta Portocarrero
// Date: 04/04/2026
// File grasp-algorythm.h: declaration file.
// Contains the declaration of the GraspAlgorythm, class.

#ifndef GRAASP_ALGORYTHM_H_
#define GRAASP_ALGORYTHM_H_

#include "algorythm.h"

/**
  * @class GraspAlgorythm
  * 
  * @brief Abstract class that implements the GRASP metaheuristic, it follows the template method pattern.
  *        The concrete implementations are responsible for defining the specific steps of the algorithm.
  */
class GraspAlgorythm : public Algorythm {
 public:
  Solution* Solve(Instance* input) override;

 protected:
  virtual void Preprocess(Instance* input) = 0;
  virtual Solution* ConstructSolution(Instance* input) = 0;
  virtual Solution* Postprocess(Solution* solution) = 0;
  virtual void UpdateBest(Solution* current, Solution*& best) = 0;
  virtual bool StopCriterion() = 0;
};

#endif