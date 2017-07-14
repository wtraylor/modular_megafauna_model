///////////////////////////////////////////////////////////////////////////////////////
/// \file 
/// \ingroup group_herbivory
/// \brief Basic forage-related classes of the herbivory module.
/// \author Wolfgang Pappa, Senckenberg BiK-F
/// \date May 2017
///////////////////////////////////////////////////////////////////////////////////////
#ifndef HERBIV_FORAGECLASSES_H
#define HERBIV_FORAGECLASSES_H

#include "assert.h"
#include <stdexcept>
#include <string> // for forage type names
#include <vector>

namespace Fauna{

	/// Different types of forage.
	enum ForageType {
		/// Forage type grass.
		FT_GRASS,   
		/// Plants that are not edible for herbivores.
		FT_INEDIBLE 
	};
	/// Number of edible forage types
	const int FORAGE_TYPE_COUNT = 1;

	/// Get a short, lowercase identifier for a forage type.
	std::string get_forage_type_name(const ForageType ft);

	/// Dry matter mass values [DMkg] for different forage types
	class ForageMass {
		public:
			/** @{ */
			/// Dry matter grass forage biomass [DMkg]. 
			/** Number is always positive. */
			double get_grass()const{return grass;}
			/** \throw std::invalid_argument if `g<0.0` */
			void set_grass(const double g){
				if (!(g >= 0.0))
					throw std::invalid_argument("Fauna::ForageMass::set_grass() "
							"Received negative mass value.");
				grass=g;
			}
			/** @} */

			// add other forage types (e.g. browse) here

			/// Constructor with zero values
			ForageMass():grass(0.0){}

			/// Sum up the values.
			double sum() const { 
				return grass; /* add other forage types */
			}

			/** @{ \name Operator overload */
			ForageMass& operator+=(const ForageMass& rhs) {
				this->grass += rhs.grass;/* add other forage types */
				return *this;
			}
			ForageMass& operator=(const ForageMass& rhs) {
				this->grass = rhs.grass;/* add other forage types */
				return *this;
			}
			/** @} */ // Operator overload
		private:
			double grass;
	};

	/// Abstract base class for herbivore forage of one type in a habitat.
	/**
	 * For more forage types create more child classes (like \ref GrassForage).
	 */
	class ForageBase {
		private:
			double digestibility, dry_matter_mass; 
		public:
			/// Constructor with zero values
			ForageBase():digestibility(0.0), dry_matter_mass(0.0){}

			//@{
			/// Fractional digestibility of the biomass for ruminants.
			/** Digestibility as measured *in-vitro* with rumen liquor.  */
			double get_digestibility() const{return digestibility;}
			/** \throw std::invalid_argument if not `0.0<=d<=1.0`*/
			void   set_digestibility(const double d){
				if (d<0.0 || d>1.0)
					throw std::invalid_argument("Fauna::ForageBase::set_digestibility(): "
							"digestibility out of range");
				digestibility = d;
			}
			//@}

			//@{
			/// Dry matter forage biomass over the whole \ref Habitat area [kgDM/m²].
			double get_mass()const{return dry_matter_mass;}
			/** \throw std::invalid_argument if dm<0.0 */
			void   set_mass(const double dm){
				if (dm<0.0)
					throw std::invalid_argument("Fauna::ForageBase::set_mass(): "
							"dry matter is smaller than zero");
				dry_matter_mass = dm;
			}
			//@}
	};

	/// Grass forage in a habitat.
	class GrassForage: public ForageBase {
		private:
			double fpc;
		public:
			/// Constructor with zero values
			GrassForage():ForageBase(), fpc(0.0){}

			/// Dry matter forage in the area covered by grass [kgDM/m²].
			/** Note that this is always greater or equal than \ref dry_matter_mass */
			double get_sward_density()const{
				if (get_fpc() == 0)
					return 0.0;
				const double sd =  get_mass() / get_fpc();
				assert( sd >= 0.0 && sd >= get_mass());
				return sd;
			}

			//@{
			/// Grass-covered area as a fraction of the habitat [fractional].
			double get_fpc()const{return fpc;}
			/** \throw std::invalid_argument if not `0.0<=f<=1.0`*/
			void   set_fpc(const double f){
				if(!( f>=0.0 && f<=1.0))
					throw std::invalid_argument("Fauna::GrassForage::set_fpc() "
							"fpc out of valid range (0.0–1.0)");
				fpc=f;
			}
			//@}
	};

	/// All values for large herbivore forage in a \ref Habitat.
	struct HabitatForage {
		/// Constructor with zero values
		HabitatForage(){}

		/// Constructor with initialization
		HabitatForage(const GrassForage& grass):
			grass(grass){}

		/// The grass forage in the habitat.
		GrassForage grass;

		/// Total forage in the habitat.
		/** Digestibility is weighted average, forage mass is sum.*/
		ForageBase get_total() const;

		// Add other forage types (e.g. browse) here.

		/// Build averages for the forage over a dataset in space or time.
		/** 
		 * The data is implemented as a vector of pointers to avoid
		 * unnecessary copying.
		 * Digestibility is a weighted average scaled by DM mass;
		 * if total mass of the dataset is zero, the unweighted average
		 * is taken.
		 * \note This function assumes that the habitats have the same area! 
		 * \throw std::invalid_argument if `data.empty()`
		 */
		static HabitatForage merge(const std::vector<const HabitatForage*> data);

	};
}
#endif //HERBIV_FORAGECLASSES_H
