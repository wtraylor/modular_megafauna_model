///////////////////////////////////////////////////////////////////////////////////////
/// \file 
/// \brief Herbivore Functional Type.
/// \ingroup group_herbivory
/// \author Wolfgang Pappa, Senckenberg BiK-F
/// \date May 2017
///
///////////////////////////////////////////////////////////////////////////////////////
#ifndef HERBIV_HFT_H
#define HERBIV_HFT_H

#include <string>
#include <vector>
#include <stdexcept>

namespace Fauna{

	// forward declarations
	class Parameters;

	/// The different digestion types of the herbivores.
	enum DigestionType{
		/// Hindgut fermenter (caecalid)
		DT_HINDGUT, 
		/// Ruminant
		DT_RUMINANT 
	};

	/// One herbivore functional type (i.e. one species).
	struct Hft{
			/// Constructor initializing values
			/** All values do not necessary need to be valid because
			 * their validity potentially depends on external variables. */
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

			/// Body mass [kg] of an adult male individual.
			int bodymass_male;
			//
			/// Body mass [kg] of an adult female individual.
			int bodymass_female;

			/// Type of digestion (ruminant or hindgut fermenter)
			DigestionType digestion_type;

			/// Habitat population mass density for initial establishment [kg/km²]
			double establishment_density;

			/// Maximum age in years [1–∞).
			int lifespan;

			/// Age of physical and sexual (female) maturity in years.
			int maturity;

			/// Annual mortality rate [0.0–1.0) after first year of life.
			double mortality;

			/// Annual mortality rate [0.0–1.0) in the first year of life.
			double mortality_juvenile;

			/// Maximum annual reproduction rate for females (0.0–∞)
			double reproduction_max;

			// add more parameters in alphabetical order

			/**@}*/ // group simulation parameters

			/** @{ \name Comparison operators*/
			/** Comparison operators are solely based on string 
			 * comparison of the \ref name.
			 */
			bool operator==(const Hft& rhs)const{return name==rhs.name;}
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
			 * \throw std::out_of_range if `!(pos < size())`*/
			const Hft& operator[](const int pos) const{
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
