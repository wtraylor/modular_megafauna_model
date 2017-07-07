Large Herbivore Model {#page_herbiv_model}
==========================================
<!-- For doxygen, this is the *page* header -->
\brief Structure and scientific explanation of the Large Herbivore Model.

Large Herbivore Model {#sec_herbiv_model}
==========================================
<!-- For doxygen, this is the *section* header -->

<!--TODO:
- Limitations of the model design:
	+ 
	+ After offspring is created no connection to parents 
    -> no lactation, bonding, herding, etc.
- Overview on the replacable model components in a nice
  UML diagram:
	+ Habitat
	+ HerbivoreInterface
	+ DigestibilityModel (for PatchHabitat)
-->

Foraging {#sec_herbiv_foraging}
-------------------------------

\note **Units**<br>All forage values (e.g. available grass biomass,
consumed forage) are *dry matter mass* in kilograms (`DMkg`).
Any forage per area (e.g. forage in a habitat) is `DMkg/m²`.
Herbivore-related mass values (e.g. body mass, fat mass) are also
`kg`, but live mass.
Population densities of herbivores are either in `kg/km²` or `ind/km²` (ind=individuals).

### Feeding on Plants in a Patch ### {#sec_herbiv_foraging_patch}

Each \ref Individual offers an amount of forage (kgDM/m²) that is available
to herbivores (\ref Individual.get_forage_mass()).

\todo Growth happens only once per year (\ref growth()) for natural vegetation.
However, seasonal shifts of forage availability are crucial for herbivore
dynamics.
A solution for that is yet to be found.


### Digestibivity ### {#sec_herbiv_digestibility}


Reproduction {#sec_herbiv_reproduction}
---------------------------------------

Mortality {#sec_herbiv_mortality}
---------------------------------

Nitrogen Excretion {#sec_herbiv_nitrogen_excretion}
---------------------------------------------------


Trampling {#sec_herbiv_trampling}
---------------------------------

------------------------------------------------------------

\author Wolfgang Pappa, Senckenberg BiK-F
\date May 2017
\see \ref page_herbiv_design
\see \ref page_herbiv_tutor
\see \ref page_herbiv_tests
\see \ref group_herbivory
