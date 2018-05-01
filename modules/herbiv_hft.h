//////////////////////////////////////////////////////////////////
/// \file 
/// \brief Herbivore Functional Type.
/// \ingroup group_herbivory
/// \author Wolfgang Pappa, Senckenberg BiK-F
/// \date May 2017
/////////////////////////////////////////////////////////////////
#ifndef HERBIV_HFT_H
#define HERBIV_HFT_H

#include <cmath> // for pow()
#include <set> // for mortality_factors
#include <string>
#include <vector>
#include <stdexcept>
#include <utility> // for std::pair

namespace Fauna{

	// forward declarations
	class Parameters;

	/// Coefficient and exponent for an allometric relationship.
	/**
	 * The allometric relationship:
	 * \f[
	 * x = c * M^e
	 * \f]
	 * - $c$ = \ref coefficient
	 * - $e$ = \ref exponent
	 */
	struct AllometryParameters{
		/// Constructor.
		AllometryParameters(const double coefficient, const double exponent):
			coefficient(coefficient), exponent(exponent){}

		double coefficient;
		double exponent;

		/// Calculate the result of the formula.
		double calc(const double M)const{
			return coefficient * pow(M, exponent);
		}
	};

	/// Selector for a function of how to calculate whole-body conductance.
	/** \see \ref sec_herbiv_thermoregulation */
	enum ConductanceModel{
		/// Allometric formula from data by Bradley & Deavers (1980)\cite bradley1980reexamination
		/** \see \ref get_conductance_bradley_deavers_1980() */
		CM_BRADLEY_DEAVERS_1980,
		/// Allometric formula extrapolating from reindeer fur conductivity.
		/** \see \ref get_conductance_cuyler_oeritsland_2004 */
		CM_CUYLER_OERITSLAND_2004
	};

	/// Model to define a herbivore’s diet in a multi-forage scenario.
	enum DietComposer{
		/// Eat exclusively grass.
		DC_PURE_GRAZER
	};

	/// Digestion type of a herbivore.
	enum DigestionType{
		/// Hindgut fermenter (colonic caecalid).
		DT_HINDGUT, 
		/// Ruminant forgut fermenter.
		DT_RUMINANT 
	};

	/// Algorithm to calculate the daily digestive capacity of a herbivore.
	enum DigestiveLimit{
		/// No digestive limit.
		DL_NONE,

		/// Dry-matter ingestion is limited to a fraction of live herbivore body mass.
		/** \see Hft::digestive_limit_allometry */
		DL_ALLOMETRIC,

		/// Dry-matter ingestion is limited to a fixed fraction of live body mass. 
		/** \see Hft::digestive_limit_fixed */
		DL_FIXED_FRACTION,

		/// Limit digestive limit with \ref GetDigestiveLimitIlliusGordon1992.
		DL_ILLIUS_GORDON_1992
	};

	/// Algorithm to calculate a herbivore’s daily energy needs.
	enum ExpenditureComponent{
		/// Simple exponential expenditure function based on current body mass.
		/**
		 * \f[
		 * E = c * M^e
		 * \f]
		 * - $E$: Daily energy expenditure [MJ/ind/day]
		 * - $c$: Coefficient (\ref AllometryParameters::coefficient in 
		 *   member \ref Hft::expenditure_allometry)
		 * - $M$: Current body mass [kg/ind]
		 * - $e$: Allometric exponent (\ref AllometryParameters::exponent in
		 *   member \ref Hft::expenditure_allometry)
		 */
		EC_ALLOMETRIC,
		/// Formula for field metabolic rate in cattle, see \ref get_expenditure_taylor_1981.
		EC_TAYLOR_1981,
		/// Increased expenditure in lower temperature, see \ref sec_herbiv_thermoregulation.
		EC_THERMOREGULATION
	};

	/// A factor limiting a herbivore’s daily forage harvesting.
	enum ForagingLimit{

		/// Foraging is limited by a functional response towards digestion limit.
		/**
		 * Illius & O’Connor (2000) \cite illius2000resource describe daily 
		 * food intake rate as a Holling Type II functional response 
		 * (compare \ref HalfMaxIntake).
		 * As the maximum daily energy intake they choose the digestive
		 * capacity (compare \ref GetDigestiveLimitIlliusGordon1992).
		 *
		 * Like in the model of Pachzelt et al. (2013) 
		 * \cite pachzelt2013coupling, the grass forage density of the 
		 * whole patch (habitat) is used (not the sward density 
		 *
		 * \note This model for maximum foraging works only for pure grazers
		 * (⇒ \ref DC_PURE_GRAZER).
		 * \ref GrassForage::get_sward_density()).
		 * \see \ref Hft::half_max_intake_density
		 */
		FL_ILLIUS_OCONNOR_2000
	};

