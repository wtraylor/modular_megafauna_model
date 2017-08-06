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

using namespace Fauna;

//forward declarations
namespace Fauna {
	class Parameters;
}

namespace FaunaSim {


	/// Helper class for performing simple grass growth to test herbivore functionality
	/**
	 * \startuml
	 * hide members
	 * hide methods
	 * FaunaSim.SimpleHabitat *--> FaunaSim.LogisticGrass
	 * \enduml
	 */
	class LogisticGrass {
		public:
			/// Settings for grass growth
			struct Parameters {
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
				Parameters():decay(0.0), digestibility(0.1), fpc(0.1), growth(0.0),
				init_mass(0.0), reserve(0.1), saturation(1.0){}

				/// Check if parameters are valid
				/** 
				 * \param[out] msg Possible warnings and error messages.
				 * \return true if values are valid, false if not.
				 */
				bool is_valid(std::string& msg)const;
			};

			/// Constructor
			LogisticGrass(const LogisticGrass::Parameters& settings): 
				settings(settings){
					std::string msg;
					if (!settings.is_valid(msg))
						throw std::invalid_argument("FaunaSim::LogisticGrass::LogisticGrass() "
								"Parameters are not valid: "+msg);
					// initialize forage
					forage.set_mass( settings.init_mass );
					forage.set_digestibility( settings.digestibility );
					forage.set_fpc( settings.fpc );
				}

			/// Perform grass growth and decay for one day.
			/** \param day_of_year January 1st = 0 
			 * \throw std::invalid_argument if not `0<=day_of_year<=364`
			 */
			void grow_daily(const int day_of_year);

			/// Get current grass forage
			const GrassForage& get_forage() const{ return forage; } 

			/// Set the grass forage
			void set_forage(const GrassForage& f) { forage = f; }

		protected:
			/// Current forage
			/** Excluding the reserve 
			 * \ref LogisticGrass::Parameters::reserve. */
			GrassForage forage;

			LogisticGrass::Parameters settings;
	};

	/// A herbivore habitat independent of the LPJ-GUESS framework for testing.
	/**
	 * \startuml
	 * namespace FaunaSim{
	 * hide members
	 * hide methods
	 * SimpleHabitat --|> .Fauna.Habitat
	 * LogisticGrass "1" <--* "1" SimpleHabitat
	 * }
	 * \enduml
	 */
	class SimpleHabitat: public Habitat{
		public:
			/// Simulation parameters for a \ref SimpleHabitat object.
			struct Parameters {
				/// Parameters for logistic grass growth
				LogisticGrass::Parameters grass;
			};

			/// Constructor with simulation settings.
			/**
			 * \param populations Herbivore populations; see \ref Fauna::Habitat::Habitat()
			 * \param settings Simulation settings for the vegetation
			 * model.
			 */
			SimpleHabitat(std::auto_ptr<HftPopulationsMap> populations,
					const SimpleHabitat::Parameters settings):
				Habitat(populations), grass(settings.grass){}

			// ------ Fauna::Habitat implementations ----
			virtual void init_day(const int today); 
			virtual HabitatForage get_available_forage() const{
				HabitatForage result;
				result.grass = grass.get_forage();
				return result;
			}
			virtual void remove_eaten_forage(const ForageMass& eaten_forage);

		protected:
			/// Perform daily growth.
			/** \param day_of_year January 1st = 0 */
			virtual void grow_daily(const int day_of_year){
				grass.grow_daily(day_of_year);
			}
		private:
			/// Grass in the habitat
			LogisticGrass grass;
	};

	/// A set of \ref Fauna::Habitat objects.
	/** In the herbivore test simulations this corresponds semantically
	 * to a \ref ::Gridcell with \ref ::Patch objects. 
	 * \startuml
	 * hide members
	 * hide methods
	 * interface Fauna.Habitat
	 * FaunaSim.SimpleHabitat --|> Fauna.Habitat
	 * Fauna.Habitat "*" <--* FaunaSim.HabitatGroup
	 * \enduml
	 */
	class HabitatGroup{
		public:

			/// Constructor
			/** 
			 * \param lon longitude (just for output labels)
			 * \param lat latitude  (just for output labels)
			 */
			HabitatGroup(const double lon, const double lat):
		lon(lon), lat(lat){}

			/// Destructor, deleting all \ref Fauna::Habitat instances.
			~HabitatGroup(){
				for (iterator itr=begin(); itr!=end(); itr++)
					delete *itr;
				vec.clear();
			}

			/// Latitude as defined in the constructor.
			double get_lon()const{return lon;}
			/// Latitude as defined in the constructor.
			double get_lat()const{return lat;}

			/// Add a newly created \ref Fauna::Habitat object
			/** The \ref Fauna::Habitat instance will be owned by the group
			 * and released on its destruction.
			 * \param new_habitat Pointer to a newly created object
			 * \throw std::invalid_argument if `new_habitat==NULL`
			 */
			void add(std::auto_ptr<Habitat> new_habitat){
				if (new_habitat.get() == NULL)
					throw std::invalid_argument("HabitatGroup::add(): "
							"received NULL-Pointer as argument.");
				Habitat* bare_pointer = new_habitat.get();
				new_habitat.release(); // release ownership of auto_ptr
				vec.push_back(bare_pointer);
			}

			/// Get list of readonly habitat references.
			std::vector<Habitat*> get_habitat_references(){
				std::vector<Habitat*> result;
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
			// Habitat objects could be lost. 
			HabitatGroup(const HabitatGroup&);
			HabitatGroup& operator=(const HabitatGroup&);
	};

	/// A simple vector of pointers \ref HabitatGroup objects.
	/** This is a helper class for \ref FaunaSim::Framework.
	 * Pointers are owned by this list.*/
	class HabitatGroupList{
		public:
			/// Constructor
			HabitatGroupList(){}

			/// Destructor
			~HabitatGroupList(){
				for (iterator itr=begin(); itr!=end(); itr++)
					delete *itr;
				vec.clear();
			}

			/// Add a new element
			/** \param pnew newly constructed object, now owned by the list.
			 * \return reference to `pnew` 
			 * \throw std::logic_error if a habitat group with the same
			 * longitute and latitude already exists.
			 */
			HabitatGroup& add( HabitatGroup* pnew){
				for (const_iterator itr=begin(); itr!=end(); itr++)
					if ((*itr)->get_lon() == pnew->get_lon() &&
							(*itr)->get_lat() == pnew->get_lat())
						throw std::logic_error("FaunaSim::HabitatGroup::add() "
								"A HabitatGroup object with the same longitude and "
								"latitude already exists in the list.");
				vec.push_back(pnew);
				return *pnew;
			}

			/** @{ \name Wrapper around std::vector 
			 * Equivalents to methods in Standard Library Container std::vector.*/
			typedef std::vector<HabitatGroup*>::iterator iterator;
			typedef std::vector<HabitatGroup*>::const_iterator const_iterator;
			iterator begin()            { return vec.begin(); }
			const_iterator begin()const { return vec.begin(); }
			iterator end()              { return vec.end();   }
			const_iterator end()const   { return vec.end();   }
			int size() const            { return vec.size();  }
			void reserve(const int size){ vec.reserve(size);  } 
			/** @} */ // Container functionality 
		private:
			std::vector<HabitatGroup*> vec;
			// deleted copy constructor and copy assignment operator
			HabitatGroupList(const HabitatGroupList&);
			HabitatGroupList& operator=(const HabitatGroupList);
	};
}

#endif // HERBIV_TESTHABITATS_H
