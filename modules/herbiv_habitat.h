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
#include <cassert>                // for assert()
#include <memory>                 // for std::auto_ptr
#include <stdexcept>              // for get_populations
#include <vector>                 // for merging data vectors


namespace Fauna{

	// Forward declaration of classes in the same namespace
	class HerbivoreInterface; 
	class Hft;

	/// Abstract class of a spatial unit populated by herbivores
	/** 
	 * \note While this base class implements the output functions,
	 * any derived class is responsible to add its own output.
	 */
	class Habitat{
	public: 
		/// Virtual Destructor
		/** Destructor must be virtual in an interface. */
		virtual ~Habitat(){}

		/// Get dry-matter biomass [kgDM/km²] that is available to herbivores to eat.
		virtual HabitatForage get_available_forage() const = 0;

		/// Get the herbivore populations in the habitat.
		HftPopulationsMap& get_populations(){ return *populations; }
		
		/// Update at the start of the day.
		/** 
		 * Call this once every day from the framework. 
		 * When overwriting this in derived classes, make sure to
		 * call this parent function first.
		 * \param today day of the year (0 ≙ Jan 1st)
		 * \throw std::invalid_argument if not `0<=today<=364`
		 */
		virtual void init_day(const int today);

		/// Remove forage eaten by herbivores.
		/**
		 * The base class implements only adding the eaten forage
		 * to the output. Any derived class should call this (the parent‘s)
		 * function and do forage removal afterwards.
		 * \param eaten_forage Dry matter leaf forage [kgDM/km²],
		 * must not exceed available forage.
		 * \throw std::logic_error if `eaten_forage` exceeds
		 * available forage (**to be implemented in derived classes**).
		 */
		virtual void remove_eaten_forage(const ForageMass& eaten_forage);

		//-----------------------------------------------
		/** @{ \name Output routines. */
		/** @} */ // Output routines

		/// The current day as set by \ref init_day().
		/** \see Date::day */
		int get_day()const{return day_of_year;}

	protected:
		/// Constructor
		/**
		 * \param populations object holding the herbivore populations
		 * \see constructor injection: \ref sec_inversion_of_control
		 * \throw std::invalid_argument If `populations` is NULL.
		 */
		Habitat(std::auto_ptr<HftPopulationsMap> populations);
	private:
		int day_of_year; 
		std::auto_ptr<HftPopulationsMap> populations;
	};

	/// A list of \ref Habitat pointers.
	typedef std::list<const Habitat*> HabitatList;
}
#endif //HERBIV_HABITAT_H
