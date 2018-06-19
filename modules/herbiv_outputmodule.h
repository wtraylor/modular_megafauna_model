///////////////////////////////////////////////////////////////////////////////////////
/// \file 
/// \brief Output module for herbivory.
/// \ingroup group_herbivory
/// \author Wolfgang Pappa, Senckenberg BiK-F
/// \date May 2017
///////////////////////////////////////////////////////////////////////////////////////

#ifndef HERBIV_OUTPUTMODULE_H
#define HERBIV_OUTPUTMODULE_H

#include "outputmodule.h"
#include "gutil.h"           // for xtring
#include <memory>            // for std::auto_ptr
#include <stdexcept>         // for std::invalid_argument

// forward declarations of referenced classes
namespace Fauna{
	class HftList;
	class SimulationUnit;
}
namespace FaunaOut{
	class CombinedData;
}

namespace GuessOutput {

	/// Output module for the herbivory module.
	/** 
	 * For an overview on all output files and variables, please look at
	 * `data/ins/herbiv_output.ins`.
	 * \note If \ref deactivate() is called, all public methods will not
	 * do anything anymore.
	 * \see \ref sec_herbiv_output (in particular \ref sec_herbiv_outputmodule)
	 * \see \ref sec_herbiv_new_output
	 * \see \ref sec_herbiv_limit_output
	 */
	class HerbivoryOutput : public OutputModule {
		public:

			/// Constructor, declaring parameters, setting global instance
			/**
			 * There is only one single instance of this class in the
			 * program.
			 * \throw std::logic_error If the constructor is called 
			 * a second time.
			 */
			HerbivoryOutput();

			/// Disable any activity all together.
			void deactivate(){ isactive = false; }

			/// Returns the one global instance of this class
			/**
			 * \throw std::logic_error If no global instance has been
			 * created yet (because it gets created by 
			 * \ref OutputModuleRegistry).
			 */
			static HerbivoryOutput& get_instance(); 

			/// Parse string parameters.
			/** \note Tables are created on first output call. */
			virtual void init();

			/// Inherited function that is not used.
			virtual void outannual(Gridcell& gridcell){}

			/// Write output of one year for a number of habitats and 
			/// their herbivores.
			/** 
			 * Depending on \ref interval, for each day, each month,
			 * etc., one row if data is added to the output tables.
			 *
			 * The date (day & year) that is printed out is set back to the 
			 * center of the time period that is being averaged. This way, 
			 * plotting the data will be visually accurate.
			 *
			 * \note This function is independent of any global LPJ-GUESS
			 * variables (except for \ref Date::ndaymonth).
			 * \param longitude Value for the longitude column.
			 * \param latitude  Value for the latitude column.
			 * \param day Day of the year (0=Jan 1st).
			 * \param simulation_year Simulation year (starting with 0).
			 * \param calendar_year Year to print out 
			 * (compare \ref Date::get_calendar_year()).
			 * \param simulation_units The group of habitats and
			 * herbivores whose output is merged to one data point.
			 * \throw std::invalid_argument If not `day` in [0,364] or
			 * `simulation_year<0`
			 */
			void outdaily(const double longitude, const double latitude,
					int day, const int simulation_year, int calendar_year,
					const std::vector<Fauna::SimulationUnit*>& simulation_units);

			/// Write output for a \ref Gridcell.
			/** This will write daily, monthly, etc. according to
			 * interval. 
			 * All \ref Fauna::SimulationUnit objects are read into a 
			 * pointer list and past to the other outdaily function.
			 */
			virtual void outdaily(Gridcell& gridcell);

			/// Set the list of HFTs for creating tables.
			/**
			 * The given \ref Fauna::HftList object is **copied** in order
			 * to make sure that it stays the same. If a pointer to an
			 * external HftList would be stored, that list could 
			 * potentially change after the tables were defined, and that
			 * would mess up the output.
			 * \throw std::logic_error If the HFT list has already been
			 * set.
			 */
			void set_hftlist(const Fauna::HftList&);

			/// How to connect different variables in column caption.
			/** For example: “hft1“ and “grass” --> “hft1_grass” */
			static const char CAPTION_SEPARATOR = '_';

		private:
			/// Value to insert for missing data.
			static const double NA_VALUE;

