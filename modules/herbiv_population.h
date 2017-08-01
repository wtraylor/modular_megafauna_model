///////////////////////////////////////////////////////////////////////////////////////
/// \file
/// \brief Management classes of herbivore populations.
/// \ingroup group_herbivory
/// \author Wolfgang Pappa, Senckenberg BiK-F
/// \date July 2017
///////////////////////////////////////////////////////////////////////////////////////

#ifndef HERBIV_POPULATION_H
#define HERBIV_POPULATION_H

#include "herbiv_createherbivores.h" // for CreateHerbivore*
#include <list>                      
#include <vector>

namespace Fauna{
	// forward declarations
	class HerbivoreInterface;
	class Hft;

	/// A list of herbivore interface pointers.
	typedef std::vector<HerbivoreInterface*> HerbivoreVector;
	/// A list of read-only herbivore interface pointers.
	typedef std::vector<const HerbivoreInterface*> ConstHerbivoreVector;

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

		/// Give birth to new herbivores
		/** 
		 * The new herbivores are owned by this population object.
		 * \param ind_per_km2 Offspring amount [ind/km²].
		 * \throw std::invalid_argument if `offspring<0.0`
		 */
		virtual void create_offspring(const double ind_per_km2)=0;

		/// Create a set of new herbivores to establish a population.
		/**
		 * - The new herbivores’ age must match
		 *   \ref Hft::maturity_age_phys_female and
		 *   \ref Hft::maturity_age_phys_male, respectively.
		 * - The sex ratio is even.
		 * - Initial density must match \ref Hft::establishment_density
		 *   as close as possible, but with at least two individuals
		 *   per habitat (if in individual mode and 
		 *   \ref Hft::establishment_density > 0.0).
		 * \throw std::logic_error If this population is not empty.
		 */
		virtual void establish()=0;

		/// The herbivore functional type of this population
		virtual const Hft& get_hft()const=0;

		/// @{ 
		/// Get pointers to the (alive!) herbivores.
		/** 
		 * \warning The pointers are not guaranteed to stay valid
		 * on changing the population in \ref create_offspring() or 
		 * \ref establish().
		 * \return Pointers to all living herbivores in the population.
		 * Guaranteed no NULL pointers.
		 */
		virtual ConstHerbivoreVector get_list()const=0; 
		virtual HerbivoreVector get_list()=0; 
		/** @} */
	};

	/// A population of \ref HerbivoreIndividual objects.
	class IndividualPopulation: public PopulationInterface{
		public: // ------ PopulationInterface -------
			virtual void create_offspring(const double ind_per_km2); 
			virtual void establish();
			virtual const Hft& get_hft()const{
				return create_individual.get_hft();
			}
			virtual ConstHerbivoreVector get_list()const; 
			virtual HerbivoreVector get_list(); 
		public:
			/// Constructor
			/**
			 * \param create_individual Functor for creating new
			 * herbivore individuals.
			 */
			IndividualPopulation(
					const CreateHerbivoreIndividual create_individual);
		private:
			const CreateHerbivoreIndividual create_individual;
			typedef std::list<HerbivoreIndividual> List;
			List list;
	};

	/// A population of \ref HerbivoreCohort objects.
	class CohortPopulation: public PopulationInterface{
		public: // ------ PopulationInterface -------
			virtual void create_offspring(const double ind_per_km2);
			virtual void establish();
			virtual const Hft& get_hft()const{
				return create_cohort.get_hft();
			}
			virtual ConstHerbivoreVector get_list()const; 
			virtual HerbivoreVector get_list(); 
		public:
			/// Constructor
			/**
			 * \param create_cohort Functor for creating new 
			 * \ref HerbivoreCohort instances.
			 * \param dead_herbivore_threshold Minimum individual density
			 * [ind/km²] for a herbivore cohort to be considered alive.
			 * \throw std::invalid_argument if any parameter is wrong.
			 */
			CohortPopulation(
					const CreateHerbivoreCohort create_cohort,
					const double dead_herbivore_threshold);

		private:
			typedef std::list<HerbivoreCohort> List;

			/// Find a cohort in the list.
			/**
			 * \param age_years Age-class number (0=first year of life).
			 * \param sex Male or female cohort?
			 * \return If found: iterator pointing to the \ref
			 * HerbivoreCohort object. If not found: end() iterator of
			 * the cohort list.
			 */
			List::iterator find_cohort(const int age_years, 
					const Sex sex);

			const CreateHerbivoreCohort create_cohort;
			const double dead_herbivore_threshold; // [ind/km²]
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
			void add(std::auto_ptr<PopulationInterface> new_pop);

			/// Get pointers to all (alive!) herbivores of all populations.
			/** 
			 * \see Warning in \ref PopulationInterface::get_list().
			 * \return Pointers to all living herbivores in all 
			 * populations. Guaranteed no NULL pointers.
			 */
			HerbivoreVector get_all_herbivores();

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
			bool empty() const          { return vec.empty(); }
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
