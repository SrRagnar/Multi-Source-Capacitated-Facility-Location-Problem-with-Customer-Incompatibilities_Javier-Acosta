// University of La Laguna
// School of Engineering and Technology
// Bachelor's Degree in Computer Engineering
// Subject: Design and Analysis of Algorithms
// Course: 3rd
// Practice 6: Multi-Source Capacitated Facility Location Problem with Customer
// Incompatibilities
// Author: Javier Acosta Portocarrero
// Date: 04/04/2026
// File ms_cflp_ci_instance_csi_loader.cc: implementation file.
// Contains the implementation of the MsCflpCiInstanceCsiLoader class.

#include <fstream>
#include <sstream>
#include <stdexcept>

#include "ms_cflp_ci_instance_csi_loader.h"

/**
 * @brief Loads an instance from the configured file.
 *
 * The method reads the main fields of the instance and, if available,
 * also reads the incompatibility pairs between customers.
 *
 * @return Pointer to the loaded instance.
 *
 * @throw std::runtime_error If the file cannot be opened or its format is not
 * valid.
 */
MsCflpCiInstance* MsCflpCiInstanceCsiLoader::Load() const {
  std::string content = ReadFileContent();
  int facility_count = ReadIntegerField(content, "Warehouses");
  int customer_count = ReadIntegerField(content, "Stores");

  std::vector<double> facility_capacities = ReadDoubleVectorField(content, "Capacity");
  std::vector<double> facility_opening_costs = ReadDoubleVectorField(content, "FixedCost");
  std::vector<double> customer_demands = ReadDoubleVectorField(content, "Goods");
  std::vector<std::vector<double> > assignment_costs =
      ReadDoubleMatrixField(content, "SupplyCost",
                            customer_count, facility_count);
  std::vector<std::pair<int, int> > incompatibility_pairs = ReadIncompatibilityPairs(content);

  MsCflpCiInstance* instance = new MsCflpCiInstance(
      customer_count, facility_count, customer_demands,
      facility_opening_costs, facility_capacities,
      assignment_costs, incompatibility_pairs);
  if (!instance->IsValid()) {
    delete instance;
    throw std::runtime_error("The loaded instance is not valid.");
  }

  return instance;
}

/**
 * @brief Reads the full content of the instance file.
 *
 * @return File content as a string.
 *
 * @throw std::runtime_error If the file cannot be opened.
 */
std::string MsCflpCiInstanceCsiLoader::ReadFileContent() const {
  std::ifstream input_file(file_path_);
  if (!input_file) {
    throw std::runtime_error("Could not open file: " + file_path_);
  }
  std::stringstream buffer;
  buffer << input_file.rdbuf();
  return buffer.str();
}

/**
 * @brief Extracts the raw value associated with a field.
 *
 * @param content Full content of the file.
 * @param field_name Name of the field to extract.
 * @return Raw text of the field value, without the final semicolon.
 *
 * @throw std::runtime_error If the field is not found or is malformed.
 */
std::string MsCflpCiInstanceCsiLoader::ExtractFieldValue(const std::string& content, const std::string& field_name) const {
  std::string key = field_name + " =";
  std::size_t start = content.find(key);
  if (start == std::string::npos) {
    throw std::runtime_error("Field not found: " + field_name);
  }
  start = content.find('=', start);
  if (start == std::string::npos) {
    throw std::runtime_error("Invalid field definition: " + field_name);
  }
  ++start;
  std::size_t end = content.find(';', start);
  if (end == std::string::npos) {
    throw std::runtime_error("Missing ';' in field: " + field_name);
  }

  return content.substr(start, end - start);
}

/**
 * @brief Removes leading and trailing blank spaces from a string.
 *
 * @param text Input string.
 * @return Trimmed string.
 */
std::string MsCflpCiInstanceCsiLoader::Trim(const std::string& text) const {
  std::size_t begin = 0;
  while (begin < text.size() && std::isspace(static_cast<unsigned char>(text[begin]))) {
    ++begin;
  }
  std::size_t end = text.size();
  while (end > begin && std::isspace(static_cast<unsigned char>(text[end - 1]))) {
    --end;
  }

  return text.substr(begin, end - begin);
}

/**
 * @brief Splits a string using a delimiter.
 *
 * @param text Input string.
 * @param delimiter Delimiter character.
 * @return Vector of substrings.
 */
std::vector<std::string> MsCflpCiInstanceCsiLoader::Split(const std::string& text, char delimiter) const {
  std::vector<std::string> parts;
  std::stringstream stream(text);
  std::string token;
  while (std::getline(stream, token, delimiter)) {
    parts.push_back(token);
  }

  return parts;
}

/**
 * @brief Reads an integer field from the file content.
 *
 * @param content Full content of the file.
 * @param field_name Name of the field.
 * @return Parsed integer value.
 */
int MsCflpCiInstanceCsiLoader::ReadIntegerField(const std::string& content, const std::string& field_name) const {
  std::string value = ExtractFieldValue(content, field_name);
  return std::stoi(Trim(value));
}

/**
 * @brief Reads a vector of double values from the file content.
 *
 * @param content Full content of the file.
 * @param field_name Name of the field.
 * @return Parsed vector of doubles.
 *
 * @throw std::runtime_error If the vector format is invalid.
 */
