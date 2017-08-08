//////////////////////////////////////////////////////////////////
/// \file 
/// \brief Herbivore Functional Type.
/// \ingroup group_herbivory
/// \author Wolfgang Pappa, Senckenberg BiK-F
/// \date May 2017
/////////////////////////////////////////////////////////////////
#ifndef HERBIV_HFT_H
#define HERBIV_HFT_H

#include <set> // for mortality_factors
#include <string>
#include <vector>
#include <stdexcept>

namespace Fauna{

	// forward declarations
	class Parameters;

	/// Model to define a herbivore’s diet in a multi-forage scenario.
	enum DietComposer{
		/// Eat exclusively grass, using \ref PureGrazerDiet.
		DC_PURE_GRAZER
	};

	/// Digestion type of a herbivore.
	enum DigestionType{
		/// Hindgut fermenter (caecalid)
		DT_HINDGUT, 
		/// Ruminant
		DT_RUMINANT 
	};

	/// Algorithm to calculate a herbivore’s daily energy needs.
	enum ExpenditureModel{
		/// Formula for cattle, see \ref get_expenditure_taylor_1981.
		EM_TAYLOR_1981
	};

	/// A factor limiting a herbivore’s daily forage intake.
	enum ForagingLimit{
		/// Foraging is limited by digestion, see \ref GetDigestiveLimitIllius1992
		FL_DIGESTION_ILLIUS_1992
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
		/** \see \ref Fauna::GetStarvationMortalityIllius2000 */
		MF_STARVATION_ILLIUS2000,
		/// Starvation death at a minimum bodyfat threshold.
		MF_STARVATION_THRESHOLD
	};

	/// Algorithm to calculate herbivore reproduction time and success.
	enum ReproductionModel{
		/// Use class \ref ReproductionIllius2000 to calculate reproduction.
		RM_ILLIUS_2000
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

			/// \name Simulation Parameters
			/**@{*/ // open doxygen group

			/// Proportional fat mass at birth [kg/kg].
			double bodyfat_birth; 

			/// Maximum proportional fat mass [kg/kg].
			double bodyfat_max;

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

			/// Model defining the herbivore’s diet composition.
			DietComposer diet_composer;

			/// Type of digestion (ruminant or hindgut fermenter)
			DigestionType digestion_type;

			/// Habitat population density for initial establishment [ind/km²]
			double establishment_density;

			/// Energy expenditure model for herbivores.
			ExpenditureModel expenditure_model;

			/// Constraints for maximum daily forage intake.
			std::set<ForagingLimit> foraging_limits;

			/// Maximum age in years [1–∞).
			int lifespan;

			/// @{ Age of physical maturity in years.
			int maturity_age_phys_female;
			int maturity_age_phys_male;
			/// @}

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
