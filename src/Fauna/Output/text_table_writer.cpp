/**
 * \file
 * \brief Output of tab-separated text tables.
 * \copyright ...
 * \date 2019
 */
#include "text_table_writer.h"
#include "datapoint.h"
#include "fileystem.h"

using namespace Fauna;
using namespace Fauna::Output;

const char* TextTableWriter::FILE_EXTENSION = ".tsv";

TextTableWriter::TextTableWriter(const OutputInterval interval,
                                 const TextTableWriterOptions& options)
    : interval(interval), options(options) {
  const std::string& dir = options.directory;

  create_directories(dir);

  // Add all selected output files to list of file streams.
  if (options.digestibility) {
    const std::string path = dir + "/digestibility" + FILE_EXTENSION;
    check_file_exists(path);
    file_streams.push_back(&digestibility);
    digestibility.open(path);
  }
  if (options.mass_density_per_hft) {
    const std::string path = dir + "/mass_density_per_hft" + FILE_EXTENSION;
    check_file_exists(path);
    file_streams.push_back(&mass_density_per_hft);
    mass_density_per_hft.open(path);
  }
  // -> Add new output files here.

  for (auto& s : file_streams) {
    // Set precision for all output streams.
    s->precision(options.precision);
    // Turn off scientific notation (like 3.14e+03), which might not be
    // understood by post-processing software.
    s->flags(std::ios::fixed);
  }
}

void TextTableWriter::check_file_exists(const std::string& path) {
  if (file_exists(path))
    throw std::runtime_error(
        "Fauna::Output::TextTableWriter "
        "Output file already exists: '" +
        path + "'");
}

void TextTableWriter::write_datapoint(const Datapoint& datapoint) {
  const CombinedData& data = datapoint.data;

  if (!datapoint.interval.matches_output_interval(interval))
    throw std::invalid_argument(
        "Fauna::Output::TextTableWriter::write_datapoint() "
        "Interval of given datapoint does not match user-selected output "
        "interval.");

  if (datapoint.data.datapoint_count == 0)
    throw std::invalid_argument(
        "Fauna::TextTableWriter::write_datapoint() "
        "The datapoint_count of given data is zero.");

  if (datapoint.aggregation_unit.find(' ') != std::string::npos)
    throw std::invalid_argument(
        "Fauna::TextTableWriter::write_datapoint()"
        "Name of aggregation unit '" +
        datapoint.aggregation_unit + "' contains a whitespace.");

  if (datapoint.aggregation_unit.find(FIELD_SEPARATOR) != std::string::npos)
    throw std::invalid_argument(
        "Fauna::TextTableWriter::write_datapoint()"
        "Name of aggregation unit '" +
        datapoint.aggregation_unit + "' contains the field delimiter '" +
        FIELD_SEPARATOR + "'");

  if (!captions_written) {
    write_captions(datapoint);
    captions_written = true;
  }

  // Write common information for all output tables.
  for (auto& f : file_streams) {
    switch (interval) {
      case OutputInterval::Daily:
        *f << datapoint.interval.get_first().get_julian_day() << FIELD_SEPARATOR
           << datapoint.interval.get_first().get_year() << FIELD_SEPARATOR;
        break;
      case OutputInterval::Monthly:
        // We don’t know if this is a 365-days year or a leap year. We
        // calculate the month number assuming it’s not a leap year.
        // Then we take the last day of the month period to get the month
        // number. This is because in a leap year the first day of the month
        // would shift to the last day of the preceding month (for after
        // February). The last day of the month can be shifted forward, but
        // will not leave the month period.
        *f << datapoint.interval.get_last().get_month() << FIELD_SEPARATOR
           << datapoint.interval.get_last().get_year() << FIELD_SEPARATOR;
        break;
      case OutputInterval::Annual:
        *f << datapoint.interval.get_first().get_year() << FIELD_SEPARATOR;
        break;
      case OutputInterval::Decadal:
        *f << datapoint.interval.get_first().get_year() << FIELD_SEPARATOR;
        break;
      default:
        std::logic_error(
            "Fauna::TextTableWriter::write_datapoint() Output time interval is "
            "not implemented.");
    }
    *f << datapoint.aggregation_unit;
  }

  // Per-ForageType Tables
  const auto digestibility_data =
      datapoint.data.habitat_data.available_forage.get_digestibility();
  for (const auto t : FORAGE_TYPES) {
    if (digestibility.is_open())
      digestibility << FIELD_SEPARATOR << digestibility_data[t];
  }
  digestibility << std::endl;

  // Per-HFT Tables
  for (const auto i : datapoint.data.hft_data) {
    if (mass_density_per_hft.is_open())
      mass_density_per_hft << FIELD_SEPARATOR << i.second.massdens;
  }
  mass_density_per_hft << std::endl;
}

void TextTableWriter::write_captions(const Datapoint& datapoint) {
  // Write common column captions for all output tables.
  for (auto& f : file_streams) {
    switch (interval) {
      case OutputInterval::Daily:
        *f << "day" << FIELD_SEPARATOR << "year" << FIELD_SEPARATOR;
        break;
      case OutputInterval::Monthly:
        *f << "month" << FIELD_SEPARATOR << "year" << FIELD_SEPARATOR;
        break;
      case OutputInterval::Annual:
        *f << "year" << FIELD_SEPARATOR;
        break;
      case OutputInterval::Decadal:
        *f << "year" << FIELD_SEPARATOR;
        break;
      default:
        std::logic_error(
            "Fauna::TextTableWriter::write_datapoint() Output time interval is "
            "not implemented.");
    }
    *f << "agg_unit";
  }

  // Per-ForageType Tables
  for (const auto t : FORAGE_TYPES) {
    if (digestibility.is_open())
      digestibility << FIELD_SEPARATOR << get_forage_type_name(t);
  }

  // Per-HFT Tables
  for (const auto i : datapoint.data.hft_data) {
    const std::string& hft_name = i.first->name;

    if (hft_name.find(' ') != std::string::npos)
      throw std::invalid_argument(
          "Fauna::TextTableWriter::write_captions()"
          "HFT name contains a space: '" +
          hft_name + "'");

    if (hft_name.find(FIELD_SEPARATOR) != std::string::npos)
      throw std::invalid_argument(
          "Fauna::TextTableWriter::write_captions()"
          "The HFT name '" +
          hft_name + "' contains the field delimiter '" + FIELD_SEPARATOR +
          "'");

    if (mass_density_per_hft.is_open())
      mass_density_per_hft << FIELD_SEPARATOR << hft_name;
  }

  for (auto& f : file_streams) *f << std::endl;
}
