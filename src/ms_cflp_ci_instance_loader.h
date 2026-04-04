// University of La Laguna
// School of Engineering and Technology
// Bachelor's Degree in Computer Engineering
// Subject: Design and Analysis of Algorithms
// Course: 3rd
// Practice 6: Multi-Source Capacitated Facility Location Problem with Customer Incompatibilities
// Author: Javier Acosta Portocarrero
// Date: 04/04/2026
// File ms_cflp_ci_instance_loader.h: declaration file.
// Contains the declaration of the MsCflpCiInstanceLoader class.

#ifndef MS_CFLP_CI_INSTANCE_LOADER_H_
#define MS_CFLP_CI_INSTANCE_LOADER_H_

#include "ms_cflp_ci_instance.h"

/**
 * @class MsCflpCiInstanceLoader
 *
 * @brief Abstract class responsible for loading instances of the Multi-Source Capacitated Facility 
 * Location Problem with Customer Incompatibilities, the concrete implementations are responsible 
 * for the actual reading.
 */
class MsCflpCiInstanceLoader {
 public:
  virtual ~MsCflpCiInstanceLoader() = default;

  /**
   * @brief Loads an instance of the Multi-Source Capacitated Facility Location Problem with Customer Incompatibilities.
   * @return A pointer to the loaded instance.
   */
  virtual MsCflpCiInstance* Load() const = 0;
};

#endif