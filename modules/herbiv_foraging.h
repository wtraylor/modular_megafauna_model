///////////////////////////////////////////////////////////////////////////////////////
/// \file herbiv_foraging.h
/// \ingroup group_herbivory
/// \brief Large herbivore forage.
/// \author Wolfgang Pappa, Senckenberg BiK-F
/// \date May 2017
///////////////////////////////////////////////////////////////////////////////////////
#ifndef HERBIV_FORAGING_H
#define HERBIV_FORAGING_H

#include <assert.h>
#include <vector>
#include <string> // for forage type names

namespace Fauna{

	/// Different types of forage.
	/** \ingroup group_herbivory **/
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
	/** This struct is so primitive that it does not contain getters and setters.
	 * \ingroup group_herbivory **/
	struct ForageMass {
		/// Dry matter grass forage biomass [DMkg].
		double grass;

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
	};

	/// Abstract base class for herbivore forage of one type in a habitat.
	/**
	 * For more forage types create more child classes (like \ref GrassForage).
	 * \ingroup group_herbivory
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
			void   set_digestibility(const double d){
				assert( d <= 1.0 );
				assert( d >= 0.0 );
				digestibility = d;
			}
			//@}

			//@{
			/// Dry matter forage biomass over the whole \ref Habitat area [kgDM/m²].
			double get_mass()const{return dry_matter_mass;}
			void   set_mass(const double dm){
				assert( dm >= 0.0);
				dry_matter_mass = dm;
			}
			//@}
	};

	/// Grass forage in a habitat.
	/** \ingroup group_herbivory **/
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
			void   set_fpc(const double f){
				assert( f>=0.0 && f<=1.0);
				fpc=f;
			}
			//@}
	};

	/// All values for large herbivore forage in a \ref Habitat.
	/** 
	 * \ingroup group_herbivory
	 */
	struct HabitatForage {
		/// Constructor with zero values
		HabitatForage(){}

		/// Constructor with initialization
		HabitatForage(const GrassForage& grass):
			grass(grass){}

		/// The grass forage in the habitat.
		GrassForage grass;

		/// Get the forage of one type.
		/** \todo Does anybody need this function? */
		ForageBase& get_by_type(const ForageType ft){
			assert(ft != FT_INEDIBLE);
			return grass; // for new forage types create if or switch statement
		}

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
		 * \note This function assumes that the habitats have the same area! */
		static HabitatForage merge(const std::vector<const HabitatForage*> data);

	};
}
#endif //HERBIV_FORAGING_H
