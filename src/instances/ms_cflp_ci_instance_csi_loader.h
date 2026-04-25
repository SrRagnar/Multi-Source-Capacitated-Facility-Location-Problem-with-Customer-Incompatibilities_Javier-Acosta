// University of La Laguna
// School of Engineering and Technology
// Bachelor's Degree in Computer Engineering
// Subject: Design and Analysis of Algorithms
// Course: 3rd
// Practice 6: Multi-Source Capacitated Facility Location Problem with Customer
// Incompatibilities
// Author: Javier Acosta Portocarrero
// Date: 04/04/2026
// File ms_cflp_ci_instance_csi_loader.h: declaration file.
// Contains the declaration of the MsCflpCiInstanceCsiLoader class.

#ifndef MS_CFLP_CI_INSTANCE_CSI_LOADER_H_
#define MS_CFLP_CI_INSTANCE_CSI_LOADER_H_

#include <string>
#include <vector>

#include "ms_cflp_ci_instance.h"
#include "ms_cflp_ci_instance_loader.h"

/**
 * @class MsCflpCiInstanceCsiLoader
 *
 * @brief Class responsible for loading MS-CFLP-CI instances from a .dzn file.
 */
class MsCflpCiInstanceCsiLoader : public MsCflpCiInstanceLoader {
 public:
  /**
   * @brief Builds a loader for a given file path.
   *
   * @param file_path Path of the instance file.
   */
  explicit MsCflpCiInstanceCsiLoader(const std::string& file_path) : file_path_(file_path) {}
  ~MsCflpCiInstanceCsiLoader() override = default;

  /**
   * @brief Loads an instance from the configured file.
   *
   * @return Pointer to the loaded instance.
   */
  MsCflpCiInstance* Load() const override;
  /**
   * @brief Sets the file path.
   *
   * @param file_path Path of the instance file.
   */
  void SetFilePath(const std::string& file_path) { file_path_ = file_path; }

 private:
  std::string ReadFileContent() const;
  std::string ExtractFieldValue(const std::string& content, const std::string& field_name) const;
  std::string Trim(const std::string& text) const;
  std::vector<std::string> Split(const std::string& text, char delimiter) const;

  int ReadIntegerField(const std::string& content, const std::string& field_name) const;
  std::vector<double> ReadDoubleVectorField(const std::string& content, const std::string& field_name) const;
  std::vector<std::vector<double>> ReadDoubleMatrixField(
      const std::string& content, const std::string& field_name,
      int expected_rows, int expected_columns) const;
  int ReadIncompatibilityCount(const std::string& content) const;

  bool HasField(const std::string& content, const std::string& field_name) const;
  std::vector<std::pair<int, int>> ReadIncompatibilityPairs(const std::string& content) const;

  std::string file_path_;
};

#endif  