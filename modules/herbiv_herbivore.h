///////////////////////////////////////////////////////////////////////////////////////
/// \file
/// \brief Herbivore interfaces and classes.
/// \ingroup group_herbivory
/// \author Wolfgang Pappa, Senckenberg BiK-F
/// \date July 2017
///////////////////////////////////////////////////////////////////////////////////////
#ifndef HERBIV_HERBIVORE_H
#define HERBIV_HERBIVORE_H

#include <memory>                 // for std::auto_ptr
#include "herbiv_forageclasses.h" // for ForageMass

namespace Fauna{

	// Forward declaration of classes in the same namespace
	class ComposeDietInterface;
	class FatmassEnergyBudget;
	class GetNetEnergyContentInterface;
	class Hft;

	/// Interface of a herbivore of a specific \ref Hft.
	/** 
	 * Derived classes will define the model mechanics.
	 * Each herbivore class has a corresponding implementation of
	 * \ref PopulationInterface which creates and manages the
	 * herbivores.
	 * \see \ref sec_herbiv_new_herbivore_class
	 * \see \ref PopulationInterface
	 * \see \ref sec_herbiv_herbivoredesign
	 */
	struct HerbivoreInterface{
		/// Virtual Destructor
		/** Destructor must be virtual in an interface. */
		virtual ~HerbivoreInterface(){}

		/// Feed the herbivore dry matter forage.
		/**
		 * \param kg_per_km2 Dry matter forage mass [kgDM/km²].
		 * \param digestibility Proportional forage digestibility.
		 * \throw std::invalid_argument If `forage` exceeds
		 * forage intake constraints.
		 * \throw std::logic_error If this herbivore is dead or
		 * has no individuals.
		 */
		virtual void eat(
				const ForageMass& kg_per_km2,
				const Digestibility& digestibility) = 0;

		/// Body mass of one individual [kg/ind].
		virtual double get_bodymass() const = 0;

		/// Get the forage the herbivore would like to eat today.
		/**
		 * Call this after \ref simulate_day().
		 * \param available_forage Available forage in the habitat
		 * [kgDM/km²].
		 * \return Dry matter forage *per m²* that the herbivore 
		 * would eat without any food competition [kgDM/km²].
		 */
		virtual ForageMass get_forage_demands(
				const HabitatForage& available_forage)const=0;

		/// Get the herbivore functional type
		virtual const Hft& get_hft()const = 0;

		/// Individuals per km²
		virtual double get_ind_per_km2()const = 0;

		/// Get herbivore biomass density [kg/km²]
		virtual double get_kg_per_km2() const = 0; 

		/// Simulate daily events.
		/** 
		 * Call this before \ref get_forage_demands().
		 * \param[in] day current day of year, 0=Jan. 1st
		 * \param[out] offspring Number of newborn today [ind/km²]
		 * \throw std::invalid_argument If `day` not in [0,364].
		 */
		virtual void simulate_day(const int day,
				double& offspring) = 0;
	};

	/// The sex of a herbivore
	enum Sex {SEX_FEMALE, SEX_MALE};
	

	/// Abstract base class for herbivores.
	/**
	 * Calculations are generally performed *per* individual.
	 * \see \ref sec_herbiv_herbivoredesign
	 */
	class HerbivoreBase: public HerbivoreInterface{
		public:
			// -------- HerbivoreInterface ----------
			virtual void eat(				
					const ForageMass& kg_per_km2,
					const Digestibility& digestibility);
			virtual double get_bodymass() const; 
			virtual ForageMass get_forage_demands(
					const HabitatForage& available_forage)const;
			virtual const Hft& get_hft()const{
				assert(hft!=NULL);
				return *hft;
			}
			virtual double get_kg_per_km2() const;
			virtual void simulate_day(const int day, double& offspring);
		public:
			/// Current age in days.
			int get_age_days()const{return age_days;} 
			
			/// Current age in years.
			double get_age_years()const{return age_days/365.0;}

			/// Proportional body fat (fat mass/total body mass).
			double get_bodyfat()const;

			/// Body mass at physical maturity [kg/ind].
			double get_bodymass_adult()const;

			/// Current fat mass [kg/ind].
			double get_fatmass()const;

			/// Get fat-free body mass [kg/ind].
			/**
			 * Use \ref Hft::bodymass_male or \ref Hft::bodymass_female
			 * if older than \ref Hft::maturity_age_phys_male or
			 * \ref Hft::maturity_age_phys_female, respectively.
			 * Otherwise interpolate linearly from \ref Hft::bodymass_birth.
			 */
			virtual double get_lean_bodymass() const; 

