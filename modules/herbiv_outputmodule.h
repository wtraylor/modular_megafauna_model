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
class Date;
namespace Fauna{
	class Habitat;
	class HftList;
}
namespace FaunaOut{
	class CombinedData;
}

namespace GuessOutput {

	/// Abstract functor strategy class to limit herbivory output to a specific time.
	/**
	 * This base class includes everything.
	 * \see \ref sec_herbiv_limit_output 
	 * \see \ref sec_strategy
	 */
	struct IncludeDate{
		/// Check whether the date shall be included in the output.
		/** 
		 * \param day_of_year Day of year (0=Jan 1st).
		 * \param year Simulation year (0=first year).
		 * \return True if the given year/date shall be included.
		 * \note This is equivalent to the check in \ref outlimit() in 
		 * \ref commonoutput.cpp, but implemented with the strategy
		 * pattern.
		 */
		virtual bool operator()(
				const int year, 
				const int day_of_year) const {return true;}
	};

	/// Limits output to the time after \ref nyear_spinup.
	struct IncludeNoSpinup: public IncludeDate{
		virtual bool operator()(const int year, 
				const int day_of_year) const;
	};

	/// Output module for the herbivory module.
	/** 
	 * If \ref deactivate() is called, all public methods will not
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

			/// Write output of one year for a number of habitats.
			/** 
			 * Depending on \ref interval, for each day, each month,
			 * etc., one row if data is added to the output tables.
			 * \note This function is independent of any global LPJ-GUESS
			 * variables (except for \ref Date::ndaymonth).
			 * \param longitude Value for the longitude column.
			 * \param latitude  Value for the latitude column.
			 * \param day Day of the year (0=Jan 1st).
			 * \param year Simulation year (starting with 0).
			 * \param habitats The group of habitats whose output is
			 * merged to one data point.
			 * \throw std::invalid_argument If not `day` in [0,364] or
			 * `year<0`
			 */
			void outdaily(const double longitude, const double latitude,
					const int day, const int year,
					const std::vector<Fauna::Habitat*>& habitats);

			/// Write output for a \ref Gridcell.
			/** This will write daily, monthly, etc. according to
			 * interval. 
			 * All \ref Fauna::Habitat objects are read into a pointer 
			 * list and past to the other outdaily function.
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

			/// Set the strategy object that limits the output.
			/** \throw std::invalid_argument if `ptr==NULL`*/
			void set_include_date(std::auto_ptr<IncludeDate> ptr){ 
				if (ptr.get()==NULL)
					throw std::invalid_argument(
							"GuessOutput::HerbivoryOutput::set_include_date() "
							"Received NULL as parameter");
				include_date = ptr; 
			}

			/// How to connect different variables in column caption.
			/** For example: “hft1“ and “grass” --> “hft1_grass” */
			static const char CAPTION_SEPARATOR = '_';

		private:
			/// Width of one column in the output table.
			static const int COLUMN_WIDTH = 12;

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
			/// The object limiting output. This is never NULL.
			std::auto_ptr<IncludeDate> include_date;
			bool isactive;
			int precision; 

			/// Write one row into each table.
			void write_datapoint( 
					const double longitude, const double latitude,
					const int day, const int year,
					const FaunaOut::CombinedData& datapoint);
		private: // tables
			/// Selector for a set of columns in a table.
			enum ColumnSelector{
				CS_FORAGE,
				CS_HABITAT,
				CS_HFT,
				CS_HFT_FORAGE
			};
			ColumnDescriptors get_columns(const ColumnSelector);

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
			// more to come, e.g. snow depth
			/** */

			/** @{ \name Forage output tables. */
			TableFile TBL_AVAILABLE_FORAGE;
			TableFile TBL_DIGESTIBILITY;
			TableFile TBL_EATEN_FORAGE;
			/** @} */

			/** @{ \name HFT output tables.*/
			TableFile TBL_BODYFAT;
			TableFile TBL_EXPENDITURE;
			TableFile TBL_INDDENS;
			TableFile TBL_MASSDENS;
			TableFile TBL_STARVATION;
			/** @} */ 

			/** @{ \name HFT–Forage output tables.*/
			TableFile TBL_EATEN_IND;
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