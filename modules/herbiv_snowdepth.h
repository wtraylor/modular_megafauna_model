///////////////////////////////////////////////////////////////////////////////////////
/// \file 
/// \brief Models to calculate snow depth for herbivores in LPJ-GUESS.
/// \ingroup group_herbivory
/// \author Wolfgang Pappa, Senckenberg BiK-F
/// \date November 2017
///////////////////////////////////////////////////////////////////////////////////////
#ifndef HERBIV_SNOWDEPTH_H
#define HERBIV_SNOWDEPTH_H

// forward declarations
class Patch;

namespace Fauna {
	struct GetSnowDepth{
		/// Get snow depth [cm].
		virtual double operator()(const Patch&)const = 0;
	};

	/// Assume a constant 10:1 ratio of snow water equivalent to snow depth.
	/** \see \ref SD_TEN_TO_ONE */
	struct SnowDepthTenToOne: public GetSnowDepth{
		/** \copybrief GetSnowDepth::operator()()
		 * \return \ref Soil::snowpack times 10. Result in cm.
		 */
		virtual double operator()(const Patch&)const;
	};
}
#endif // HERBIV_SNOWDEPTH_H
