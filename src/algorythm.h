// University of La Laguna
// School of Engineering and Technology
// Bachelor's Degree in Computer Engineering
// Subject: Design and Analysis of Algorithms
// Course: 3rd
// Practice 6: Multi-Source Capacitated Facility Location Problem with Customer Incompatibilities
// Author: Javier Acosta Portocarrero
// Date: 04/04/2026
// File algorythm.h: declaration file.
// Contains the declaration of the Algoryth, class.

#ifndef ALGORYTHM_H_
#define ALGORYTHM_H_

#include "instance.h"
#include "solution.h"

/**
  * @class Algorythm
  * 
  * @brief Abstract interface class for algorithms, which receive an instance
  *        as input and produce a solution as output. It also follows
  *        the strategy pattern.  
  */
class Algorythm {
 public:

  virtual ~Algorythm() = default; 
 /**
   * @brief Pure virtual method to solve any type of algorithm
   *
   * @param input Pointer to any possible instance that an algorithm can receive
   * @return A pointer to the solution of the problem for that specific instance,
   *         it can be any type of solution that an algorithm can return.
   */
  virtual Solution* Solve(Instance* input) = 0;
};

#endif