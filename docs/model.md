# Modular Megafauna Model {#page_model}
<!-- For doxygen, this is the *page* header -->
\brief Scientific background of the Modular Megafauna Model.

# Modular Megafauna Model {#sec_model}
<!-- For doxygen, this is the *section* header -->
\tableofcontents

This document explains the design choices for the megafauna model from a scientific rather than technical angle.
It also discusses the different modules in the model framework: what their assumptions are, how to use them, and how to combine them.

Some aspects of the model can only be evaluated in the context of the connected vegetation model.
For LPJ-GUESS you will find those aspects in the megafauna doxygen page of the LPJ-GUESS repository.

\todo
- Give a general introduction for what use case this model was originally developed. -> scientific motivation
- Limitations of the model design:
	+ habitats equal size
	+ After offspring is created no connection to parents ⇒ no lactation, bonding, herding, etc.
- Explain some design choices:
	+ Why differentiate sexes? ⇒ e.g. Shannon et al. (2013), Guthrie (1990)
	+ Why daily time steps?
	+ Why forage types?
	+ Suggest use cases for individual mode.
- Explain the drama of population fluctuations:
	+ What makes populations crash to zero?
	+ What’s the problem with annual allocation?
	+ What mechanisms have we explored to prevent population crashes?
- What is the problem with β (half-max intake density) in Illius & O’Connor (2000) and Pachzelt et al.?

## Basic Model Concepts {#sec_basicconcepts}

The world of the megafauna model is comprised of **simulation units.**
Each such unit consists of a **habitat** and the herbivore **populations** inhabiting it.
The habitat must be implemented by the outside vegetation model.
Each herbivore cohort (or individual) is assigned a **herbivore functional type (HFT),** the herbivore species so to say.
An HFT is simply a user-defined, constant set of parameters defining physiology, life history, and everything else.
Each population contains all herbivores of one HFT in a particular habitat.

Herbivores can be simulated as cohorts or as individuals.
In cohort mode, all herbivores of one annual age class have the same state variables.
In individual mode, each individual herbivore has its own state.
These are the state variables for each herbivore object:

- Age
- Sex
- Current energy need
- Fat mass

![Basic model entities in the Modular Megafauna Model. Aggregation units (disjunct sets of simulation units) are only needed to aggregate output spatially.](images/model_entities.svg)

## Nitrogen Cycling {#sec_nitrogen_cycling}

The vegetation model defines the nitrogen content in forage.
The maximum amount of nitrogen (\f$N_{bound}\f$, kgN/km²) bound in herbivores is comprised of the body tissue and the contents of the digestive tract.
Any ingested nitrogen is added to the pool of herbivore-bound nitrogen, and the surplus is returned to the vegetation model, which should make it available to plants again.

The amount of nitrogen bound in body tissue is approximated with 3% of live body weight (Robbins 1983\cite robbins1983wildlife); this ignores variation in fat and structural mass.
Upon death, this amount of nitrogen is also returned to the vegetation model.

The nitrogen of ingesta in stomach and intestines depends on the mean retention time (\f$MRT\f$, hours) and the day’s intake of nitrogen (\f$I_N\f$, kgN/ind/day).

\f[
  N_{bound} = N_{guts} + N_{body} = I_N * MRT * P + 0.03 * M * P
\f]

\f$P\f$ is the population density (ind/km²) and \f$M\f$ is the body mass (kg/ind).
Mean retention time in hours is calculated according to Clauss et al. (2007)\cite clauss2007case, Fig. 2:

\f[
  MRT = 32.8 * M^{0.07}
\f]

## Energetics {#sec_energetics}

### Thermoregulation by Conductance {#sec_thermoregulation}

This model of thermoregulation is often called the **Scholander-Irving model** and was published in two seminal papers in 1950: \cite scholander1950adaptation \cite scholander1950heat.
The more detailed implementation is taken from Peters (1983)\cite peters1983ecological.

Homeothermic animals have extra energy costs to maintain their body core temperature.
Through basal metabolism and other ways of energy burning, heat is already passively created.
Thermoregulatory costs arise when the ambient temperature drops below the *lower critical temperature*: the passive heat from thermoneutral metabolism is not counterbalance heat loss to the environment.
The rate of heat loss depends on the *thermal conductance* of the whole animal (energy flow per temperature difference), which in turn depends on the *thermal conductivity* (energy flow per temperature difference and per thickness) of fur and skin and the body surface.
Conductance is the inverse of resistance or insulation, and conductivity is the inverse of resistivity.

