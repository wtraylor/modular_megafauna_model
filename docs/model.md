# Ecological Model Description {#page_model}
\brief Scientific background of the Modular Megafauna Model.

\tableofcontents

This document explains the design choices for the megafauna model from a scientific rather than a programmatical angle.
It also discusses the different modules in the model framework: what their assumptions are, how to use them, and how to combine them.

The first section introduces the framework of the simulation scheme.
The following four sections describe the four distinct, but interlinked conceptual elements of the modeled animal ecology.
The last section covers some lessons learned from emerging model behavior.

- Basic Model Concepts
- Energy Household
- Reproduction
- Mortality
- Effects on Environment
- Population Dynamics

Some aspects of the model can only be evaluated in the context of the connected vegetation model.
For LPJ-GUESS you will find those aspects in the megafauna doxygen page of the LPJ-GUESS repository.

## Basic Model Concepts {#sec_basic_model_concepts}

\todo
- Give a general introduction for what use case this model was originally developed. -> scientific motivation
- Limitations of the model design:
	+ habitats equal size
	+ After offspring is created no connection to parents ⇒ no lactation, bonding, herding, etc.
- Explain some design choices:
	+ Why differentiate sexes? ⇒ e.g. Shannon et al. (2013), Guthrie (1990)
	+ Why forage types?
	+ Suggest use cases for individual mode.

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

The simulations run with daily time steps. The predecessor model by Adrian
Pachzelt \cite pachzelt2013coupling operated on a monthly schedule and was thus
much faster. However, the Modular Megafauna Model should be applicable on
different spatial and temporal scales. LPJ-GUESS simulates vegetation in a
daily schedule, and so naturally the attached herbivores should be treated the
same. Moreover, there has been no formal analysis how much a coarser temporal
resolution affects the model outcome. So it seemed better to air on the side of
a finer resolution.

Offspring of large herbivores usually shows an even sex ratio.
Most model processes don’t differentiate between males and females, only body size and age of maturity have sex-specific parameters.
It seems important to at least set the basis for gender differentiation because some large herbivores do show pronounced sexual dimorphism not only in size (e.g. bison or proboscideans) but also in diet and behavior (e.g. elephants: \cite shannon2013diet).

### Life History {#sec_life_history}

\todo Explain how growth is linear in \ref Fauna::HerbivoreBase::get_bodymass()

## Energy Household {#sec_energy_household}

![Energy household for a ruminant or hindgut fermenter. Modified after Minson (1990), Fig. 5.1.](images/energy_household.svg)

### Energy Content of Forage {#sec_energy_content}

Research has focused mainly on the digestion of ruminant livestock, and so the
megafauna model works primarily with the well-established formulas for
ruminants. To account for the less efficient digestion of hindgut fermenters, a
constant factor **η** is applied to the net energy extracted by these species
\cite illius1992modelling.

The diagram on the energy household shows how energy from the forage is used by
an herbivore: "Gross energy" (**GE**) is the heat that could be produced from
complete combustion of the feedstuff. From that, the part which is not excreted
in faeces is the "digestible energy" (**DE**). Some proportion of it is then lost
to urine and gas production, but the rest is "metabolizable energy" (**ME**).
After deducing now the losses due to heat increment, the remaining "net energy"
(**NE**) is effectively utilizable for all physiological processes.

Gross energy depends only on the physical properties of the forage and measured
in a combustion chamber. It is therefore independent of the animal.
McDonald et al. (2010)\cite mcdonald2010animal provide an overview of gross
energy in different feedstuffs for livestock (p. 259). It typically ranges
between 18 to 20 MJ/kgDM.

The proportional dry-matter digestibility (**DMD**) of the forage is a central
variable in the model. It measures the fraction of the gross energy that is
usable by the animal. The rest gets excreted in the feces because it is
undigestible fiber: protected cellulose and hemicellulose, silica, and cutin.
Agricultural research has shown that the digestibility is closely correlated
with metabolizable energy and net energy (Minson, 1990, p. 7
\cite minson1990forage).

Digestibility is modelled as the one indicator for forage quality, which means
forage energy density, and must be given by the vegetation model. This neglects
any other effects on the digestibility, like interactions of different forages
or effects of the individual animal on the digestibility.

Digestibility is best measured *in vivo* in the rumen of a living ruminant, but
there exist various indirect methods with reliable conversions. For an overview
see \cite minson1990forage and \cite mcdonald2010animal. Formulas in the
megafauna model assume *in vivo* digestibility.

Ruminants typically lose a relatively constant fraction of about 19% of
digestible energy in urine and methane (López et al. 2000
\cite lopez2000prediction, McDonald et al. 2010 \cite mcdonald2010animal,
p. 258). The values for cattle and sheep are very similar here (McDonald et al.
2010, p. 260). McDonald et al. (2010, p. 258) specify that 11–13 percent of
digestible energy is lost as methane. The 19% loss to urine and gases is often
expressed as the ratio of metabolizable energy to digestible energy,
ME/DE=0.81. With a gross energy of about 19 MJ/kg, metabolizable energy in the
digestible fraction of the forage is then about 15–16 MJ/kg. Various herbivore
models work with these numbers, e.g.
Givens et al. (1989)\cite givens1989digestibility,
Illius and Gorden (1991)\cite illius1991prediction,
Parker et al. (1991)\cite parker1996foraging,
Illius and Gordon (1999)\cite illius1999scaling,
Smallegange and Brinsting (2002)\cite smallegange2002food.

