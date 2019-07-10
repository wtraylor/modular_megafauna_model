//////////////////////////////////////////////////////////////////////////
/// \file
/// \brief   \ref Fauna::Habitat implementations for testing purpose.
/// \author  Wolfgang Pappa, Senckenberg BiK-F
/// \date    June 2017
//////////////////////////////////////////////////////////////////////////

#ifndef TESTHABITATS_H
#define TESTHABITATS_H

#include <vector>
#include "habitat.h"

using namespace Fauna;

// forward declarations
namespace Fauna {
class SimulationUnit;
}

namespace FaunaSim {

/// Helper class for performing simple grass growth to test herbivore
/// functionality
class LogisticGrass {
 public:
  /// Settings for grass growth
  struct Parameters {
    /// Constructor with arbitrary simple, *valid* values, but no growth.
    Parameters() : fpc(0.1), init_mass(0.0), reserve(0.1), saturation(1.0) {
      digestibility.push_back(.5);
      decay_monthly.push_back(0.0);
      growth_monthly.push_back(0.0);
    }

    /// Proportional daily rates of grass decay [day^-1]
    /**
     * This is a vector of *daily* decay rates for each month. When
     * the end of the vector is reached, the values are recycled.
     * A vector of length 12 creates the same behaviour every year.
     */
    std::vector<double> decay_monthly;

    /// Proportional digestibility of the grass [frac].
    /**
     * This is a vector of digestibility values for each month. When
     * the end of the vector is reached, the values are recycled.
     * A vector of length 12 creates the same behaviour every year.
     */
    std::vector<double> digestibility;

    /// \brief Percentage of habitat covered with grass
    ///        (Foliar Percentage Cover) [frac]
    double fpc;

    /// Proportional daily grass growth rates [day^-1]
    /**
     * This is a vector of *daily* growth rates for each month. When
     * the end of the vector is reached, the values are recycled.
     * A vector of length 12 creates the same behaviour every year.
     */
    std::vector<double> growth_monthly;

    /// Initial available forage [kgDM/km²]
    /** This should be smaller than \ref saturation */
    double init_mass;

    /// \brief Ungrazable grass biomass reserve, inaccessable
    ///        to herbivores [kgDM/km²]
    /** Owen-Smith (2002) gives value of 20 g/m²*/
    double reserve;

    /// Saturation grass biomass [kgDM/m²]
    /** Owen-Smith (2002): 200 g/m²*/
    double saturation;

    /// Check if parameters are valid
    /**
     * \param[out] msg Possible warnings and error messages.
     * \return true if values are valid, false if not.
     */
    bool is_valid(std::string& msg) const;
  };

  /// Constructor
  /**
   * \throw std::invalid_argument If `settings` are not valid.
   */
  LogisticGrass(const LogisticGrass::Parameters& settings);

  /// Perform grass growth and decay for one day.
  /**
   * \param day_of_year January 1st = 0
   * \throw std::invalid_argument if not `0<=day_of_year<=364`
   */
  void grow_daily(const int day_of_year);

  /// Get current grass forage
  const GrassForage& get_forage() const { return forage; }

  /// Set the grass forage
  void set_forage(const GrassForage& f) { forage = f; }

 private:
  /// Current forage
  /** Excluding the reserve
   * \ref LogisticGrass::Parameters::reserve. */
  GrassForage forage;

  LogisticGrass::Parameters settings;

  /// The current simulation month, starting with zero.
  /** We need this to address the current value in
   * \ref Parameters::growth_monthly and
   * \ref Parameters::decay_monthly.
   */
  int simulation_month;
};

/// A herbivore habitat independent of the LPJ-GUESS framework for testing.
class SimpleHabitat : public Habitat {
 public:
  /// Simulation parameters for a \ref SimpleHabitat object.
  struct Parameters {
    /// Parameters for logistic grass growth.
    LogisticGrass::Parameters grass;

    /// Snow depth [cm] for each month.
    /** When the end of the vector is reached, the values are recycled.
     * A vector of length 12 creates the same behaviour every year. */
    std::vector<double> snow_depth_monthly;
  };

  /// Constructor with simulation settings.
  /**
   * \param settings Simulation settings for the vegetation
   * model.
   */
  SimpleHabitat(const SimpleHabitat::Parameters settings)
      : settings(settings),
        grass(settings.grass),
        simulation_month(0),
        snow_depth(0.0) {}