	/// How forage net energy content is calculated.
	enum NetEnergyModel{
		/// Use \ref GetNetEnergyContentDefault
		NE_DEFAULT
	};

	/// One way how a herbivore can die.
	enum MortalityFactor{
		/// Independent background mortality for juveniles and adults.
		/** \see \ref GetBackgroundMortality */
		MF_BACKGROUND,
		/// A herbivore dies if its age exceeds \ref Hft::lifespan.
		/** \see \ref Fauna::GetSimpleLifespanMortality */
		MF_LIFESPAN,
		/// Starvation death following Illius & O’Connor (2000).
		/** \see \ref Fauna::GetStarvationIlliusOConnor2000 */
		MF_STARVATION_ILLIUS_OCONNOR_2000,
		/// Starvation death at a minimum bodyfat threshold.
		MF_STARVATION_THRESHOLD
	};

	/// Algorithm to calculate herbivore reproduction time and success.
	enum ReproductionModel{
		/// Use class \ref ReproductionConstMax for reproduction.
		RM_CONST_MAX,
		/// Use class \ref ReprIlliusOconnor2000 to calculate reproduction.
		RM_ILLIUS_OCONNOR_2000,
		/// Use class \ref ReproductionLinear for reproduction.
		RM_LINEAR
	};

	/// One herbivore functional type (i.e. one species).
	struct Hft{
			/// Constructor initializing values
			/** All values do not necessary need to be valid because
			 * their validity potentially depends on external variables. 
			 * For the sake of easy unit-testing, arbitrary default
			 * values are given that lie within the valid ranges.
			 */
			Hft();

			/// Check if all variables are okay
			/** 
			 * \param[in] params The global simulation parameters.
			 * \param[out] msg Warning or error messages for output.
			 * \return true if the object has valid values */
			bool is_valid(const Parameters& params, std::string& msg) const;

			/// Check if all variables are okay
			/** 
			 * \param[in] params The global simulation parameters.
			 * \return true if the object has valid values */
			bool is_valid(const Parameters& params) const{
				std::string dump;
				return is_valid(params, dump);
			}

			/// Whether to include the HFT in the simulation.
			bool is_included;

			/// Unique name of the herbivore type.
			std::string name;

			/** @{ \name Simulation Parameters */

			/// Proportional fat mass at birth [kg/kg].
			double bodyfat_birth; 

			/// Standard deviation in body condition for \ref GetStarvationIlliusOConnor2000.
			/**
			 * Body condition is the proportion of current body fat relative to
			 * physiological maximum.
			 * For herbivore individuals, the standard deviation refers to the 
			 * whole population. In cohort mode, it refers only to one cohort.
			 *
			 * \note For juvevniles (1st year of life), body fat variation is
			 * always zero in order to avoid artificially high death rates if
			 * body fat is low at birth.
			 */
			double bodyfat_deviation;

			/// Maximum proportional fat mass [kg/kg].
			double bodyfat_max;

			/// Maximum rate of fat mass gain in kg fat per kg body mass per day.
			/** A value of zero indicates no limit. */
			double bodyfat_max_daily_gain;

			/// Body mass [kg] at birth for both sexes.
			int bodymass_birth; 

			/// Body mass [kg] of an adult female individual (with full fat reserves).
			int bodymass_female;

			/// Body mass [kg] of an adult male individual (with full fat reserves).
			int bodymass_male;

			/// Length of parturition season in days.
			int breeding_season_length;

			/// First day of parturition season (0=Jan 1st).
			int breeding_season_start;

			/// Algorithm to calculate whole-body conductance for thermoregulation.
			ConductanceModel conductance;

			/// Body core temperature [°C].
			/** Default is 38°C (Hudson & White, 1985\cite hudson1985bioenergetics) */
			double core_temperature;

			/// Minimum individual density [ind/km²] for a living herbivore cohort.
			double dead_herbivore_threshold;

			/// Model defining the herbivore’s diet composition.
			DietComposer diet_composer;

			/// Type of digestion (ruminant or hindgut fermenter)
			DigestionType digestion_type;

			/// Constraints for maximum daily forage intake.
			DigestiveLimit digestive_limit;

			/// Allometric coefficient and exponent for \ref DL_ALLOMETRIC.
			AllometryParameters digestive_limit_allometry;

			/// Constant fraction of body mass for \ref DL_FIXED_FRACTION.
			double digestive_limit_fixed;

			/// Youngest and oldest age [years] for herbivore establishment.
			std::pair<int, int> establishment_age_range;

			/// Total population density for establishment in one habitat [ind/km²]
			double establishment_density;

			/// Parameters for allometric expenditure component: \ref EC_ALLOMETRIC.
			AllometryParameters expenditure_allometry;