A unitless cofficient **k** defines the efficiency of using the metabolizable
energy for meeting maintenance energy needs, i.e. for converting metabolizable
energy content to net energy content (**NE**) of the forage. Some livestock
models differentiate between different k values to reflect different conversion
efficiencies: for meeting maintenance needs (k<sub>m</sub>), for growth and
fattening (k<sub>f</sub>), and for lactation (k<sub>l</sub>) (Minson, 1990,
p. 151).

However, in the Modular Megafauna Model, the energy budget only calculates with
the “currency” NE. Therefore there is only one value k, which is functionally
equivalent to k<sub>m</sub> of other models. The energy lost when anabolising
fat is accounted for by an anabolism conversion coefficient, given in MJ/kg
fat: net energy needed to build up one kg body fat. For using fat reserves to
meet current energy needs fat is catabolised with a corresponding coefficient:
the net energy gained from burning one kg of body fat.

In summary, net energy content, NE in MJ/kgDM, depends on variable dry-matter
digestibility, DMD, as the key variable. It is calculated for livestock
ruminants, but can be adjusted with a species-specific, unitless factor, η. The
metabolizable energy, ME, depends on the gross energy, and digestible energy,
but for simplicity’s sake the user defines an empirical value, which usually
lies around 16 MJ/kgDM.

NE = ME * k * η = 16 MJ/kgDM * DMD * k * η

### Thermoregulation by Conductance {#sec_thermoregulation}

This model of thermoregulation is often called the **Scholander-Irving model** and was published in two seminal papers in 1950: \cite scholander1950adaptation \cite scholander1950heat.
The more detailed implementation is taken from Peters (1983)\cite peters1983ecological.

Homeothermic animals have extra energy costs to maintain their body core temperature.
Through basal metabolism and other ways of energy burning, heat is already passively created.
Thermoregulatory costs arise when the ambient temperature drops below the *lower critical temperature*: the passive heat from thermoneutral metabolism is not counterbalance heat loss to the environment.
The rate of heat loss depends on the *thermal conductance* of the whole animal (energy flow per temperature difference), which in turn depends on the *thermal conductivity* (energy flow per temperature difference and per thickness) of fur and skin and the body surface.
Conductance is the inverse of resistance or insulation, and conductivity is the inverse of resistivity.

- T<sub>crit</sub>: Lower critical temperature [°C].
- T<sub>core</sub>: Body core temperature [°C].
- T<sub>air</sub>: Ambient air temperature [°C].
- E<sub>neu</sub>: Thermoneutral metabolic rate [MJ/ind/day]
- C: Whole-body thermal conductance [W/ind].
- Φ: Heat loss [MJ/ind/day]

T<sub>crit</sub> = T<sub>core</sub> - E<sub>neu</sub> / C

Φ = C * max(T<sub>crit</sub> - T<sub>air</sub>, 0)

![](thermoregulation.png "Schematic description of the effects of external temperature on the metabolic rate in homeotherms. – Peters 1983, Fig. 5.6")

\note In its current form, the model only considers costs when temperatures are too low.
Overheating effects are not implemented since the model was developed with the focus on Arctic megafauna.

#### Conductance

The critical parameter for thermoregulatory expenditure is the (whole-body) conductance: the rate of heat flow per difference between core and air temperature (W/°C).
The conductance can be approximated from the average conductivity and the body surface.
Conductivity is the inverse of insulation: it is the heat flow per temperature difference per area.

Body surface in m² scales roughly as \f$0.09*M^{0.66}\f$ ([Hudson & White 1985](\cite hudson1985bioenergetics)).

### Foraging {#sec_foraging}

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

\todo
- What is the problem with β (half-max intake density) in Illius & O’Connor (2000) and Pachzelt et al.?

## Reproduction {#sec_reproduction}

## Mortality {#sec_mortality}

## Effects on Environment {#sec_effects_on_environment}

### Nitrogen Cycling {#sec_nitrogen_cycling}
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

## Population Dynamics {#sec_population_dynamics}
\todo
- Explain the drama of population fluctuations:
	+ What makes populations crash to zero?
	+ What’s the problem with annual allocation?
	+ What mechanisms have we explored to prevent population crashes?

### Minimum Density Threshold {#sec_minimum_density_threshold}
The parameter \ref Fauna::Hft::mortality_minimum_density_threshold defines at which point a dwindling population (sum of all cohorts/individuals) may be considered dead.
It is an arbitrary, but critical value for model performance.
Possible re-establishment only happens if all cohorts are dead within one habitat.

It is important to keep this parameter low enough for slow-breeding and long-lived animals because otherwise they may die out after establishment:
After establishment, the background mortality continually diminishes the adult cohorts, and after some years the total population (all cohorts together) my drop below the `minimum_density_threshold` before reproduction could compensate.

On the other hand, the `minimum_density_threshold` should not be set *too* low as this would result in extremely thin “ghost” populations that are effectively preventing re-establishment.

### Species Coexistence {#sec_coexistence}

The classical competitive exclusion principle predicts that no two species can coexist in the long term if they each solely depend on one shared resource (\cite hardin1960competitive).
One species will inevitably outcompete the other one.
Though there are indeed ecological mechanisms that can facilitate coexistence with a shared resource (\cite chesson2000mechanisms), the parameter space for this to happen in a model is usually very narrow (e.g. \cite vanlangevelde2008intantaneous).

In order to simply avoid competition among different HFTs, the option `one_hft_per_habitat` can be enabled: Each HFT exists on its own, without any interaction with other species.
With that option enabled, all HFTs should each be assigned to the same number of habitats.
It is the responsibility of the host application (the vegetation model) to ensure that the number of habitats is an integer multiple of the HFT count.

---

\author Wolfgang Traylor, Senckenberg BiK-F
\date 2019
\copyright ...
