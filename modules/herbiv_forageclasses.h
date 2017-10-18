///////////////////////////////////////////////////////////////////////////////////////
/// \file 
/// \ingroup group_herbivory
/// \brief Basic forage-related classes of the herbivory module.
/// \author Wolfgang Pappa, Senckenberg BiK-F
/// \date May 2017
///////////////////////////////////////////////////////////////////////////////////////
#ifndef HERBIV_FORAGECLASSES_H
#define HERBIV_FORAGECLASSES_H

#include <cassert>   // for assert()
#include <cmath>     // for NAN and INFINITY
#include <map>       // for ForageValues
#include <set>       // for FORAGE_TYPES
#include <stdexcept>
#include <string>    // for forage type names

namespace Fauna{
	class HerbivoreInterface; // for ForageDistribution

	/// Different types of forage.
	enum ForageType {
		/// Forage type grass.
		FT_GRASS,   
		/// Plants that are not edible for herbivores.
		FT_INEDIBLE 
	};

	/// Build weighted average of two numbers.
	/**
	 * \throw std::invalid_argument If one weight is smaller than
	 * zero.
	 * \throw std::invalid_argument If the sum of weights is zero.
	 * \throw std::invalid_argument If one weight is NAN or INFINITY.
	 * \note `NAN` is checked by `weight_a != weight_a`, but this
	 * might not work if compiled with `g++` with the option
	 * `-fastmath`.
	 */
	inline double average(const double a, const double b,
			const double weight_a=1.0, const double weight_b=1.0){
		if (weight_a < 0.0 || weight_b < 0.0)
			throw std::invalid_argument("Fauna::average() "
					"Weight must be >=0.0");
		if (weight_a != weight_a || weight_b != weight_b)
			throw std::invalid_argument("Fauna::average() "
					"Weight is NAN");
		if (weight_a == INFINITY || weight_b == INFINITY)
			throw std::invalid_argument("Fauna::average() "
					"Weight is INFINITY");
		if (weight_a + weight_b == 0.0)
			throw std::invalid_argument("Fauna::average() "
					"Sum of weights is zero.");
		return (a*weight_a + b*weight_b) / (weight_a + weight_b);
	}

	/// Set with all enum entries of \ref ForageType.
	/**
	 * This set serves mainly the purpose to iterate easily over
	 * all forage types:
	 * \code
	 * for (std::set<ForageType>::const_iterator ft = FORAGE_TYPES.begin();
	 *      ft != FORAGE_TYPES.end(); ft++)
	 * {
	 *   const ForageType forage_type = *ft;
	 *   \/\/Do your calculations
	 * }
	 * \endcode
	 */
	extern const std::set<ForageType> FORAGE_TYPES;

	/// Get a short, lowercase string identifier for a forage type.
	/** The names are 
	 * - unique,
	 * - lowercase, 
	 * - without blank spaces or tabs, newlines etc.,
	 * - without the output column header separation character 
	 *   \ref GuessOutput::HerbivoryOutput::CAPTION_SEPARATOR.
	 */
	const std::string& get_forage_type_name(const ForageType);

	/// Describes which values are allowed in \ref Fauna::ForageValues.
	enum ForageValueTag{
		/// Only values in interval [0,∞) are allowed.
		POSITIVE_AND_ZERO,
		/// Only values in interval [0,1] are allowed.
		ZERO_TO_ONE
	};