			/// Temporal aggregation interval (monthly, yearly, ...)
			enum Interval{
				/// Daily output
				DAILY,
				/// Monthly output
				MONTHLY,
				/// Output every year
				ANNUAL,
				/// Output every 10 years 
				DECADAL
			};
			Interval interval; 
			/// Interval parameter string as read from instruction file.
			xtring interval_xtring; 

			static HerbivoryOutput* global_instance; 
			std::auto_ptr<Fauna::HftList> hftlist;
			bool isactive;
			int precision; 

			/// Write one row into each table.
			void write_datapoint( 
					const double longitude, const double latitude,
					const int day, const int year,
					const FaunaOut::CombinedData& datapoint);

		private: // helper functions
			/// Selector for a set of columns in a table.
			enum ColumnSelector{
				CS_FORAGE,
				CS_HABITAT,
				CS_HFT,
				CS_HFT_FORAGE
			};
			ColumnDescriptors get_columns(const ColumnSelector);

			/// Set day and year back into the center of the interval that is averaged.
			/**
			 * \warning This function does not claim to be wholly accurate. 
			 * Month length is approximated as 30 days and year length is 365
			 * days.
			 * \param[in,out] day Day of the year (0: January 1st)
			 * \param[in,out] year Simulation or calendar year.
			 * \see \ref interval
			 */
			void set_date_to_period_center(int& day, int& year)const;

			/// Whether this day is included in the output.
			/**
			 * \return True for years after spinup.
			 * \see \ref date, \ref nyear_spinup
			 * \note This is functionally the counterpart of `outlimit()` in
			 * \ref commonoutput.cpp. The difference is that in this 
			 * implementation, all preparation of output data is skipped in the
			 * first place if the day is not to be included.
			 */
			bool is_today_included()const;

		private: // tables
			/// File and table descriptor for one output variable.
			struct TableFile{
				TableFile(
						const char* paramname, 
						const std::string& description,
						const std::string& unit,
						const ColumnSelector column_selector):
					column_selector(column_selector),
					paramname(paramname),
					description(description),
					unit(unit){}

				ColumnSelector column_selector;
				std::string description; // help text
				std::string filename; // read as parameter
				Table table;
				std::string unit; // help text
				const char* paramname; // for instruction file parameter
				// NOTE: declare_parameter() in parameters.h takes
				// a const char* as parameter name. To use std::string
				// here and then convert it to char* with std::string::c_str()
				// caused plib.h to not recognize the parameter. Therefore,
				// the parameter name needs to be defined directly as
				// const char*.
			};
			/// List of all \ref TableFile objects.
			const std::vector<TableFile*> init_tablefiles();
			const std::vector<TableFile*> TABLEFILES;

			/** @{ \name Habitat output tables. */
			TableFile TBL_HABITAT;
			/** */

			/** @{ \name Forage output tables. */
			TableFile TBL_AVAILABLE_FORAGE;
			TableFile TBL_DIGESTIBILITY;
			TableFile TBL_EATEN_FORAGE;
			/** @} */

			/** @{ \name HFT output tables.*/
			TableFile TBL_BODYFAT;
			TableFile TBL_BOUND_NITROGEN;
			TableFile TBL_EXPENDITURE;
			TableFile TBL_INDDENS;
			TableFile TBL_MASSDENS;
			TableFile TBL_MORT_BACKGROUND;
			TableFile TBL_MORT_LIFESPAN;
			TableFile TBL_MORT_STARVATION;
			TableFile TBL_OFFSPRING;
			/** @} */ 

			/** @{ \name HFT–Forage output tables.*/
			TableFile TBL_EATEN_PER_IND;
			TableFile TBL_EATEN_PER_MASS;
			TableFile TBL_ENERGY_CONTENT;
			TableFile TBL_ENERGY_INTAKE_PER_IND;
			TableFile TBL_ENERGY_INTAKE_PER_MASS;
			/** @} */ 
	};

	/// Helper function to see if a day is the first of a month.
	/**
	 * \param day Day of year (0=Jan 1st).
	 * \return True if `day` (0–364) is first day of a month.
	 * \throw std::invalid_argument If `day` not in [0,364].
	 */
	bool is_first_day_of_month(int day);
}

#endif // HERBIV_OUTPUTMODULE_H
