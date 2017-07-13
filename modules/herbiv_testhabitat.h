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
			/**
			 * \param populations Herbivore populations; see \ref Habitat::Habitat()
			 * \param settings Simulation settings for the vegetation
			 * model.
			 */
			TestHabitat(std::auto_ptr<HftPopulationsMap> populations,
					const TestHabitatSettings settings):
				Habitat(populations), grass(settings.grass){}

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

	/// A set of \ref Habitat objects.
	/** In the herbivore test simulations this corresponds semantically
	 * to a \ref Gridcell with \ref Patch objects. */
	class TestHabitatGroup{
		public:

			/// Constructor, creating \ref TestHabitat objects
			/** 
			 * \param lon longitude (just for output labels)
			 * \param lat latitude  (just for output labels)
			 */
			TestHabitatGroup(const double lon, const double lat):
		lon(lon), lat(lat){}

			/// Destructor, deleting all \ref Habitat instances.
			~TestHabitatGroup(){
				iterator iter = begin();
				while (iter != end()){
					delete *iter;
					vec.erase(iter);
				}
			}

			/// Latitude as defined in the constructor.
			double get_lon()const{return lon;}
			/// Latitude as defined in the constructor.
			double get_lat()const{return lat;}

			/// Add a newly created \ref Habitat object
			/** The \ref Habitat instance will be owned by the group
			 * and released on its destruction.
			 * \param new_habitat Pointer to a newly created object
			 * \throw std::invalid_argument if `new_habitat==NULL`
			 */
			void add(Habitat* new_habitat){
				if (new_habitat == NULL)
					throw std::invalid_argument("TestHabitatGroup::add(): "
							"received NULL-Pointer as argument.");
				vec.push_back(new_habitat);
			}

			/// Get list of readonly habitat references.
			std::vector<const Habitat*> get_habitat_references()const{
				std::vector<const Habitat*> result;
				for (int i=0; i<vec.size(); i++)
					result.push_back(vec[i]);
				return result;
			}

			/** @{ \name Wrapper around std::vector 
			 * Equivalents to methods in Standard Library Container std::vector.*/
			typedef std::vector<Habitat*>::iterator iterator;
			typedef std::vector<Habitat*>::const_iterator const_iterator;
			iterator begin()            { return vec.begin(); }
			const_iterator begin()const { return vec.begin(); }
			iterator end()              { return vec.end();   }
			const_iterator end()const   { return vec.end();   }
			int size() const            { return vec.size();  }
			void reserve(const int size){ vec.reserve(size);  } 
			/** @} */ // Container functionality 
		private:
			std::vector<Habitat*> vec;
			double lon,lat;

			// Deleted copy constructor and copy assignment operator.
			// If they were not deleted, the unique ownership of the
			// PopulationInterface objects could be lost. 
			TestHabitatGroup(const TestHabitatGroup& other);
			TestHabitatGroup& operator=(const TestHabitatGroup& other);
	};

}

#endif // HERBIV_TESTHABITATS_H