	/// Multi-purpose template class for double values mapped by edible(!) forage type.
	/**
	 * The forage type \ref FT_INEDIBLE is excluded from all operations.
	 * \tparam tag Defines the allowed data range.
	 */
	template<ForageValueTag tag> class ForageValues{
		private:
			typedef std::map<ForageType, double>MapType;

			/** @{ \name Write access iteration */
			typedef MapType::iterator iterator; // write access
			iterator begin()            { return map.begin(); }
			iterator end()              { return map.end();   }
			/** @} */

			/// Initialize: create map entries for every forage type.
			/** \see \ref FORAGE_TYPES */
			void init(){
				assert( map.empty() );
				for (std::set<ForageType>::const_iterator ft=FORAGE_TYPES.begin();
						ft != FORAGE_TYPES.end(); ft++) 
					map[*ft] = 0.0;
				assert( map.size() == FORAGE_TYPES.size() );
			}
		public:
			/// Constructor with initializing value.
			/**
			 * \throw std::invalid_argument If `init_value` is not allowed
			 * by given `tag`.
			 * \throw std::logic_error If `tag` is not implemented.
			 *
			 */
			ForageValues(const double init_value = 0.0){
				init();
				for (iterator i=begin(); i!=end(); i++)
					set(i->first, init_value);
			}

			/// Divide safely also by zero values.
			/**
			 * \param divisor Numbers to divide by; can contain zeros.
			 * \param na_value Value that shall be set as a result for
			 * division by zero.
			 * \return New object with the division result.
			 * \see operator/()
			 */
			ForageValues<tag> divide_safely(
					const ForageValues<tag>& divisor, 
					const double na_value)const{
				ForageValues<tag> result(*this);
				for (const_iterator i=begin(); i!=end(); i++) {
					const double d = divisor.get(i->first);
					if (d != 0.0) 
						result.set(i->first, i->second / d); // normal
					else
						result.set(i->first, na_value); // division by zero
				}
				return result;
			}

			/// Get a value (read-only).
			/** 
			 * \throw std::invalid_argument If \ref FT_INEDIBLE is passed.
			 * \throw std::logic_error If forage type not accessible.
			 * This error should never occur, as all forage types are
			 * initialized in the constructors. 
			 */
			double get(const ForageType ft)const{
				if (ft == FT_INEDIBLE)
					throw std::invalid_argument("Fauna::ForageValues<>::get() "
							"The forage type `FT_INEDIBLE` is not allowed.");
				const_iterator element = map.find(ft);
				if (element != map.end())
					return element->second;
				else
					throw std::logic_error("Fauna::ForageValues<>::get() "
							"Forage type \""+
							get_forage_type_name(ft)
							+"\" not implemented or invalid.");
			}

			/// Merge this object with another one by building (weighted) means.
			/**
			 * \param other Other object to merge into this one.
			 * \param this_weight Weight of this object’s values.
			 * \param other_weight Weight of the other object’s values.
			 * \return This object. 
			 * \throw std::invalid_argument The same as \ref average().
			 * \see \ref Fauna::average().
			 */
			ForageValues<tag>& merge(const ForageValues<tag>& other,
					const double this_weight=1.0, const double other_weight=1.0){
				for (const_iterator i=other.begin(); i!=other.end(); i++)
					set(i->first, 
							average((*this)[i->first], i->second,
								this_weight, other_weight));
			}

			/// For each forage type, take the minimum value.
			/** \param other The object to compare this object with. 
			 * \return This object. */
			ForageValues<tag>& min(const ForageValues<tag>& other){
				if (&other == this) return *this;
				for (const_iterator i=other.begin(); i!=other.end(); i++)
					set(i->first, fmin(get(i->first), i->second));
				return *this;
			}

			/// Read-only value access.
			/** \throw std::logic_error If forage type not accessible.
			 * This error should never occur, as all forage types are
			 * initialized in the constructors. */
			double operator[](const ForageType ft)const{ return get(ft); }

			/// Set a value, only finite values are allowed..
			/**
			 * \throw std::invalid_argument If `value` is not allowed
			 * by given `tag`, is NAN or is INFINITY.
			 * \throw std::invalid_argument If `forage_type==FT_INEDIBLE`.
			 * \throw std::logic_error If `tag` is not implemented.
			 */
			void set(const ForageType forage_type, const double value){
				switch (tag){
					case POSITIVE_AND_ZERO: 
						if (value<0.0)
							throw std::invalid_argument((std::string) 
									"ForageValues<POSITIVE_AND_ZERO> "
									"Value < 0 not allowed."+
									" ("+get_forage_type_name(forage_type)+")");
						break;
					case ZERO_TO_ONE: 
						if (value<0.0 || value>1.0)
							throw std::invalid_argument((std::string)
									"ForageValues<ZERO_TO_ONE> "
									"Value is not in interval [0,1]."+
									" ("+get_forage_type_name(forage_type)+")");
						break;
					default:
						throw std::logic_error("ForageValues<> "
								"ForageValueType not implemented.");
				}
				if (std::isnan(value))
					throw std::invalid_argument((std::string)
							"ForageValues<> "
							"NAN is not allowed as a value."+
							" ("+get_forage_type_name(forage_type)+")");
				if (std::isinf(value))
					throw std::invalid_argument((std::string)"ForageValues<> "
							"INFINITY is not allowed as a value."+
							" ("+get_forage_type_name(forage_type)+")");
				if (forage_type == FT_INEDIBLE)
					throw std::invalid_argument((std::string)"ForageValues<> "
							"Forage type `FT_INEDIBLE` is not allowed."+
							" ("+get_forage_type_name(forage_type)+")");
				
				// The map entry for any forage type should have been
				// created on initialization.
				assert( map.size() == FORAGE_TYPES.size() );
				assert( map.count(forage_type) );

				// Change the value.
				map[forage_type] = value;

				assert( map.size() == FORAGE_TYPES.size() );
			}

			/// Sum of all values.
			double sum()const{
				double sum = 0.0;
				for (const_iterator i=begin(); i!=end(); i++)
					sum += i->second;
				return sum;
			}

			/** @{ \name Read-only wrapper around std::map. */
			typedef MapType::const_iterator const_iterator;
			const_iterator begin()const { return map.begin(); }
			const_iterator end()const   { return map.end();   }
			/** @} */ // Wrapper around std::map

		public:
			/** @{ \name Operator overload. */
			ForageValues<tag>& operator+=(const double rhs){
				for (iterator i=begin(); i!=end(); i++) 
					set(i->first, i->second + rhs);
				return *this;
			}
			ForageValues<tag>& operator-=(const double rhs){
				for (iterator i=begin(); i!=end(); i++)
					set(i->first, i->second - rhs);
				return *this;
			}
			ForageValues<tag>& operator*=(const double rhs){
				for (iterator i=begin(); i!=end(); i++)
					set(i->first, i->second * rhs);
				return *this;
			}
			/** \throw std::domain_error If `rhs==0.0`. */
			ForageValues<tag>& operator/=(const double rhs){
				if (rhs==0) throw std::domain_error(
						"Fauna::ForageValues<> Division by zero.");
				for (iterator i=begin(); i!=end(); i++) 
					set(i->first, i->second / rhs);
				return *this;
			}

			ForageValues<tag> operator+(const double rhs)const{
				ForageValues<tag> result(*this);
				return result.operator+=(rhs);
			}
			ForageValues<tag> operator-(const double rhs)const{
				ForageValues<tag> result(*this);
				return result.operator-=(rhs);
			}
			ForageValues<tag> operator*(const double rhs)const{
				ForageValues<tag> result(*this);
				return result.operator*=(rhs);
			}
			/** \throw std::domain_error If `rhs==0.0`. */
			ForageValues<tag> operator/(const double rhs)const{
				ForageValues<tag> result(*this);
				return result.operator/=(rhs);
			}

			ForageValues<tag> operator+(const ForageValues<tag>& rhs)const{
				ForageValues<tag> result(*this);
				return result.operator+=(rhs);
			}
			ForageValues<tag> operator-(const ForageValues<tag>& rhs)const{
				ForageValues<tag> result(*this);
				return result.operator-=(rhs);
			}
			ForageValues<tag> operator*(const ForageValues<tag>& rhs)const{
				ForageValues<tag> result(*this);
				return result.operator*=(rhs);
			}
			/** \throw std::domain_error On division by zero. */
			ForageValues<tag> operator/(const ForageValues<tag>& rhs)const{
				ForageValues<tag> result(*this);
				return result.operator/=(rhs);
			}

			ForageValues<tag>& operator+=(const ForageValues<tag>& rhs){
				for (iterator i=begin(); i!=end(); i++) 
					set(i->first, i->second + rhs.get(i->first));
				return *this;
			}
			ForageValues<tag>& operator-=(const ForageValues<tag>& rhs){
				for (iterator i=begin(); i!=end(); i++)
					set(i->first, i->second - rhs.get(i->first));
				return *this;
			}
			ForageValues<tag>& operator*=(const ForageValues<tag>& rhs){
				for (iterator i=begin(); i!=end(); i++)
					set(i->first, i->second * rhs.get(i->first));
				return *this;
			}
			/** \throw std::domain_error On division by zero. */
			ForageValues<tag>& operator/=(const ForageValues<tag>& rhs){
				for (iterator i=begin(); i!=end(); i++) {
					const double divisor = rhs.get(i->first);
					if (divisor==0) throw std::domain_error((std::string)
							"Fauna::ForageValues<> Division by zero." +
							" ("+get_forage_type_name(i->first)+")");
					set(i->first, i->second / divisor);
				}
				return *this;
			}

			ForageValues<tag>& operator=(const ForageValues<tag>& rhs){
				for (const_iterator i=rhs.begin(); i!=rhs.end(); i++) 
					set(i->first, i->second);
				return *this;
			}

			bool operator==(const ForageValues<tag>& rhs)const{
				for (const_iterator i=rhs.begin(); i!=rhs.end(); i++) 
					if (get(i->first) != i->second) return false;
				return true;
			}
			bool operator!=(const ForageValues<tag>& rhs)const{
				return !operator==(rhs);
			}

			bool operator<(const ForageValues<tag>& rhs)const{
				bool result = true;
				for (const_iterator i=rhs.begin(); i!=rhs.end(); i++) 
					result &= (get(i->first) < i->second);
				return result;
			}
			bool operator<=(const ForageValues<tag>& rhs)const{
				return operator<(rhs) || operator==(rhs);
			}
			bool operator>(const ForageValues<tag>& rhs)const{
				bool result = true;
				for (const_iterator i=rhs.begin(); i!=rhs.end(); i++) 
					result &= (get(i->first) > i->second);
				return result;
			}
			bool operator>=(const ForageValues<tag>& rhs)const{
				return operator>(rhs) || operator==(rhs);
			}
			/** @} */ // Operator overload
		private:
			MapType map;
	};

