//////////////////////////////////////////////////////////////////////////
/// \file 
/// \brief Function objects to distribute forage among herbivores.
/// \ingroup group_herbivory
/// \author Wolfgang Pappa, Senckenberg BiK-F
/// \date 2018-06-16
//////////////////////////////////////////////////////////////////////////

#include "config.h"
#include "feed.h"
#include "habitat.h"       // for Habitat and Population
#include "herbivore.h"     // for HerbivoreInterface

using namespace Fauna;

//============================================================
// DistributeForageEqually
//============================================================

void DistributeForageEqually::operator()(
		const HabitatForage& available,
		ForageDistribution& forage_distribution) const {
	if (forage_distribution.empty()) return;

	// BUILD SUM OF ALL DEMANDED FORAGE
	ForageMass demand_sum;
	// iterate through all herbivores
	for (ForageDistribution::iterator itr = forage_distribution.begin();
			itr != forage_distribution.end(); itr++) {
		demand_sum += itr->second;
	}

	// Only distribute a little less than `available` in order
	// to mitigate precision errors.
	const ForageMass avail_mass = available.get_mass() * 0.999;

	// If there is not more demanded than is available, nothing
	// needs to be distributed.
	if (demand_sum <= avail_mass)
		return;

	// MAKE EQUAL PORTIONS
	// iterate through all herbivores
	for (ForageDistribution::iterator itr = forage_distribution.begin();
			itr != forage_distribution.end(); itr++) {
		assert(itr->first != NULL);
		const HerbivoreInterface& herbivore = *(itr->first);
		const ForageMass& demand = itr->second; // input
		ForageMass&      portion = itr->second; // output

		// calculate the right portion for each forage type
		for (ForageMass::const_iterator itr_ft = demand.begin();
				itr_ft != demand.end(); itr_ft++){
			const ForageType ft = itr_ft->first;

			if (demand_sum[ft] != 0.0)
				portion.set(ft,  avail_mass[ft] * 
						demand[ft] / demand_sum[ft]);
		}
	}
}

//============================================================
// FeedHerbivores
//============================================================

FeedHerbivores::FeedHerbivores(std::auto_ptr<DistributeForage> _distribute_forage):
	distribute_forage(_distribute_forage)
{
	if (distribute_forage.get() == NULL)
		throw std::invalid_argument("Fauna::FeedHerbivores::FeedHerbivores() "
				"Parameter `distribute_forage` is NULL.");
}

void FeedHerbivores::operator()(
		HabitatForage& available,
		const HerbivoreVector& herbivores) const{


	// loop as many times as there are forage types
	// to allow prey switching: 
	// If one forage type gets “empty” in the first loop, the
	// herbivores can then demand from another forage type, and so
	// on until it’s all empty or they are all satisfied or cannot
	// switch to another forage type.
	for (int i=0; i<FORAGE_TYPES.size(); i++){

		// If there is no forage available (anymore), abort!
		if (available.get_mass() <= 0.00001)
			break;

		//------------------------------------------------------------
		// GET FORAGE DEMANDS
		ForageDistribution forage_demand;
		for (HerbivoreVector::const_iterator itr=herbivores.begin();
				itr != herbivores.end(); itr++){
			HerbivoreInterface& herbivore = **itr;

			// Skip dead herbivores.
			if (herbivore.is_dead())
				continue;

			// calculate forage demand for this herbivore
			const ForageMass ind_demand = herbivore.get_forage_demands(available);

			// only add those herbivores that do want to eat
			if (!(ind_demand == 0.0))
				forage_demand[&herbivore] = ind_demand;
		}

		// abort if all herbivores are satisfied
		if (forage_demand.empty())
			break;

		// get the forage distribution
		assert(distribute_forage.get() != NULL);
		(*distribute_forage)( available, forage_demand);

		// rename variable to make clear it’s not the demands anymore
		// but the portions to feed the herbivores
		ForageDistribution& forage_portions = forage_demand;

		//------------------------------------------------------------
		// LET THE HERBIVORES EAT

		const Digestibility digestibility     = available.get_digestibility();
		const ForageFraction nitrogen_content = available.get_nitrogen_content();

		// Loop through all portions and feed it to the respective
		// herbivore
		for (ForageDistribution::iterator iter=forage_portions.begin();
				iter != forage_portions.end(); iter++)
		{
			const ForageMass& portion = iter->second; // [kgDM/km²]
			HerbivoreInterface& herbivore = *(iter->first);

			const ForageMass& nitrogen = portion * nitrogen_content;

			if (herbivore.get_ind_per_km2() > 0.0) {
				// feed this herbivore
				herbivore.eat(portion, digestibility, nitrogen);

				// reduce the available forage
				for (std::set<ForageType>::const_iterator ft = FORAGE_TYPES.begin();
						ft != FORAGE_TYPES.end(); ft++)
				{
					available[*ft].set_nitrogen_mass(
							available[*ft].get_nitrogen_mass() - nitrogen[*ft] );
					available[*ft].set_mass(
							available[*ft].get_mass() - portion[*ft] );
				}
			}
		}
	}
}