			/// Energy expenditure components, summing up to actual expenditure.
			std::set<ExpenditureComponent> expenditure_components;

			/// Constraints for maximum daily forage procurement.
			std::set<ForagingLimit> foraging_limits;

			/// Duration of pregnancy [number of months].
			int gestation_months;

			/// Grass density [gDM/m²] where intake is half of its maximum.
			/** 
			 * Grass (sward) density at which intake rate reaches half 
			 * of its maximum (in a Holling Type II functional response). 
			 * Required by specific foraging limits.
			 */
			double half_max_intake_density;

			/// Maximum age in years [1–∞).
			int lifespan;

			/// Age of physical maturity in years for females.
			int maturity_age_phys_female;

			/// Age of physical maturity in years for males.
			int maturity_age_phys_male;

			/// Age of female sexual maturity in years.
			int maturity_age_sex;

			/// Annual mortality rate [0.0–1.0) after first year of life.
			double mortality;

			/// Ways how herbivores can die.
			std::set<MortalityFactor> mortality_factors;

			/// Annual mortality rate [0.0–1.0) in the first year of life.
			double mortality_juvenile;

			/// Algorithm for forage energy content.
			NetEnergyModel net_energy_model; 

			/// Maximum annual reproduction rate for females (0.0–∞)
			double reproduction_max;

			/// Algorithm to calculate herbivore reproduction.
			ReproductionModel reproduction_model;

			/// Whether to shift mean cohort body condition on starvation mortality.
			/** See \ref GetStarvationIlliusOConnor2000. Default: true */
			bool shift_body_condition_for_starvation;

			// add more parameters in alphabetical order

			/**@}*/ // group simulation parameters

			/** @{ \name Comparison operators*/
			/** Comparison operators are solely based on string 
			 * comparison of the \ref name.
			 */
			bool operator==(const Hft& rhs)const{return name==rhs.name;}
			bool operator!=(const Hft& rhs)const{return name!=rhs.name;}
			bool operator<( const Hft& rhs)const{return name<rhs.name;}
			/** @} */ // Comparison

	};

	/// A set of herbivore functional types, unique by name
	class HftList{
		public:
			/// Get \ref Hft object by its name identifier.
			const Hft& operator[](const std::string& name) const{
				const int pos = find(name);
				if (pos < 0)
					throw std::logic_error("HftList::operator[](): "
							"No Hft object with name \""+name+"\" in list.");
				else
					return operator[](pos);
			}

			/// Get \ref Hft object by its number.
			/** \param pos Position in the vector
			 * \throw std::out_of_range If not 0≤pos≤size()
			 */
			const Hft& operator[](const int pos) const{
				if (pos>=size() || pos < 0)
					throw std::out_of_range("Fauna::HftList::operator[]() "
							"Parameter \"pos\" out of range.");
				return vec.at(pos);
			}

			/// Check whether an \ref Hft of given name exists in the list
			/** \return true if object in list, false if not */
			bool contains(const std::string& name) const{
				return find(name) >= 0;
			}

			/// Add or replace an \ref Hft object.
			/** If an object of the same name already exists,
			 * it will be replaced with the new one.
			 * \throw std::invalid_argument `if (hft.name=="")`
			 */
			void insert(const Hft hft){ 
				if (hft.name == "")
					throw std::invalid_argument("HftList::add(): "
							"Hft.name is empty");

				const int pos = find(hft.name);
				if (pos >= 0)
					vec[pos] = hft; // replace
				else
					vec.push_back(hft); // append new
			}

			/// Remove all elements with `is_included==false`
			void remove_excluded(){
				std::vector<Hft>::iterator iter = vec.begin();
				while (iter != vec.end())
					if (!iter->is_included)
						iter = vec.erase(iter);
					else
						iter++;
			}

			
			//------------------------------------------------------------
			/** @{ \name Wrapper around std::vector */
			typedef std::vector<Hft>::iterator iterator;
			typedef std::vector<Hft>::const_iterator const_iterator;
			iterator begin(){return vec.begin();}
			const_iterator begin()const{return vec.begin();}
			iterator end(){return vec.end();}
			const_iterator end()const{return vec.end();}
			const int size() const{ return vec.size(); }
			/** @} */ // Container Functionality
		private:
			/// Vector holding the Hft instances.
			std::vector< Hft > vec;
			
			/// Find the position of the \ref Hft with given name.
			/** \param name \ref Hft::name identifier
			 * \return list position if found, -1 if not in list */
			int find(const std::string& name)const{
				for (int i=0; i<size(); i++)
					if (vec.at(i).name == name) 
						return i;
				return -1;
			}
	};
	
}
#endif //HERBIV_HFT_H
