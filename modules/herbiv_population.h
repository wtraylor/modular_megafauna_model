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
		 * \throw std::invalid_argument If `offspring<0.0`.
		 */
		virtual void create_offspring(const double ind_per_km2)=0;

		/// Create a set of new herbivores to establish a population.
		/**
		 * - The age of new herbivores is evenly distributed in the range
		 *   \ref Hft::establishment_age_range.
		 * - The sex ratio is even.
		 * - Total density matches \ref Hft::establishment_density
		 *   as closely as possible.
		 * \throw std::logic_error If this population is not empty.
		 */
		virtual void establish()=0;

		/// The herbivore functional type of this population
		virtual const Hft& get_hft()const=0;

		/// Get pointers to the (alive!) herbivores.
		/** 
		 * \warning The pointers are not guaranteed to stay valid
		 * on changing the population in \ref create_offspring() or 
		 * \ref establish().
		 * \return Pointers to all living herbivores in the population.
		 * Guaranteed no NULL pointers.
		 */
		virtual ConstHerbivoreVector get_list()const=0; 

		/** \copydoc get_list()const */
		virtual HerbivoreVector get_list()=0; 
	};

	/// A population of \ref HerbivoreIndividual objects.
	class IndividualPopulation: public PopulationInterface{
		public: // ------ PopulationInterface -------
			/** \copydoc PopulationInterface::create_offspring() 
			 * Since we can only creat ‘complete’ (discrete) individuals, but 
			 * the given density `ind_per_km2` is continuous, the remainder
			 * (‘incomplete individual’) for each sex will be remembered until 
			 * next call of `create_offspring()`.
			 */
			virtual void create_offspring(double ind_per_km2); 
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
			/// Create either male or female newborn individuals.
			/** \see \ref create_offspring() */
			void create_offspring_by_sex( const Sex sex, const double ind_per_km2);

			const CreateHerbivoreIndividual create_individual;
			typedef std::list<HerbivoreIndividual> List;
			List list;
			/// ‘Incomplete` newborn herbivore (<1.0).
			/** \see \ref create_offspring() */
			std::map<Sex, double> incomplete_offspring;
	};

	/// A population of \ref HerbivoreCohort objects.
	class CohortPopulation: public PopulationInterface{
		public: // ------ PopulationInterface -------
			/** \copydoc PopulationInterface::create_offspring() 
			 * If the resulting newborn cohort would be below the viable
			 * minimum density (\ref dead_herbivore_threshold), the offspring
			 * is ‘accumulated’ until it reaches (after several calls to 
			 * `create_offspring()`) a sum above the threshold.
			 */
			virtual void create_offspring(const double ind_per_km2);
			/** \copydoc PopulationInterface::establish()
			 * Establish with even sex ratio and *at least* as many 
			 * individuals as given by \ref Hft::establishment_density.
			 */
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

			/// Add newborn animals to the population either males or females.
			/** \see \ref create_offspring() */
			void create_offspring_by_sex(const Sex sex, double ind_per_km2);

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
			/// Minimum viable density for one cohort [ind/km²].
			const double dead_herbivore_threshold; 
			/// Offspring accumulated until above minimum threshold [ind/km²].
			std::map<Sex, double> cumulated_offspring;
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
			HftPopulationsMap(): last_all_herbivores_count(0){}

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

			/// Access to a population by HFT.
			/** 
			 * Uses \ref Hft::operator==() for comparison.
			 * \throw std::invalid_argument if `hft` not in the vector */
			PopulationInterface& operator[](const Hft&);

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

			// see comment in get_all_herbivores()
			int last_all_herbivores_count;

			// Deleted copy constructor and copy assignment operator.
			// If they were not deleted, the unique ownership of the
			// PopulationInterface objects could be lost. 
			HftPopulationsMap(const HftPopulationsMap& other);
			HftPopulationsMap& operator=(const HftPopulationsMap& other);
	};
};

#endif // HERBIV_POPULATION_H
