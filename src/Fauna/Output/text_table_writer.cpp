#include "text_table_writer.h"
#include "datapoint.h"
#include "fileystem.h"

using namespace Fauna;
using namespace Fauna::Output;

const char* TextTableWriter::FILE_EXTENSION = ".tsv";

TextTableWriter::TextTableWriter(
    const OutputInterval interval,
    const Parameters::TextTableWriterOptions& options)
    : interval(interval), options(options) {
  const std::string& dir = options.output_directory;

  create_directories(dir);

  // Add all selected output files to list of file streams.
  if (options.mass_density_per_hft) {
    file_streams.push_back(&mass_density_per_hft);
    mass_density_per_hft.open(dir + "/mass_density_per_hft" + FILE_EXTENSION);
  }
  // -> Add new output files here.

  // Set precision for all output streams.
  for (auto& s : file_streams) s->precision(options.precision);
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
        // TODO
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

  // Per HFT Tables
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

  // Per HFT Tables
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
