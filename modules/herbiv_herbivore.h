///////////////////////////////////////////////////////////////////////////////////////
/// \file
/// \brief Herbivore interfaces and classes.
/// \ingroup group_herbivory
/// \author Wolfgang Pappa, Senckenberg BiK-F
/// \date July 2017
///////////////////////////////////////////////////////////////////////////////////////
#ifndef HERBIV_HERBIVORE_H
#define HERBIV_HERBIVORE_H

#include <list> // for HerbivoreFactory::create()

#include "herbiv_foraging.h" // for ForageMass

namespace Fauna{

	// Forward declaration of classes in the same namespace
	class Hft;

	/// Interface of a herbivore of a specific \ref Hft.
	/** 
	 * This does not define any model functionality.
	 */
	class HerbivoreInterface{
		public:
			/// Destructor
			virtual ~HerbivoreInterface(){}

			/// Feed the herbivore dry matter forage
			/**
			 * \param forage Proffered dry matter forage to eat
			 * [DMkg/m²].
			 * \throw std::invalid_argument if `forage` exceeds
			 * forage intake constraints
			 */
			virtual void eat(const ForageMass& forage) = 0;

			/// Body mass of one individual [kg/ind]
			virtual double get_bodymass() const = 0;

			/// Get the forage the herbivore would like to eat today.
			/**
			 * \param day current day of year, 0=Jan. 1st
			 * \param available_forage Available forage in the habitat
			 * [kgDM/km²].
			 * \return Dry matter forage *per km²* that the herbivore 
			 * would eat without any food competition.
			 */
			virtual ForageMass get_forage_demands(
					const int day,
					const HabitatForage& available_forage)const=0;

			/// Get herbivore biomass [kg/km²]
			/** If zero, the herbivore is considered dead and will
			 * be removed.
			 * \return Total herbivore biomass [kg/km²] */
			virtual double get_mass_density() const = 0;

			/// Simulate daily events.
			/** 
			 * - Mortality (=reduction of density)
			 * - Metabolism
			 * \param[in] day current day of year, 0=Jan. 1st
			 * \param[out] offspring Mass density [kg/km²] of offspring
			 * produced.
			 */
			virtual void simulate_day(const int day,
					double& offspring) = 0;

			/// Get the herbivore functional type
			virtual const Hft& get_hft()const = 0;
	};

	/// Abstract base class for herbivores.
	class HerbivoreBase: public HerbivoreInterface{
		public:
			/// Constructor
			/**
			 * \param hft Herbivore functional type
			 * \param age age in days
			 */
			HerbivoreBase(const Hft& hft, const int age):
				hft(hft), 
				age(age){}

			virtual void eat(const ForageMass& forage) {}
			virtual double get_bodymass() const {}
			virtual ForageMass get_forage_demands(
					const int day,
					const HabitatForage& available_forage)const{}
			virtual const Hft& get_hft()const{return hft;}
			virtual double get_mass_density() const {}
			virtual void simulate_day(const int day,
					double& offspring) {}
		protected:

			/// Current fat mass [kg].
			double fatmass;

		private: 
			int age;
			const Hft& hft; 
	};

	/// One herbivore individual
	class HerbivoreIndividual: public HerbivoreBase{
		public:
	};


	// TODO
	struct IndividualFactory{
		virtual void create(std::list<HerbivoreIndividual> list,
				const double density, const int age);
	};

	/// A herbivore cohort (age-class)
	/**
	 * Any state variables describe mean values across all 
	 * individuals.
	 * All individuals have the same age.
	 */
	class HerbivoreCohort: public HerbivoreBase{
			/// Number of individuals.
			double individual_count;
	};

	// TODO
	struct CohortFactory{
		virtual void create(std::list<HerbivoreCohort> list,
				const double density, const int age);
	};

}
#endif //HERBIV_HERBIVORE_H
