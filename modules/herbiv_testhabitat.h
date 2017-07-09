///////////////////////////////////////////////////////////////////////////////////////
/// \file    
/// \brief   \ref Fauna::Habitat implementations for testing purpose.
/// \ingroup group_herbivory
/// \author  Wolfgang Pappa, Senckenberg BiK-F
/// \date    June 2017
///////////////////////////////////////////////////////////////////////////////////////

#ifndef HERBIV_TESTHABITATS_H
#define HERBIV_TESTHABITATS_H

#include "herbiv_habitat.h"

namespace Fauna {

	/// Simulation parameters for a \ref TestHabitat
	struct TestHabitatSettings {
		/// Settings for grass growth
		struct Grass {
			/// Proportional daily rate of grass decay (day^-1)
			/** Owen-Smith (2002) gives a value of 0.01 week^-1, that is 1.01^(1/7)-1 = 0.0014
			 * Illius & O'Connor 2000  give a value of 0.03 day^-1*/
			double decay;

			/// Proportional digestibility of the grass [frac].
			double digestibility;

			/// Percentage of habitat covered with grass (Foliar Percentage Cover) [frac]
			double fpc;

			/// Proportional daily grass growth rate
			double growth;

			/// Initial available forage [kgDM/m²]
			/** This should be smaller than \ref saturation */
			double init_mass;

			/// Ungrazable grass biomass reserve, inaccessable to herbivores [kgDM/m²]
			/** Owen-Smith (2002) gives value of 20 g/m²*/
			double reserve;

			/// Saturation grass biomass [kgDM/m²]
			/** Owen-Smith (2002): 200 g/m²*/
			double saturation;

			/// Constructor with arbitrary simple values that are valid
			Grass():decay(0.0), digestibility(0.1), fpc(0.1), growth(0.0),
			init_mass(0.0), reserve(0.1), saturation(1.0){}
		} grass;

		/// Constructor with zero values
		TestHabitatSettings():grass(){}
	};

	/// Helper class for performing simple grass growth to test herbivore functionality
	class TestGrass {
		public:
			/// Constructor
			TestGrass(const TestHabitatSettings::Grass& settings): 
				settings(settings){
					assert( settings.decay         >= 0.0 );
					assert( settings.digestibility >  0.0 );
					assert( settings.digestibility <= 1.0 );
					assert( settings.fpc           >= 0.0 );
					assert( settings.fpc           <= 1.0 );
					assert( settings.growth        >= 0.0 );
					assert( settings.init_mass     >= 0.0 );
					assert( settings.reserve       >  0.0 );
					assert( settings.saturation    >  0.0 );
					assert( settings.init_mass     <= settings.saturation );
					// initialize forage
					forage.set_mass( settings.init_mass );
					forage.set_digestibility( settings.digestibility );
					forage.set_fpc( settings.fpc );
				}

			/// Perform grass growth and decay for one day.
			/** \param day_of_year January 1st = 0 */
			void grow_daily(const int day_of_year);

			/// Get current grass forage
			const GrassForage& get_forage() const{ return forage; } 

			/// Set the grass forage
			void set_forage(const GrassForage& f) { forage = f; }

		protected:
			/// Current forage
			/** Excluding the reserve 
             * \ref TestHabitatSettings::Grass::reserve. */
			GrassForage forage;

			TestHabitatSettings::Grass settings;

	};

	/// A herbivore habitat independent of the LPJ-GUESS framework for testing.
	class TestHabitat: public Habitat{
		public:

			/// Constructor with simulation settings.
			TestHabitat(const TestHabitatSettings settings):
				grass(settings.grass){}

			/// Update output and perform vegetation growth
			/** \see \ref Fauna::Habitat::init_todays_output() */
			virtual void init_todays_output(const int today);

			/// Get dry-matter biomass [kg/m²] that is available to herbivores to eat.
			virtual HabitatForage get_available_forage() const{
				HabitatForage result;
				result.grass = grass.get_forage();
				return result;
			}

			/// Remove forage eaten by herbivores.
			virtual void remove_eaten_forage(const ForageMass& eaten_forage);

		protected:
			/// Perform daily growth.
			/** \param day_of_year January 1st = 0 */
			virtual void grow_daily(const int day_of_year){
				grass.grow_daily(day_of_year);
			}

			/// Grass in the habitat
			TestGrass grass;
	};

	/// A clustor of \ref TestHabitat objects with the same settings.
	/** In the herbivore test simulations this corresponds to a 
	 * \ref Gridcell with \ref Patch objects. */
	class TestHabitatGroup{
		public:

			/// Constructor, creating \ref TestHabitat objects
			/** 
			 * \param lon longitude (just for output labels)
			 * \param lat latitude  (just for output labels)
			 * \param nhabitats Number of habitats to create.
			 * \param settings Parameters passed on to the test habitats.
			 */
			TestHabitatGroup(
					const double lon, const double lat,
					const int nhabitats,
					const TestHabitatSettings settings);

			/// Latitude as defined in the constructor.
			double get_lon()const{return lon;}
			/// Latitude as defined in the constructor.
			double get_lat()const{return lat;}

			/// Get a handle to the test-habitats vector.
			std::vector<TestHabitat>& get_habitats(){return habitats;}

			/// Get list of readonly habitat references.
			std::vector<const Habitat*> get_habitat_references()const{
				std::vector<const Habitat*> result;
				for (int i=0; i<habitats.size(); i++)
					result.push_back(&(habitats[i]));
				return result;
			}

		private:
			std::vector<TestHabitat> habitats;
			double lon,lat;
	};

}

#endif // HERBIV_TESTHABITATS_H