	/// Digestibility [fraction] for different forage types.
	typedef ForageValues<ZERO_TO_ONE> Digestibility;

	/// Energy values [MJ] for different forage types.
	typedef ForageValues<POSITIVE_AND_ZERO> ForageEnergy;

	/// Net energy content [MJ/kgDM] for different forage types.
	typedef ForageValues<POSITIVE_AND_ZERO> ForageEnergyContent;

	/// A fraction for each forage type.
	typedef ForageValues<ZERO_TO_ONE> ForageFraction;

	/// Dry matter mass values [kgDM or kgDM/km²] for different forage types.
	typedef ForageValues<POSITIVE_AND_ZERO> ForageMass;

	/// Map defining which herbivore gets what to eat [kgDM/km²].
	typedef std::map<HerbivoreInterface*, ForageMass> ForageDistribution;

	// ---------------------------------------------------------------

	/// Base class for herbivore forage in a habitat.
	class ForageBase {
		private:
			double digestibility, dry_matter_mass; 
		public:
			/// Constructor with zero values
			ForageBase():digestibility(0.0), dry_matter_mass(0.0){}

			/// Fractional digestibility of the biomass for ruminants.
			/** Digestibility as measured *in-vitro* with rumen liquor. */
			double get_digestibility() const{return digestibility;}

