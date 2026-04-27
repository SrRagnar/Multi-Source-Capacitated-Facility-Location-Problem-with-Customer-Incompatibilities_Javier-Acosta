// University of La Laguna
// School of Engineering and Technology
// Bachelor's Degree in Computer Engineering
// Subject: Design and Analysis of Algorithms
// Course: 3rd
// Practice 6: Multi-Source Capacitated Facility Location Problem with Customer Incompatibilities
// Author: Javier Acosta Portocarrero
// Date: 27/04/2026
// File ms_cflp_ci_solution_saver.h: declaration file.
// Contains the declaration of the MsCflpCiSolutionSaver class.

#ifndef MS_CFLP_CI_SOLUTION_SAVER_H_
#define MS_CFLP_CI_SOLUTION_SAVER_H_

#include <string>

#include "ms_cflp_ci_solution.h"

/**
 * @class MsCflpCiSolutionSaver
 *
 * @brief Class used to save MS-CFLP-CI solutions into an output file.
 *
 * The class appends the solution information to the selected file, so several
 * solutions can be stored in the same file without overwriting previous ones.
 */
class MsCflpCiSolutionSaver {
 public:
  explicit MsCflpCiSolutionSaver(const std::string& file_path) : file_path_(file_path) {}

  void Save(const MsCflpCiSolution& solution,
            const std::string& instance_name,
            const std::string& algorithm,
            int lrc_size,
            int execution,
            double cpu_time) const;

 private:
  std::string file_path_;
};

#endif