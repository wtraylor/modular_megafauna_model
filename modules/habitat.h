//////////////////////////////////////////////////////////////////////////
/// \file 
/// \brief Classes for the spatial units where herbivores live.
/// \ingroup group_herbivory
/// \author Wolfgang Pappa, Senckenberg BiK-F
/// \date May 2017
//////////////////////////////////////////////////////////////////////////
#ifndef HERBIV_HABITAT_H
#define HERBIV_HABITAT_H

#include "forageclasses.h" // for HabitatForage
#include "outputclasses.h" // for FaunaOut::HabitatOutput & HabitatData
#include <cassert>                // for assert()
#include <list>                   // for HabitatList
#include <memory>                 // for std::auto_ptr

namespace Fauna{
	// Forward declaration of classes in the same namespace
	class HabitatEnvironment;

	/// Abstract class of a homogenous spatial unit populated by herbivores
	/** 
	 * \note While this base class implements the basic output 
	 * functions, any derived class is responsible to add its 
	 * own output.
	 * \see \ref Fauna::SimulationUnit
	 */
	class Habitat{
		public: 
			/// Virtual Destructor
			/** Destructor must be virtual in an interface. */
			virtual ~Habitat(){}

			/// Account for nitrogen cycling back to soil (faeces + carcasses).
			/** \param kgN_per_km2 Nitrogen deposited in habitat [kgN/km²]. 
			 * \throw std::invalid_argument If `kgN_per_km2 < 0.0`.*/
			virtual void add_excreted_nitrogen(const double kgN_per_km2) = 0;

			/// Get dry-matter biomass [kgDM/km²] that is available to herbivores to eat.
			virtual HabitatForage get_available_forage() const = 0;

			/// Get today’s abiotic environmental variables in the habitat.
			virtual HabitatEnvironment get_environment() const = 0;

			/// Update at the start of the day.
			/** 
			 * Call this once every day from the framework. 
			 * When overwriting this in derived classes, make sure to
			 * call this parent function first.
			 * \param today Day of the year (0 ≙ Jan 1st).
			 * \throw std::invalid_argument If not `0<=today<=364`.
			 */
			virtual void init_day(const int today);

			/// Remove forage eaten by herbivores.
			/**
			 * The base class implements only adding the eaten forage
			 * to the output. Any derived class should call this (the parent‘s)
			 * function and do forage removal afterwards.
			 * \param eaten_forage Dry matter leaf forage [kgDM/km²],
			 * must not exceed available forage.
			 * \throw std::logic_error If `eaten_forage` exceeds
			 * available forage (**to be implemented in derived classes**).
			 */
			virtual void remove_eaten_forage(const ForageMass& eaten_forage){
				get_todays_output().eaten_forage += eaten_forage;
			}

			/// The current day as set by \ref init_day().
			/** \see Date::day */
			int get_day()const{return day_of_year;}

			/// The current output data (read-only).
			const FaunaOut::HabitatData& get_todays_output()const{ 
				return current_output; 
			}
		protected:
			/// Class-internal read/write access to current output data.
			FaunaOut::HabitatData& get_todays_output(){ 
				return current_output; 
			}
		private:
			FaunaOut::HabitatData current_output;
			int day_of_year; 
	};

	/// A list of \ref Habitat pointers.
	typedef std::list<const Habitat*> HabitatList;
}
#endif //HERBIV_HABITAT_H