			/// Dry matter forage biomass over the whole area [kgDM/km²].
			double get_mass()const{return dry_matter_mass;}

			/** \copydoc get_digestibility() 
			 * \throw std::invalid_argument if not `0.0<=d<=1.0`*/
			void   set_digestibility(const double d){
				if (d<0.0 || d>1.0)
					throw std::invalid_argument("Fauna::ForageBase::set_digestibility(): "
							"Digestibility out of range.");
				digestibility = d;
			}

			/** \copydoc get_mass()
			 * \throw std::invalid_argument if dm<0.0 */
			void   set_mass(const double dm){
				if (dm<0.0)
					throw std::invalid_argument("Fauna::ForageBase::set_mass(): "
							"Dry matter is smaller than zero.");
				dry_matter_mass = dm;
			}

		protected:
			/** \copydoc ForageValues::merge() */
			ForageBase& merge_base(const ForageBase& other,
					const double this_weight, const double other_weight);
	};

	/// Grass forage in a habitat.
	class GrassForage: public ForageBase {
		private:
			double fpc;
		public:
			/// Constructor with zero values
			GrassForage():ForageBase(), fpc(0.0){}

			/// Dry matter forage in the area covered by grass [kgDM/km²].
			/** Note that this is always greater or equal than \ref dry_matter_mass */
			double get_sward_density()const{
				if (get_fpc() == 0)
					return 0.0;
				const double sd =  get_mass() / get_fpc();
				assert( sd >= 0.0 && sd >= get_mass());
				return sd;
			}

