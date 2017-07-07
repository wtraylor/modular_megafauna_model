///////////////////////////////////////////////////////////////////////////////////////
/// \file
/// \brief Management classes of herbivore populations.
/// \ingroup group_herbivory
/// \author Wolfgang Pappa, Senckenberg BiK-F
/// \date July 2017
///////////////////////////////////////////////////////////////////////////////////////

#include "config.h"
#include "herbiv_population.h"
#include "herbiv_herbivore.h"
#include "herbiv_hft.h"
#include <stdexcept> // for std::out_of_range
#include <sstream> // for std::ostringstream

using namespace Fauna;

//============================================================
// IndividualPopulation
//============================================================

void IndividualPopulation::create(const double density, const int age){
	// TODO
}

std::vector<const HerbivoreInterface*> IndividualPopulation::get_list()const{
	std::vector<const HerbivoreInterface*> result;
	result.resize(list.size());
	// TODO
}

std::vector<HerbivoreInterface*> IndividualPopulation::get_list(){
	// TODO
}

void IndividualPopulation::remove_dead(){
	// TODO
}

//============================================================
// CohortPopulation
//============================================================

void CohortPopulation::create(const double density, const int age){
	// TODO
}

std::vector<const HerbivoreInterface*> CohortPopulation::get_list()const{
	std::vector<const HerbivoreInterface*> result;
	result.resize(list.size());
	// TODO
}

std::vector<HerbivoreInterface*> CohortPopulation::get_list(){
	// TODO
}

void CohortPopulation::remove_dead(){
	// TODO
}

//============================================================
// HftPopulationsMap
//============================================================

void HftPopulationsMap::add(PopulationInterface* new_pop) {
	if (new_pop == NULL)
		throw std::invalid_argument("HftPopulationsMap::add(): "
				"pointer is NULL");

	const Hft& hft = new_pop->get_hft();

	for (iterator iter=begin(); iter!=end(); iter++){
		PopulationInterface& pop = **iter;
		if (pop.get_hft() == hft)
			throw std::logic_error("HftPopulationsMap::add(): "
					"A population of HFT \""+hft.name+"\" already exists.");
	}

	vec.push_back(new_pop);
}

PopulationInterface& HftPopulationsMap::operator[](const Hft& hft){
	for (iterator iter=begin(); iter!=end(); iter++){
		PopulationInterface& pop = **iter;
		if (pop.get_hft() == hft)
			return pop;
	}
	// loop ended without finding the HFT
	throw std::invalid_argument("HftPopulationsMap::operator[](): "
			"No population of HFT \""+hft.name+"\"");
}

HftPopulationsMap::~HftPopulationsMap(){
	iterator iter = begin();
	while (iter != end()){
		delete *iter;
		vec.erase(iter);
	}
}