			/// Physiological maximum of fat mass [kg/ind].
			double get_max_fatmass()const;

			/// The potential (maximum) body mass [kg/ind] with full fat reserves.
			double get_potential_bodymass()const;

			/// Current day of the year, as set in \ref simulate_day().
			int get_today()const{
				assert(today>=0 && today<365);
				return today;
			}

			/// The sex of the herbivore
			Sex get_sex()const{return sex;}
		protected:
			/// Establishment constructor.
			/**
			 * \param age_days age in days
			 * \param body_condition Initial fat reserve as fraction
			 * of physiological maximum [kg/kg].
			 * \param hft Herbivore functional type.
			 * \param sex The sex of the herbivore.
			 * \throw std::invalid_argument If `hft==NULL` or
			 * `age_days <= 0` or `body_condition` not in [0,1].
			 */
			HerbivoreBase( 
					const int age_days,
					const double body_condition,
					const Hft* hft, 
					const Sex sex
					);

			/// Birth constructor.
			/**
			 * Herbivores are born with \ref Hft::bodyfat_birth.
			 * \param hft Herbivore functional type.
			 * \param sex The sex of the herbivore.
			 * \throw std::invalid_argument If `hft==NULL`.
			 */
			HerbivoreBase( const Hft* hft, const Sex sex);

			/// Copy constructor.
			HerbivoreBase(const HerbivoreBase& other);

			/// Copy assignment
			HerbivoreBase& operator=(const HerbivoreBase& other);

			/// Destructor
			// std::auto_ptr cleans up itself, no need to do implement
			// anything in the destructor
			~HerbivoreBase(){} 

			/// Apply a fractional mortality.
			/**
			 * \param mortality Fraction of individual density/count
			 * that is dying.
			 * \throw std::invalid_argument if not `0.0≤mortality≤1.0`
			 */
			virtual void apply_mortality(const double mortality) = 0;

			/// @{ \brief The herbivore’s energy budget object.
			FatmassEnergyBudget& get_energy_budget(){
				assert(energy_budget.get() != NULL);
				return *energy_budget;
			}
			const FatmassEnergyBudget& get_energy_budget()const{
				assert(energy_budget.get() != NULL);
				return *energy_budget;
			}
			/**@}*/

		private: 
			/// Calculate mortality according to \ref Hft::mortality_factors.
			/** Calls \ref apply_mortality(), which is implemented by 
			 * child classes.*/
			void apply_mortality_factors_today();

			/// The \ref ComposeDietInterface selected by \ref Hft::diet_composer.
			/** \return Reference to the function object to define
			 * diet composition 
			 * \throw std::logic_error if \ref Hft::diet_composer not
			 * implemented.
			 */
			ComposeDietInterface& compose_diet()const;

			/// Get the amount of forage the herbivore would be able to forage [kgDM/day/ind]
			/**
			 * Each forage type is calculated independently: is if the
			 * herbivore would only eat *one* type of forage.
			 * All foraging limits (\ref Hft::foraging_limits) are 
			 * considered.
			 * \param available_forage forage in the habitat
			 * \return maximum potentially foraged dry matter of 
			 * each forage type [kgDM/day/ind]
			 * \throw std::logic_error if a \ref ForagingLimit is not
			 * implemented
			 */
			ForageMass get_max_foraging(
					const HabitatForage& available_forage)const;

			/// Returns the function object selected by \ref Hft::net_energy_model.
			/** \throw std::logic_error If the selected model is not
			 * implemented. */
			GetNetEnergyContentInterface& get_net_energy_content()const;

			/// Calculate energy expenditure with given expenditure model.
			/** \return Today’s energy needs [MJ/ind]*/
			double get_todays_expenditure()const;

			/// Get the proportional offspring for today using selected model.
			/**
			 * Reads \ref Hft::reproduction_model and performs 
			 * calculations accordingly.
			 * \return Number of offspring per individual [ind/ind/day].
			 * Zero if this herbivore is male, or has not yet reached
			 * reproductive maturity (\ref Hft::maturity_age_sex).
			 * \throw std::logic_error If \ref Hft::reproduction_model
			 * is not implemented.
			 */
			double get_todays_offspring_proportion()const;

			/// @{ \name State Variables
			int age_days;
			std::auto_ptr<FatmassEnergyBudget> energy_budget;
			int today;
			/** @} */ // state variables

			/// @{ \name Constants
			const Hft* hft;
			Sex sex;
			/** @} */ // constants
	};

