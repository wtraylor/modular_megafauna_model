///////////////////////////////////////////////////////////////////////////////////////
/// \file 
/// \brief Classes for the spatial units where herbivores live.
/// \ingroup group_herbivory
/// \author Wolfgang Pappa, Senckenberg BiK-F
/// \date May 2017
///////////////////////////////////////////////////////////////////////////////////////
#ifndef HERBIV_HABITAT_H
#define HERBIV_HABITAT_H

#include "herbiv_forageclasses.h" // for ForageMass
#include "herbiv_population.h"    // for HftPopulationsMap
#include <map>                    // for output data mapped to HFTs
#include <memory>                 // for std::auto_ptr
#include <stdexcept>              // for get_populations
#include <vector>                 // for merging data vectors


namespace Fauna{

	// Forward declaration of classes in the same namespace
	class HerbivoreInterface; 
	class Hft;

	/// Output data for one time unit (day, month, …) in a \ref Habitat.
	class HabitatOutputData{
	public:
		/// Constructor
		HabitatOutputData():is_valid(false){}

		/// Whether the object contains valid data.
		/** set this flag to true once all values are filled in */
		bool is_valid;

		/** @{ \name Accumulated values
		 * These values are summed up when merged.*/
		/// Available forage before feeding each day.
		HabitatForage available_forage;
		/// Forage consumed by herbivores [kgDM/km²].
		ForageMass eaten_forage;
		/** @} */ // Accumulated values

		/** @{ \name Averaged values 
		 * For these values, averages are built when merged.*/
		/// Individual herbivore density [ind/km²].
		std::map<const Hft*, double> ind_density;
		/// Body mass herbivore density [kg/km²].
		std::map<const Hft*, double> mass_density;
		/** @} */ // Averaged values

		/// Builds averages and sums for a range of data.
		/**
		 * Any items in the data vector will make the function stop
		 * and return an invalid object whose data is undefined.
		 * \param data Vector of output data.
		 * \param first Index of the first vector entry to include.
		 * \param last  Index of the last vector entry to include. A value of -1 means
		 * the end of the vector.
		 * \warning This function assumes that the other habitat has the same area size!
		 * \see \ref HabitatForage::merge()
		 */
		static HabitatOutputData merge(
				const std::vector<HabitatOutputData> data,
				const int first = 0,
				int last = -1
		);
	};

	/// A vector of \ref HabitatOutputData objects
	typedef std::vector<HabitatOutputData> HabitatOutputVector;

	/// Abstract class of a spatial unit populated by herbivores
	/** 
	 * \note While this base class implements the output functions,
	 * any derived class is responsible to add output data to 
	 * \ref daily_output.
	 */
	class Habitat{
	public: 
		/// Virtual Destructor
		/** Destructor must be virtual in an interface. */
		virtual ~Habitat(){}

		/// Get dry-matter biomass [kg/m²] that is available to herbivores to eat.
		virtual HabitatForage get_available_forage() const = 0;

		/// Get the herbivore populations in the habitat.
		HftPopulationsMap& get_populations(){ return *populations; }
		
		/// Remove forage eaten by herbivores.
		/**
		 * The base class implements only adding the eaten forage
		 * to the output. Any derived class should call this (the parent‘s)
		 * function and do forage removal afterwards.
		 * \param eaten_forage Dry matter leaf forage [kg/m²],
		 * must not exceed available forage.
		 */
		virtual void remove_eaten_forage(const ForageMass& eaten_forage);

		//-----------------------------------------------
		/** @{ \name Output routines. */

		/// Update today’s output data at the start of the day.
		/** Call this once every day from the framework. 
		 * When overwriting this in derived classes, make sure to
		 * call this parent function first.
		 * \param today day of the year (0 ≙ Jan 1st)*/
		virtual void init_todays_output(const int today);

		/// Get output data for each day in the year.
		/** Call this only at the end of the year to have no empty
		 * values or data from last year.
		 * \param day day of the year (0=Jan 1st) */
		HabitatOutputData get_daily_output(const int day) const{
			assert( day >= 0 && day <= daily_output.size() );
			return daily_output[day];
		}

		/// Get output data for the current day.
		/** Call this only after all herbivore activities were performed in order to get
		 * correct data. The current date must have been set by
		 * \ref init_todays_output() before. */
		const HabitatOutputData& read_todays_output();

		/// Get output data as monthly averages for the last year.
		/** Call this only at the end of the year to have no empty
		 * values or data from
		 * last year.
		 * \return Vector of size 12.  
		 * \todo Once moving to C++11, this should be of type 
		 * std::array. */
		std::vector<HabitatOutputData> get_monthly_output() const;

		/// Get output data averaged over the whole year.
		/** Call this only at the end of the year to have no empty values or data from
		 * last year.  */
		HabitatOutputData get_annual_output() const;
		/** @} */ // output routines
		//-----------------------------------------------
	protected:
		/// Constructor
		/**
		 * \param populations object holding the herbivore populations
		 * \see constructor injection: \ref sec_inversion_of_control
		 */
		Habitat(std::auto_ptr<HftPopulationsMap> populations):
			populations(populations),
			daily_output(365){}

		/// The current day as set by \ref read_todays_output().
		/** \see Date::day */
		int get_day()const{return day_of_year;}

		/// Get an editable reference to today’s output object.
		/** Use this within this and derived classes. To get the data
		 * for actual output, call \ref read_todays_output() from
		 * outside.
		 * This function uses the date as set by \ref read_todays_output(). */
		HabitatOutputData& get_todays_output(){
			return  daily_output[day_of_year];
		}
	private:
		int day_of_year;

		/// Vector with output for every day of the year.
		/** \todo When switching to C++11 standard, this should be 
		 * std::array so that the container size is fixed.*/ 
		std::vector<HabitatOutputData> daily_output;

		std::auto_ptr<HftPopulationsMap> populations;
	};
}
#endif //HERBIV_HABITAT_H