- \f$T_{crit}\f$: Lower critical temperature [°C].
- \f$T_{core}\f$: Body core temperature [°C].
- \f$T_{air}\f$: Ambient air temperature [°C].
- \f$E_{neu}\f$: Thermoneutral metabolic rate [MJ/ind/day]
- \f$C\f$: Whole-body thermal conductance [W/ind].
- \f$\Phi\f$: Heat loss [MJ/ind/day]
\f[
  T_{crit} = T_{core} - \frac{E_{neu}}{C}
\f]
\f[
  \Phi = C * max(T_{crit} - T_{air}, 0)
\f]

![](thermoregulation.png "Schematic description of the effects of external temperature on the metabolic rate in homeotherms. – Peters 1983, Fig. 5.6")

\note In its current form, the model only considers costs when temperatures are too low.
Overheating effects are not implemented since the model was developed with the focus on Arctic megafauna.

#### Conductance

The critical parameter for thermoregulatory expenditure is the (whole-body) conductance: the rate of heat flow per difference between core and air temperature (W/°C).
The conductance can be approximated from the average conductivity and the body surface.
Conductivity is the inverse of insulation: it is the heat flow per temperature difference per area.

Body surface in m² scales roughly as \f$0.09*M^{0.66}\f$ ([Hudson & White 1985](\cite hudson1985bioenergetics)).

## Energy Content of Forage {#sec_energycontent}

\todo explain gross, digestible, metabolizable and net energy

## Foraging {#sec_foraging}

\note **Units**
- All forage values (e.g. available grass biomass, consumed forage) are *dry matter mass* in kilograms (`DMkg`).
- Any forage per area (e.g. forage in a habitat) is `kgDM/km²`.
- Herbivore-related mass values (e.g. body mass, fat mass) are also `kg`, but live mass.
- Population densities of herbivores are either in `kg/km²` or `ind/km²` (ind=individuals).
- Digestibility values are interpreted as in-vitro digestibility.

\todo Find a reference for this definition of in-vitro digestibility.

@startuml "Levels of herbivore intake constraints. What and how much of the available forage an herbivore ingests is limited by a cascade of internal and external factors."
	!include diagrams.iuml!intake_limit_levels
@enduml

## Reproduction {#sec_reproduction}

## Life History {#sec_life_history}

\todo Explain how growth is linear in \ref Fauna::HerbivoreBase::get_bodymass()

## Minimum Density Threshold {#sec_minimum_density_threshold}
The parameter \ref Fauna::Hft::mortality_minimum_density_threshold defines at which point a dwindling population (sum of all cohorts/individuals) may be considered dead.
It is an arbitrary, but critical value for model performance.
Possible re-establishment only happens if all cohorts are dead within one habitat.

It is important to keep this parameter low enough for slow-breeding and long-lived animals because otherwise they may die out after establishment:
After establishment, the background mortality continually diminishes the adult cohorts, and after some years the total population (all cohorts together) my drop below the `minimum_density_threshold` before reproduction could compensate.

On the other hand, the `minimum_density_threshold` should not be set *too* low as this would result in extremely thin “ghost” populations that are effectively preventing re-establishment.

## Mortality {#sec_mortality}

## Species Coexistence {#sec_coexistence}

The classical competitive exclusion principle predicts that no two species can coexist in the long term if they each solely depend on one shared resource (\cite hardin1960competitive).
One species will inevitably outcompete the other one.
Though there are indeed ecological mechanisms that can facilitate coexistence with a shared resource (\cite chesson2000mechanisms), the parameter space for this to happen in a model is usually very narrow (e.g. \cite vanlangevelde2008intantaneous).

In order to simply avoid competition among different HFTs, the option `one_hft_per_patch` can be enabled: Each HFT exists on its own, without any interaction with other species.
With that option enabled, all HFTs should each be assigned to the same number of habitats.
It is the responsibility of the host application (the vegetation model) to ensure that the number of habitats is an integer multiple of the HFT count.

------------------------------------------------------------

\author Wolfgang Traylor, Senckenberg BiK-F
\date 2019
\copyright ...