	/// One herbivore individual
	/**
	 * \see \ref sec_herbiv_herbivoredesign
	 */
	class HerbivoreIndividual: public HerbivoreBase{
		public:
			// -------- HerbivoreInterface ----------
			virtual double get_ind_per_km2()const{ 
				assert(area_km2 > 0.0);
				return 1.0/area_km2 * !is_dead(); 
			}
		public:
			/// Establishment constructor
			/**
			 * \param age_days Age in days (must be > 0).
			 * \param body_condition Initial fat reserve as fraction
			 * of physiological maximum [kg/kg].
			 * \param hft Herbivore functional type.
			 * \param sex The sex of the herbivore.
			 * \throw std::invalid_argument if any parameter is invalid
			 *
			 * \param area_km2 The absolute area of the habitat [km²].
			 */
			HerbivoreIndividual(
					const int age_days,
					const double body_condition,
					const Hft* hft, 
					const Sex sex,
					const double area_km2
					);

			/// Birth constructor
			/**
			 * \param hft Herbivore functional type.
			 * \param sex The sex of the herbivore.
			 * \throw std::invalid_argument if any parameter is invalid
			 *
			 * \param area_km2 The absolute area of the habitat [km²].
			 */
			HerbivoreIndividual(
					const Hft* hft, 
					const Sex sex,
					const double area_km2
					);

			/// Copy Constructor.
			HerbivoreIndividual(const HerbivoreIndividual& other);

			/// Copy assignment operator.
			HerbivoreIndividual& operator=(const HerbivoreIndividual& other);

			/// Destructor
			~HerbivoreIndividual(){};

			/// Habitat area [km²]
			double get_area_km2()const{return area_km2;}

			/// Whether the individual is dead.
			bool is_dead()const{return dead;}

		protected:
			// -------- HerbivoreBase ---------------
			virtual void apply_mortality(const double mortality);
		private:
			double area_km2; // constant
			bool dead;
	};

	/// A herbivore cohort (age-class)
	/**
	 * Any state variables describe mean values across all 
	 * individuals.
	 * All individuals have the same age.
	 * \see \ref sec_herbiv_herbivoredesign
	 */
	class HerbivoreCohort: public HerbivoreBase{
		public:
			// -------- HerbivoreInterface ----------
			virtual double get_ind_per_km2()const{
				return ind_per_km2;
			} 
		public:
			/// Establishment constructor.
			/**
			 * \param age_days age in days
			 * \param body_condition Initial fat reserve as fraction
			 * of physiological maximum [kg/kg].
			 * \param hft Herbivore functional type.
			 * \param sex The sex of the herbivore.
			 * \throw std::invalid_argument if any parameter is invalid
			 *
			 * \param ind_per_km2 Initial individual density [ind/km²]. 
			 * Can be 0.0, but must not be negative.
			 */
			HerbivoreCohort(
					const int age_days,
					const double body_condition,
					const Hft* hft, 
					const Sex sex,
					const double ind_per_km2
					);

			/// Birth constructor
			/**
			 * \param hft Herbivore functional type.
			 * \param sex The sex of the herbivore.
			 * \throw std::invalid_argument if any parameter is invalid
			 *
			 * \param ind_per_km2 Initial individual density [ind/km²]. 
			 * Can be 0.0, but must not be negative.
			 */
			HerbivoreCohort(
					const Hft* hft, 
					const Sex sex,
					const double ind_per_km2
					);

			/// Copy constructor.
			HerbivoreCohort(const HerbivoreCohort& other);

			/// Copy assignment operator.
			HerbivoreCohort& operator=(const HerbivoreCohort& other);

			/// Destructor.
			~HerbivoreCohort(){};

			/// Check if this and the other cohort are of the same age
			/** 
			 * Two cohorts are considered coeval if they are in the
			 * same year of life:
			 * - First year:  `0<=age_days<365`
			 * - Second year: `365<=age_days<730`
			 * - etc.
			 */
			bool is_same_age(const HerbivoreCohort& other) const{
				return (this->get_age_days()/365 == other.get_age_days()/365);
			}

			/// Merge another cohort into this one.
			/**
			 * All state variables are averaged between the two cohorts
			 * by the weight of population density.
			 * \param other The other cohort that is merged into `this`.
			 * The density of `other` will be reduced.
			 * \throw std::invalid_argument If `fraction` not in [0,1].
			 * \throw std::logic_error If the other cohort is not
			 * compatible: different age, different HFT, or different
			 * sex.
			 */
			void merge(HerbivoreCohort& other);
		protected:
			// -------- HerbivoreBase ---------------
			virtual void apply_mortality(const double mortality);
		private:
			double ind_per_km2; 
	};
}
#endif //HERBIV_HERBIVORE_H
