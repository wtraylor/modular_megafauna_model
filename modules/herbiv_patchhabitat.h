///////////////////////////////////////////////////////////////////////////////////////
/// \file 
/// \brief Implementation of a herbivore habitat for a \ref Patch.
/// \ingroup group_herbivory
/// \author Wolfgang Pappa, Senckenberg BiK-F
/// \date June 2017
///////////////////////////////////////////////////////////////////////////////////////
#ifndef HERBIV_PATCHHABITAT_H
#define HERBIV_PATCHHABITAT_H

#include "herbiv_habitat.h"

// forward declarations
class Patch;
class Pft;

namespace Fauna {
	// forward declarations
	class Parameters;
	class GetDigestibility;

	/// Class with herbivore habitat functionality for an LPJ-GUESS \ref Patch.
	/** 
	 * Objects of this class are supposed to be owned by the
	 * corresponding \ref Patch object which takes care of 
	 * instantiating and releasing.
	 *
	 * @startuml "Dependencies of Fauna::PatchHabitat" 
	 * hide members
	 * hide methods
	 * Patch "1"                 <--> "1" Fauna.PatchHabitat
	 * abstract Fauna.Habitat
	 * Fauna.Habitat         <|-- Fauna.PatchHabitat
	 * interface "Fauna.GetDigestibility"
	 * Fauna.GetDigestibility <-- Fauna.PatchHabitat
	 * Fauna.HftPopulationsMap --* Fauna.Habitat
	 * interface Fauna.PopulationInterface
	 * Fauna.HftPopulationsMap *-- "*" Fauna.PopulationInterface
	 * @enduml
	 * \note There are no unit tests for this class because the
	 * class \ref Patch cannot reasonably be instantiated in a
	 * unit test without the whole LPJ-GUESS framework.
	 */
	class PatchHabitat : public Habitat{
	public:
		/// Constructor
		/**
		 * \param populations The herbivore populations
		 * \param patch The one-to-one relationship to the patch
		 * \param digestibility_model Strategy object for
		 * calculating the digestibility of forage (constructor
		 * injection)
		 * \see \ref sec_inversion_of_control
		 */
		PatchHabitat( std::auto_ptr<HftPopulationsMap> populations,
				Patch& patch, 
				const GetDigestibility& digestibility_model):
			Habitat(populations), // parent constructor
			patch(patch), 
			get_digestibility(digestibility_model){}

		/// Get currently available herbivore forage in the \ref Patch.
		/** \return in kg/km² */
		virtual HabitatForage get_available_forage() const;

		/// Remove eaten forage from the \ref Patch.
		/** \param eaten_forage in kg/m² */
		virtual void remove_eaten_forage(const ForageMass& eaten_forage);

	protected:
		const GetDigestibility& get_digestibility;
	private:
		/// Reference to the patch.
		Patch& patch;

	};
}
#endif // HERBIV_PATCHHABITAT_H
