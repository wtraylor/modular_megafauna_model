///////////////////////////////////////////////////////////////////////////////////////
/// \file 
/// \ingroup group_herbivory
/// \brief Basic forage-related classes of the herbivory module.
/// \author Wolfgang Pappa, Senckenberg BiK-F
/// \date May 2017
///////////////////////////////////////////////////////////////////////////////////////
#ifndef HERBIV_FORAGECLASSES_H
#define HERBIV_FORAGECLASSES_H

#include <cassert> // for assert()
#include <map>     // for ForageTypeMap
#include <stdexcept>
#include <string> // for forage type names
#include <vector>

namespace Fauna{
	class HerbivoreInterface; // for ForageDistribution

	/// Different types of forage.
	enum ForageType {
		/// Forage type grass.
		FT_GRASS,   
		/// Plants that are not edible for herbivores.
		FT_INEDIBLE 
	};
	/// Number of edible forage types.
	const int FORAGE_TYPE_COUNT = 1;

	/// Get a short, lowercase identifier for a forage type.
	std::string get_forage_type_name(const ForageType ft);

	/// One double value mapped to one \ref ForageType.
	/** Simple wrapper around std::map, but with const subscript
	 * operator. */
	class ForageTypeMap {
		private:
			typedef std::map<ForageType, double>MapType;
		public:
			/// Constructor
			/** Initializes each forage type with a zero value. */
			ForageTypeMap(){
				for (int i=0; i<FORAGE_TYPE_COUNT; i++){
					ForageType ft = (ForageType) i;
					map[ft] = 0.0;
				}
			}

			/// Read-only value access.
			double operator[](const ForageType ft)const{
				const_iterator element = map.find(ft);
				if (element != map.end())
					return element->second;
				else
					return 0.0;
			}

			/** @{ \name Wrapper around std::map 
			 * Equivalents to methods in Standard Library Container std::map.*/
			double& operator[](const ForageType ft){ return map[ft]; }
			typedef MapType::iterator iterator;
			typedef MapType::const_iterator const_iterator;
			iterator begin()            { return map.begin(); }
			const_iterator begin()const { return map.begin(); }
			iterator end()              { return map.end();   }
			const_iterator end()const   { return map.end();   }
			/** @} */ // Wrapper around std::map
		private:
			MapType map;
	};

	/// Dry matter mass values [kgDM or kgDM/m²] for different forage types.
	class ForageMass {
		public:
			/** @{ */
			/// Dry matter grass forage biomass [kgDM or kgDM/m²]. 
			/** Number is always positive. */
			double get_grass()const{return grass;}
			/** \throw std::invalid_argument if `g<0.0` */
			void set_grass(const double g){
				if (!(g >= 0.0))
					throw std::invalid_argument("Fauna::ForageMass::set_grass() "
							"Received negative mass value.");
				grass=g;
			}
			/** \throw std::invalid_argument if `value<0.0` */
			void set(const ForageType forage_type, const double value){
				switch(forage_type){
					case FT_GRASS: grass = value; break;
												 // add other forage types here
					default: break;
				}
			}
			/** @} */

			// add other forage types (e.g. browse) here

			/// Constructor with zero values.
			ForageMass():grass(0.0){}

			/// Sum up the values [kgDM or kgDM/m²].
			double sum() const { 
				return grass; /* add other forage types */
			}

			/** @{ \name Operator overload */
			double operator[](const ForageType ft)const{ // read-only
				switch (ft){
					case FT_GRASS: return grass;
					// add new forage types here
					default: return 0.0;
				}
			}
			ForageMass& operator*=(const double factor){
				this->grass *= factor; /* add other forage types */
				return *this;
			}
			ForageMass operator*(const double factor)const{
				ForageMass result = *this;
				result.grass *= factor;/* add other forage types */
				return result;
			}
			ForageMass& operator/=(const double factor){
				if (factor==0.0)
					throw std::invalid_argument("Fauna::ForageMass::operator/=() "
							"Division by zero.");
				this->grass /= factor; /* add other forage types */
				return *this;
			}
			ForageMass operator/(const double factor)const {
				if (factor==0.0)
					throw std::invalid_argument("Fauna::ForageMass::operator/() "
							"Division by zero.");
				ForageMass result = *this;
				result.grass /= factor;/* add other forage types */
				return result;
			}
			ForageMass& operator+=(const ForageMass& rhs) {
				this->grass += rhs.grass;/* add other forage types */
				return *this;
			}
			ForageMass operator+(const ForageMass& rhs)const{
				ForageMass result = *this;
				result.grass += rhs.grass;/* add other forage types */
				return result;
			}
			ForageMass& operator=(const double rhs) {
				this->grass = rhs;/* add other forage types */
				return *this;
			}
			ForageMass& operator=(const ForageMass& rhs) {
				this->grass = rhs.grass;/* add other forage types */
				return *this;
			}
			bool operator==(const ForageMass& rhs)const{
				return (this->grass == rhs.grass);/* add other forage types */
			}
			bool operator!=(const ForageMass& rhs)const{
				return !operator==(rhs);
			}
			bool operator<(const ForageMass& rhs)const{
				return (this->grass < rhs.grass); // add more types
			}
			bool operator<=(const ForageMass& rhs)const{
				return operator==(rhs) || operator<(rhs);
			}
			/** @} */ // Operator overload
		private:
			double grass;
	};

	/// Map defining which herbivore gets what to eat [kgDM/m²].
	typedef std::map<HerbivoreInterface*, ForageMass> ForageDistribution;

	/// Base class for herbivore forage in a habitat.
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
			/// \brief Dry matter forage biomass over the whole 
			/// area [kgDM/m²].
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

		/// Get dry matter mass [kgDM/m²] for all forage types.
		ForageMass get_mass()const;

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

		/// Read-only access by forage type.
		/** 
		 * \param ft Forage type.
		 * \return Polymorphic reference to forage class object.
		 * If `ft==FT_INEDIBLE` a zero-value object is returned.
		 * \throw std::logic_error if `ft` is not implemented. */
		const ForageBase& operator[](const ForageType ft)const{
			switch (ft){
				case FT_GRASS: return grass;
				case FT_INEDIBLE: 
											 static ForageBase empty;
											 return empty;
				default: throw std::logic_error("Fauna::HabitatForage::operator[]() "
										 "Forage Type is not implemented.");
			}
		}
	};
}
#endif //HERBIV_FORAGECLASSES_H
