/**
 * \file
 * \brief Output of tab-separated text tables.
 * \copyright ...
 * \date 2019
 */
#ifndef FAUNA_OUTPUT_TEXT_TABLE_WRITER_H
#define FAUNA_OUTPUT_TEXT_TABLE_WRITER_H

#include <fstream>
#include <vector>
#include "parameters.h"
#include "writer_interface.h"

namespace Fauna {
namespace Output {

/// Writes output data to tabular plaintext files.
/**
 * In \ref Parameters there are boolean variables prefixed with "text_tables".
 * They are switches to enable and disable the creation of different data
 * tables in plaintext files. The files have the same name as the corresponding
 * boolean variable in \ref TextTableWriterOptions.
 * All files are created in a directory specified by
 * \ref TextTableWriterOptions::directory.
 */
class TextTableWriter : public WriterInterface {
 public:
  /// Constructor
  /**
   * Create all files that are selected in `options` as empty files.
   * \param interval Selector if output is daily/monthly/annual/...
   * \param options Specific user-defined options for this class.
   * \throw std::runtime_error If one of the output files already exists.
   */
  TextTableWriter(const OutputInterval interval,
                  const TextTableWriterOptions& options);

  /// Append spatially & temporally aggregated output data to table files.
  /**
   * \param datapoint The output data to write.
   *
   * \warning `datapoint` must not change its structure between calls. For
   * example, the list of HFTs must not change. This is because the column
   * captions are written in the first call to this function. In subsequent
   * calls there is no check whether the data tuples match the column captions.
   *
   * \throw std::invalid_argument If an HFT name contains whitespaces or the
   * \ref FIELD_SEPARATOR (only checked on first call).
   *
   * \throw std::invalid_argument If `datapoint.data.datapoint_count`
   * is zero.
   *
   * \throw std::invalid_argument If `datapoint.interval` does not match the
   * given \ref OutputInterval.
   *
   * \throw std::invalid_argument If `datapoint.aggregation_unit` contains
   * a whitespace or \ref FIELD_SEPARATOR.
   *
   * \throw std::logic_error If the \ref OutputInterval is not implemented.
   */
  virtual void write_datapoint(const Datapoint& datapoint);

  /// String to print for values that are not available (NA).
  static const char* NA_VALUE;

  /// Character to separate columns.
  static const char FIELD_SEPARATOR = '\t';

  /// File extension for tabular plaintext files.
  static const char* FILE_EXTENSION;

 private:
  /// Throw an exception if output file already exists.
  static void check_file_exists(const std::string& path);

  /// Write the first line in the output files: column headers
  /**
   * \param datapoint Any output data with the same structure (e.g. list of
   * HFTs) as following output data points will have.
   * \throw std::logic_error If the \ref OutputInterval is not implemented.
   *
   * \throw std::invalid_argument If an HFT name contains whitespaces or the
   * \ref FIELD_SEPARATOR.
   */
  void write_captions(const Datapoint& datapoint);

  bool captions_written = false;
  std::vector<std::ofstream*> file_streams;  // only selected ones
  const OutputInterval interval;
  const TextTableWriterOptions options;

  /** @{ \name File Streams */
  std::ofstream digestibility;
  std::ofstream mass_density_per_hft;
  // Add new output variables here (alphabetical order).
  /** @} */  // File Streams
};
}  // namespace Output
}  // namespace Fauna

#endif  // FAUNA_OUTPUT_TEXT_TABLE_WRITER_H