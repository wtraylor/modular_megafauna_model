Quickstart Guide for the Large Herbivore Module {#page_quickstart}
=========================================================================
<!-- For doxygen, this is the *page* header -->
\brief How to get the first simulations running.

Quickstart Guide for the Large Herbivore Module {#sec_quickstart}
========================================================================
<!-- For doxygen, this is the *section* header -->
\tableofcontents


Constant Population {#sec_constant_pop}
----------------------------------------------

Prescribing a constant herbivore density to the model may be used to simulate the effects of different stocking regimes.
Reproduction should then be disabled.
The herbivores may be immortal so that the stocking rate remains constant even if the animals are starving.
This is a minimal example herbivore instruction file for cattle:

```fortran
ifherbivory 1
digestibility_model "npp"
free_herbivory_years 100
herbivore_type "cohort"
hft "cattle" (
	include 1
	bodyfat_max   0.3 ! fractional
	bodyfat_max_daily_gain 0.0 ! fractional
	bodymass_female 400 ! kg
	bodymass_male   400 ! kg
	diet_composer "pure_grazer"
	digestion_type "ruminant"
	digestive_limit "illius_gordon_1992"
	establishment_age_range 1 1 ! arbitrary
	establishment_density 1.0 ! ind/km²
	expenditure_components "taylor_1981"
	foraging_limits "illius_oconnor_2000"
	half_max_intake_density  40 ! gDM/m²
)
```
The parameter values are mostly taken from Illius & O’Connor (2000)\cite illius2000resource.
As this only defines the parameters for the herbivory model, you will need to `import` the LPJ-GUESS instruction files, specify the driving data, and select the herbivory output files you are interested in.

With this instruction file, one immortal cohort of adult body mass will be established.
The mass density may fluctuate because of fat mass, but individual density will remain constant.

The `establishment_density` is probably the most crucial parameter in this setup.
Currently, the density can only prescribed globally and is constant.