			/// Foliar percentage cover [fractional].
			/**
			 * Grass-covered area as a fraction of the habitat.
			 * \throw std::logic_error If the values of mass and FPC
			 * don’t fit together: If mass is zero, FPC must also be zero,
			 * and if mass is non-zero, FPC must not be zero.
			 * (Call \ref set_mass() and \ref set_fpc() with correct
			 * values before calling \ref get_fpc().)
			 */
			double get_fpc()const{
				if (get_mass() == 0.0 && fpc != 0.0)
					throw std::logic_error("Fauna::GrassForage::get_fpc() "
							"Mass is zero, but FPC is not zero.");
				if (get_mass() != 0.0 && fpc == 0.0)
					throw std::logic_error("Fauna::GrassForage::get_fpc() "
							"Mass is not zero, but FPC is zero.");
				return fpc;
			}

			/// \copydoc ForageValues::merge()
			GrassForage& merge(const GrassForage& other,
					const double this_weight, const double other_weight);

			/** \copybrief get_fpc()
			 * \note Call \ref get_mass() before this function to
			 * avoid any illogical state (see exceptions in \ref get_fpc()).
			 * \throw std::invalid_argument If not `0.0<=f<=1.0`.
			 * \throw std::logic_error If `f==0.0 && get_mass()==0.0`
			 * or `f==0.0 && get_mass()>0.0`.*/
			void set_fpc(const double f){
				if(!( f>=0.0 && f<=1.0))
					throw std::invalid_argument("Fauna::GrassForage::set_fpc() "
							"FPC out of valid range (0.0–1.0).");
				if (get_mass()==0.0 && f>0.0)
					throw std::logic_error("Fauna::GrassForage::set_fpc() "
							"FPC must be zero if mass is zero.");
				if (get_mass()>0.0 && f==0.0)
					throw std::logic_error("Fauna::GrassForage::set_fpc() "
							"FPC cannot be zero if there is grass mass.");
				fpc=f;
			}
	};

	/// All values for large herbivore forage in a \ref Habitat.
	struct HabitatForage {
		/// Constructor with zero values
		HabitatForage(){}

		/// The grass forage in the habitat.
		GrassForage grass;

		// ADD NEW FORAGE TYPES (E.G. BROWSE) HERE.

		/// Get digestibility [fractional] for all edible forage types.
		/** If mass is zero, digestibility is undefined.*/
		Digestibility get_digestibility()const;

		/// Get dry matter mass [kgDM/km²] for all edible forage types.
		ForageMass get_mass()const;

		/// Total forage in the habitat.
		/** Digestibility is weighted average, forage mass is sum.
		 * If mass is zero, digestibility is also zero.*/
		ForageBase get_total() const;

		/// \copydoc ForageValues::merge()
		HabitatForage& merge(const HabitatForage& other,
				const double this_weight, const double other_weight);

		/// Reference to forage object by forage type.
		/** 
		 * \param ft Forage type.
		 * \return Polymorphic reference to forage class object.
		 * If `ft==FT_INEDIBLE` a zero-value object is returned.
		 * \throw std::logic_error if `ft` is not implemented. */
		const ForageBase& operator[](const ForageType ft)const{
			switch (ft){
				case FT_GRASS: return grass;
				// ADD NEW FORAGE TYPES HERE.
				case FT_INEDIBLE: 
											 static ForageBase empty;
											 return empty;
				default: throw std::logic_error((std::string)
										 "Fauna::HabitatForage::operator[]()const "
										 "Forage type \"" +
										 get_forage_type_name(ft) + "\" "
										 "is not implemented.");
			}
		}

		/** \copydoc operator[]() */
		ForageBase& operator[](const ForageType ft){
			switch (ft){
				case FT_GRASS: return grass;
				// ADD NEW FORAGE TYPES HERE.
				case FT_INEDIBLE: 
											 static ForageBase empty;
											 return empty;
				default: throw std::logic_error("Fauna::HabitatForage::operator[]() "
										 "Forage type \"" +
										 get_forage_type_name(ft) + "\" "
										 "is not implemented.");
			}
		}
	};
}
#endif //HERBIV_FORAGECLASSES_H