std::vector<double> MsCflpCiInstanceCsiLoader::ReadDoubleVectorField(const std::string& content, const std::string& field_name) const {
  std::string value = Trim(ExtractFieldValue(content, field_name));
  if (value.size() < 2 || value[0] != '[' || value[value.size() - 1] != ']') {
    throw std::runtime_error("Invalid vector format in field: " + field_name);
  }
  value = value.substr(1, value.size() - 2);
  std::vector<std::string> tokens = Split(value, ',');
  std::vector<double> result;
  for (int i = 0; i < static_cast<int>(tokens.size()); ++i) {
    std::string token = Trim(tokens[i]);
    if (!token.empty()) {
      result.push_back(std::stod(token));
    }
  }

  return result;
}

/**
 * @brief Reads a matrix of double values from the file content.
 *
 * @param content Full content of the file.
 * @param field_name Name of the field.
 * @param expected_rows Expected number of rows.
 * @param expected_columns Expected number of columns.
 * @return Parsed matrix of doubles.
 *
 * @throw std::runtime_error If the matrix format is invalid or its dimensions do
 * not match the expected ones.
 */
std::vector<std::vector<double> > MsCflpCiInstanceCsiLoader::ReadDoubleMatrixField(
    const std::string& content, const std::string& field_name,
    int expected_rows, int expected_columns) const {
  std::string value = Trim(ExtractFieldValue(content, field_name));
  if (value.size() < 2 || value[0] != '[' || value[value.size() - 1] != ']') {
    throw std::runtime_error("Invalid matrix format in field: " + field_name);
  }
  value = value.substr(1, value.size() - 2);
  std::vector<std::string> raw_rows = Split(value, '|');
  std::vector<std::vector<double> > matrix;

  for (int i = 0; i < static_cast<int>(raw_rows.size()); ++i) {
    std::string row_text = Trim(raw_rows[i]);
    if (row_text.empty()) {
      continue;
    }
    std::vector<std::string> row_tokens = Split(row_text, ',');
    std::vector<double> row;
    for (int j = 0; j < static_cast<int>(row_tokens.size()); ++j) {
      std::string token = Trim(row_tokens[j]);
      if (!token.empty()) {
        row.push_back(std::stod(token));
      }
    }
    if (!row.empty()) {
      matrix.push_back(row);
    }
  }
  if (static_cast<int>(matrix.size()) != expected_rows) {
    throw std::runtime_error("Unexpected number of rows in field: " + field_name);
  }
  for (int i = 0; i < expected_rows; ++i) {
    if (static_cast<int>(matrix[i].size()) != expected_columns) {
      throw std::runtime_error("Unexpected number of columns in field: " + field_name);
    }
  }

  return matrix;
}

/**
 * @brief Checks whether a field exists in the file content.
 *
 * @param content Full content of the file.
 * @param field_name Name of the field.
 * @return True if the field exists, false otherwise.
 */
bool MsCflpCiInstanceCsiLoader::HasField(const std::string& content, const std::string& field_name) const {
  std::string key = field_name + " =";
  return content.find(key) != std::string::npos;
}

/**
 * @brief Reads the incompatibility pairs if the corresponding field exists.
 *
 * @param content Full content of the file.
 * @return Vector of incompatibility pairs.
 *
 * @throw std::runtime_error If the field exists but its format is invalid.
 */
std::vector<std::pair<int, int>> MsCflpCiInstanceCsiLoader::ReadIncompatibilityPairs(
    const std::string& content) const {
  std::vector<std::pair<int, int> > incompatibility_pairs;
  std::string field_name = "";
  if (HasField(content, "IncompatiblePairs")) {
    field_name = "IncompatiblePairs";
  } else if (HasField(content, "IncompatibilityPairs")) {
    field_name = "IncompatibilityPairs";
  } else if (HasField(content, "IncompatibilityPairsSet")) {
    field_name = "IncompatibilityPairsSet";
  } else {
    return incompatibility_pairs;
  }

  std::string value = Trim(ExtractFieldValue(content, field_name));
  if (value.size() < 2 || value[0] != '[' || value[value.size() - 1] != ']') {
    throw std::runtime_error("Invalid incompatibility pair field format.");
  }
  value = value.substr(1, value.size() - 2);
  std::vector<std::string> raw_rows = Split(value, '|');
  for (int i = 0; i < static_cast<int>(raw_rows.size()); ++i) {
    std::string row_text = Trim(raw_rows[i]);
    if (row_text.empty()) {
      continue;
    }
    std::vector<std::string> tokens = Split(row_text, ',');
    if (tokens.size() != 2) {
      throw std::runtime_error("Invalid incompatibility pair format.");
    }
    int customer_1 = std::stoi(Trim(tokens[0])) - 1;
    int customer_2 = std::stoi(Trim(tokens[1])) - 1;
    incompatibility_pairs.push_back(std::make_pair(customer_1, customer_2));
  }
  
  int incompatibility_count = ReadIncompatibilityCount(content);
  if (incompatibility_count != -1 && incompatibility_count != static_cast<int>(incompatibility_pairs.size())) {
    throw std::runtime_error("The number of incompatibility pairs does not match the value specified.");
  }

  return incompatibility_pairs;
}

/**
 * @brief Reads the number of incompatibilities declared in the file.
 *
 * @param content Full content of the file.
 * @return Number of incompatibilities, or -1 if the field is not present.
 */
int MsCflpCiInstanceCsiLoader::ReadIncompatibilityCount(const std::string& content) const {
  if (!HasField(content, "Incompatibilities")) {
    return -1;
  }

  return ReadIntegerField(content, "Incompatibilities");
}