// University of La Laguna
// School of Engineering and Technology
// Bachelor's Degree in Computer Engineering
// Subject: Design and Analysis of Algorithms
// Course: 3rd
// Practice 6: Multi-Source Capacitated Facility Location Problem with Customer Incompatibilities
// Author: Javier Acosta Portocarrero
// Date: 04/04/2026
// File instance.h: declaration file.
// Contains the declaration of the Instance class.

#ifndef INSTANCE_H_
#define INSTANCE_H_

/**
  * @class Instance
  * 
  * @brief Abstract interface class for the instances received by the algorithms,
  *        thus following the strategy pattern.  
  */
class Instance {
 public:
  virtual ~Instance() = default;
};

#endif