 public:  // ------ Fauna::Habitat implementations ----
  virtual void add_excreted_nitrogen(const double) {}  // disabled
  virtual HabitatForage get_available_forage() const {
    HabitatForage result;
    result.grass = grass.get_forage();
    return result;
  }
  virtual HabitatEnvironment get_environment() const;
  virtual void init_day(const int today);
  virtual void remove_eaten_forage(const ForageMass& eaten_forage);

 protected:
  /// Perform daily growth.
  /** \param day_of_year January 1st = 0 */
  virtual void grow_daily(const int day_of_year) {
    grass.grow_daily(day_of_year);
  }

 private:
  SimpleHabitat::Parameters settings;

  /// Snow depth in cm, as read from \ref Parameters::snow_depth_monthly.
  double snow_depth;

  /// Grass in the habitat
  LogisticGrass grass;

  /// The current simulation month, starting with zero.
  /** We need this to address the current value in
   * \ref Parameters::snow_depth_monthly. */
  int simulation_month;
};

/// A set of \ref Fauna::SimulationUnit objects.
/**
 * In the herbivore test simulations this corresponds
 * semantically to a \ref ::Gridcell with \ref ::Patch objects.
 */
class HabitatGroup {
 public:
  /// Constructor
  /**
   * \param lon longitude (just for output labels)
   * \param lat latitude  (just for output labels)
   */
  HabitatGroup(const double lon, const double lat) : lon(lon), lat(lat) {}

  /// Destructor, deleting all \ref Fauna::SimulationUnit instances.
  ~HabitatGroup();

  /// Add a newly created \ref Fauna::SimulationUnit object
  /** The object instance will be owned by the group
   * and released on its destruction.
   * \param new_unit Pointer to a newly created object
   * \throw std::invalid_argument if `new_unit.get()==NULL`
   */
  void add(std::auto_ptr<SimulationUnit> new_unit);

  /// Latitude as defined in the constructor.
  double get_lon() const { return lon; }
  /// Latitude as defined in the constructor.
  double get_lat() const { return lat; }

  /// Get the underlying vector of references.
  const std::vector<SimulationUnit*>& get_vector() { return vec; }

  /** @{ \name Wrapper around std::vector
   * Equivalents to methods in Standard Library Container std::vector.*/
  typedef std::vector<SimulationUnit*>::iterator iterator;
  typedef std::vector<SimulationUnit*>::const_iterator const_iterator;
  iterator begin() { return vec.begin(); }
  const_iterator begin() const { return vec.begin(); }
  iterator end() { return vec.end(); }
  const_iterator end() const { return vec.end(); }
  int size() const { return vec.size(); }
  void reserve(const int size) { vec.reserve(size); }
  /** @} */  // Container functionality
 private:
  double lon, lat;
  std::vector<SimulationUnit*> vec;

  // Deleted copy constructor and copy assignment operator.
  // If they were not deleted, the unique ownership of the
  // Habitat objects could be lost.
  HabitatGroup(const HabitatGroup&);
  HabitatGroup& operator=(const HabitatGroup&);
};

/// A simple vector of pointers \ref HabitatGroup objects.
/** This is a helper class for \ref FaunaSim::Framework.
 * Pointers are owned by this list.*/
class HabitatGroupList {
 public:
  /// Constructor
  HabitatGroupList() {}

  /// Destructor
  ~HabitatGroupList() {
    for (iterator itr = begin(); itr != end(); itr++) delete *itr;
    vec.clear();
  }

  /// Add a new element.
  /** \param new_group New object, now owned by the list.
   * \return Reference to `new_group`.
   * \throw std::logic_error If a habitat group with the same
   * longitute and latitude already exists.
   * \throw std::invalid_argument If `new_group.get()==NULL`.
   */
  HabitatGroup& add(std::auto_ptr<HabitatGroup> new_group);

  /** @{ \name Wrapper around std::vector
   * Equivalents to methods in Standard Library Container std::vector.*/
  typedef std::vector<HabitatGroup*>::iterator iterator;
  typedef std::vector<HabitatGroup*>::const_iterator const_iterator;
  iterator begin() { return vec.begin(); }
  const_iterator begin() const { return vec.begin(); }
  iterator end() { return vec.end(); }
  const_iterator end() const { return vec.end(); }
  int size() const { return vec.size(); }
  void reserve(const int size) { vec.reserve(size); }
  /** @} */  // Container functionality
 private:
  std::vector<HabitatGroup*> vec;
  // deleted copy constructor and copy assignment operator
  HabitatGroupList(const HabitatGroupList&);
  HabitatGroupList& operator=(const HabitatGroupList);
};
}  // namespace FaunaSim

#endif  // TESTHABITATS_H
