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


	/// The different digestion types of the herbivores.
	/** \ingroup group_herbivory */
	enum DigestionType{
		/// Hindgut fermenter (caecalid)
		DT_HINDGUT, 
		/// Ruminant
		DT_RUMINANT 
	};

	/// One herbivore functional type (i.e. one species).
	/**
	 * \ingroup group_herbivory
	 */
	struct Hft{
			/// Constructor initializing values
			/** All values do not necessary need to be valid because
			 * their validity potentially depends on external variables. */
			Hft();

			/// Check if all variables are okay
			/** \param msg Warning or error messages for output.
			 * \return true if the object has valid values */
			bool is_valid(std::string& msg) const;

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
	};

	/// The set of herbivore functional types (singleton)
	/** \see \ref sec_singleton
	 */
	class HftList{
		public:
			/// Get the global singleton instance.
			static HftList& get_global(){
				static HftList instance = HftList(); // instantiated on first call
				return instance;
			}

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

			/// Number of elements in the list
			const int size() const{ return vec.size(); }
			
			/// Add or replace an \ref Hft object.
			/** If an object of the same name already exists,
			 * it will be replaced with the new one.
			 * \throw std::invalid_argument `if (hft.name=="")`
			 * \throw std::logic_error if the list is closed*/
			void insert(const Hft hft){ 
				if (is_closed)
					throw std::logic_error("HftList::add(): "
							"List is closed to changes.");
				if (hft.name == "")
					throw std::invalid_argument("HftList::add(): "
							"Hft.name is empty");

				const int pos = find(hft.name);
				if (pos >= 0)
					vec[pos] = hft; // replace
				else
					vec.push_back(hft); // append new
			}

			/// Check whether an \ref Hft of given name exists in the list
			/** \return true if object in list, false if not */
			bool contains(const std::string& name) const{
				return find(name) >= 0;
			}

			/// Remove all elements with `is_included==false`
			/** \throw std::logic_error if list is closed */
			void remove_excluded(){
				if (is_closed)
					throw std::logic_error("HftList::remove_excluded(): "
							"List is closed.");
				std::vector<Hft>::iterator iter = vec.begin();
				while (iter != vec.end())
					if (!iter->is_included)
						iter = vec.erase(iter);
					else
						iter++;
			}

			/// Close the list for any future change attempts.
			void close(){ is_closed=true; }
		private:
			/// Vector holding the Hft instances.
			std::vector< Hft > vec;
			
			bool is_closed;

			/// Find the position of the \ref Hft with given name.
			/** \param name \ref Hft::name identifier
			 * \return list position if found, -1 if not in list */
			int find(const std::string& name)const{
				for (int i=0; i<size(); i++)
					if (vec.at(i).name == name) 
						return i;
				return -1;
			}

			HftList():is_closed(false){}     // hidden constructor
			HftList(HftList const&);        // deleted copy constructor
			void operator=(HftList const&); // deleted assignment constructor
	};
	
}
#endif //HERBIV_HFT_H
