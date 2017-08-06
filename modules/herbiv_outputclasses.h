///////////////////////////////////////////////////////////////////
/// \file 
/// \brief Output classes of the herbivory module.
/// \ingroup group_herbivory
/// \author Wolfgang Pappa, Senckenberg BiK-F
/// \date August 2017
/// \see \ref sec_herbiv_output
////////////////////////////////////////////////////////////////////
#ifndef HERBIV_OUTPUT_H
#define HERBIV_OUTPUT_H

namespace Fauna {
	/// Helper function to see if a day is the first of a month.
	/**
	 * \param day Day of year (0=Jan 1st).
	 * \return True if `day` (0–364) is first day of a month.
	 * \throw std::invalid_argument If `day` not in [0,364].
	 * \todo This is misplaced.
	 */
	bool is_first_day_of_month(int day);
}

namespace FaunaOut {
}

#endif // HERBIV_OUTPUT_H
