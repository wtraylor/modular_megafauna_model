///////////////////////////////////////////////////////////////////////////////////////
/// \file
/// \brief Management classes of herbivore populations.
/// \ingroup group_herbivory
/// \author Wolfgang Pappa, Senckenberg BiK-F
/// \date July 2017
///////////////////////////////////////////////////////////////////////////////////////

#ifndef HERBIV_POPULATION_H
#define HERBIV_POPULATION_H

#include "herbiv_herbivore.h" // for IndividualFactory and CohortFactory
#include <list>               // for Populatinos::list
#include <vector>             // for HftPopulationsMap::get_all()

namespace Fauna{
	// forward declarations
	class Hft;

	/// A container of herbivore objects.
	/** 
	 * Manages a set of \ref HerbivoreInterface instances, which
	 * have all the same \ref Hft.
	 * It also instantiates all new objects of herbivore classes
	 * (derived from \ref HerbivoreInterface) in a simulation.
	 */
	struct PopulationInterface{
		/// Virtual destructor
		/** Destructor must be virtual in an interface. */
		virtual ~PopulationInterface(){}

		/// Create new herbivores 
		/** 
		 * The new herbivores are owned by this population object.
		 * \param density Total mass density of created herbivores
		 * [kg/km²]
		 * \param age Age of new herbivores in days
		 * \throw std::invalid_argument if `density<=0.0`
		 * \throw std::invalid_argument if `age<0` or `age` is 
		 * greater than \ref Hft::lifespan
		 */
		virtual void create(const double density, const int age=0)=0;

		/// The herbivore functional type of this population
		virtual const Hft& get_hft()const=0;

		/// @{ 
		/// Get pointers to the herbivores.
		/** \warning The pointers are not guaranteed to stay valid
		 * on changing the population in \ref create() or 
		 * \ref remove_dead().
		 */
		virtual std::vector<const HerbivoreInterface*> get_list()const=0; 
		virtual std::vector<HerbivoreInterface*> get_list()=0; 
		/** @} */

		/// Clean the list of any dead herbivore objects
		virtual void remove_dead()=0;
	};

	/// A population of \ref HerbivoreIndividual objects.
	class IndividualPopulation: public PopulationInterface{
		public:
			/// Constructor
			IndividualPopulation(const Hft& hft, const IndividualFactory& factory):
				hft(hft), factory(factory){}

			/// Create new \ref HerbivoreIndividual objects using \ref IndividualFactory.
			virtual void create(const double density, const int age);

			virtual const Hft& get_hft()const{return hft;}

			virtual std::vector<const HerbivoreInterface*> get_list()const; 
			virtual std::vector<HerbivoreInterface*> get_list(); 
			
			virtual void remove_dead();
		private:
			const Hft& hft;
			const IndividualFactory& factory;
			typedef std::list<HerbivoreIndividual> List;
			List list;
	};

	/// A population of \ref HerbivoreCohort objects.
	class CohortPopulation: public PopulationInterface{
		public:
			/// Constructor
			/**
			 * \param hft Herbivore Functional Type
			 * \param factory Creates new objects
			 * \param dead_herbivore_threshold Minimum mass density 
			 * [kg/km²] for a herbivore cohort to be considered alive.
			 */
			CohortPopulation(const Hft& hft, const CohortFactory& factory,
					const double dead_herbivore_threshold):
				hft(hft), factory(factory), 
				dead_herbivore_threshold(dead_herbivore_threshold){}

			/// Create new \ref HerbivoreCohort objects using \ref CohortFactory.
			virtual void create(const double density, const int age);

			virtual const Hft& get_hft()const{return hft;}

			virtual std::vector<const HerbivoreInterface*> get_list()const; 
			virtual std::vector<HerbivoreInterface*> get_list(); 
			
			virtual void remove_dead();
		private:
			const Hft& hft;
			const CohortFactory& factory;
			const double dead_herbivore_threshold;
			typedef std::list<HerbivoreCohort> List;
			List list;
	};
	
	/// Helper class managing object instances of \ref PopulationInterface
	/**
	 * There is one \ref PopulationInterface object per \ref Hft.
	 *
	 * \ref PopulationInterface object instances passed to this class are
	 * owned by this class and will be deleted in the destructor.
	 * Because ownership is unique, copy constructor and copy
	 * assignment operator are deleted (\ref sec_rule_of_three).
	 * In order to pass an instance of \ref HftPopulationsMap 
	 * between functions, std::auto_ptr can be used.
	 */
	class HftPopulationsMap{
		public:
			/// Constructor
			HftPopulationsMap(){}

			/// Destructor, delete all \ref PopulationInterface instances.
			virtual ~HftPopulationsMap();
			
			/// Add a new \ref PopulationInterface object for a HFT
			/**
			 * \param new_pop Pointer to a newly created object. The
			 * object will be owned by this \ref HftPopulationsMap
			 * and deleted in the destructor.
			 * \throw std::logic_error if a population of that HFT already
			 * exists
			 * \throw std::invalid_argument if `new_pop==NULL`
			 */
			void add(PopulationInterface* new_pop);

			/// Access to a population
			/** \param hft The herbivore functional type
			 * \throw std::invalid_argument if `hft` not in the vector */
			PopulationInterface& operator[](const Hft& hft);

			//------------------------------------------------------------
			/** @{ \name Wrapper around std::vector 
			 * Equivalents to methods in Standard Library Container std::vector.*/
			typedef std::vector<PopulationInterface*>::iterator iterator;
			typedef std::vector<PopulationInterface*>::const_iterator const_iterator;
			iterator begin()            { return vec.begin(); }
			const_iterator begin()const { return vec.begin(); }
			iterator end()              { return vec.end();   }
			const_iterator end()const   { return vec.end();   }
			int size() const            { return vec.size();  }
			/** @} */ // Container Functionality
		private:
			std::vector<PopulationInterface*> vec;
			// Deleted copy constructor and copy assignment operator.
			// If they were not deleted, the unique ownership of the
			// PopulationInterface objects could be lost. 
			HftPopulationsMap(const HftPopulationsMap& other);
			HftPopulationsMap& operator=(const HftPopulationsMap& other);
	};
};

#endif // HERBIV_POPULATION_H
