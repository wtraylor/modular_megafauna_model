//////////////////////////////////////////////////////////
/// \file 
/// \ingroup group_herbivory
/// \brief Functions and classes shared across parts of the herbivory 
///        module. 
/// \author Wolfgang Pappa, Senckenberg BiK-F
/// \date October 2017
///////////////////////////////////////////////////////////
#ifndef HERBIV_UTILS_H
#define HERBIV_UTILS_H

#include <deque>
#include <cstdlib> // for std::rand(), srand(), RAND_MAX
#include <ctime>   // for time(), to init random seed

namespace Fauna{
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
	double average(const double a, const double b,
			const double weight_a=1.0, const double weight_b=1.0);

	/// Get a random number in the interval [0,1]
	/**
	 * For stochastic mortality.
	 *
	 * This implementation simply uses the standard library 
	 * function `rand()` with the current time as random seed.
	 */
	inline double get_random_fraction(){
		// initialize random seed
		srand (time(NULL));
		return (double) rand() / RAND_MAX;
	}

  /// Average of a `double` value over a given time period.
  /**
   * This helper class successively takes `double` values and 
   * stores/records them up to a given count. 
   * At any time, the average (arithmetic mean) over the stored values
   * can be queried with \ref get_average().
   *
   * Use this to keep track of for instance the average body condition
   * of the last month or the average phenology of the last year.
   * In the first case, you would create the object with `count==30` and
   * call \ref add_value() exactly once every day. 
   * In the second scenario, `count` would equal `365`.
   */
  class PeriodAverage{
    public:
      /// Constructor.
      /**
       * \param count Number of values to remember and use for average.
       * \throw std::invalid_argument If `count<=0`.
       */
      PeriodAverage(const int count);

      /// Add a value to the record.
      void add_value(const double);

      /// Get arithmetic mean over all so-far recorded values.
			/**
			 * \throw std::logic_error If no values were added yet.
			 */
      double get_average()const;
    private:
      std::deque<double> deque;
      int count; // const
  };

}
#endif // HERBIV_UTILS_H
