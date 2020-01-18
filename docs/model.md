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

## Symbols and Abbreviations

- bf  = Current body fat as fraction of lipids per empty body [frac.]
- BM  = Live body mass [kg/ind]
- DM  = Dry matter
- DMD = Dry-matter digestibility [frac.]
- eb  = Empty body fraction [frac.]
- FM  = Fat mass of an individual [kg/ind]
- GE  = Gross energy, also known as heat of combustion or calorific value [MJ/kgDM]
- k   = Net energy coefficient, efficiency of converting ME to usable energy
    - Subscript f = conversion to fat gross energy
    - Subscript m = conversion to NE, to meet needs of maintenance and field metabolic rate
    - Subscript p = conversion to protein gross energy
- ME  = Metabolizable energy content in forage [MJ/kgDM]
- MRT = Mean retention time [hours]
- NE  = Net energy content in forage, usable for meeting energy requirements [MJ/kgDM]
- SM  = Structural mass [kg/ind]
- General subscripts:
    - ad    = Adult
    - birth = At birth/for neonates
    - max   = Maximum (body fat, fat mass, reproduction rate, …)

Note that “mass” and “weight” are used interchangeably.

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

Offspring of large herbivores usually shows an even sex ratio. Most model
processes don’t differentiate between males and females, only body size and age
of maturity have sex-specific parameters. It seems important to at least set
the basis for gender differentiation because some large herbivores do show
pronounced sexual dimorphism not only in size (e.g. bison or proboscideans) but
also in diet and behavior (e.g. elephants: \cite shannon2013diet).

### Body Mass and Composition {#sec_body_mass_and_composition}

The user-defined **live body mass** of simulated herbivores is the sum of
blood, gut contents (ingesta), structural (fat-free) mass and deposited body
fat. It is very important to realize that the **body fat** that the model works
with is the fraction of fat in the empty body. The **empty body mass** is the
live body mass minus blood, ingesta, hair, and antlers/horns. The body fat is
total lipid content, which is also known as ether extract, free lipid content,
or crude fat (Hyvönen, 1996 \cite hyvonen1996approach). This is different from
the mass of suet and organ fat because the fat tissue also contains water. The
term **lean body mass** is live body mass minus all fat mass (compare quote
from Blaxter, 1989 \cite blaxter1989energy in Section
\ref sec_fat_as_energy_storage). Note that lean body mass includes digesta,
hair, antlers, and blood.

![Body composition in the Modular Megafauna Model.](images/body_composition.svg)

Calder (1996, p. 14) \cite calder1996function discusses the question of
variable ingesta load:

> Should the total body mass used in allometry include gut contents, a major
> source of variability (but representing mass that the animal must be designed
> to support and carry), or should gut contents be subtracted from live mass?
> Disallowing gut contents is not practical in studies wherein the animals are
> not, or should not be sacrificed.

In this line of argument, the gut contents are always included the body mass
values given in the megafauna model. It is designed for large herbivores, in
particular extinct ones, and their body mass is most commonly given as a total
live weight. Live body mass is easy to measure. Most allometric regressions are
based on it.

Technically, the empty body mass is different from the “ingesta-free mass” in
the literature because ingesta-free mass usually includes hair. For less furry
animals the two can be considered approximately equal, though.

### Fat as Energy Storage {#sec_fat_as_energy_storage}

The variable amount of body fat, which serves as energy reserves, is a critical
component of the herbivore simulations. As Blaxter (1989, p. 51)
\cite blaxter1989energy explains, the ingesta-free animal body can be viewed as
composed of fat and fat-free mass:

> Schematically the body can be regarded as consisting of two components – fat
> and non-fat. […] The non-fat material consists of water, the minerals of bone
> and soft tissue, carbohydrate, nitrogen-containing compounds, and, in the
> living animal, the contents of the digestive tract. The non-fat component of
> the body is usually referred to as the *lean body mass* or *fat-free mass.*
> Many studies, commencing with those of Murray (1922) and embracing a wide
> range of adult species, have shown that the chemical composition of the
> fat-free body is approximately constant. The wide range of composition of
> animals is largely, but not entirely, due to variation in the proportion of
> fat.

When defining the fractional body fat parameter for an herbivore, you should
not rely on measurements of weight loss of starving or fattening animals. In
such data it is difficult to disentangle the contributions of changing fat
mass, gut contents, water content, and fat-free mass (e.g. Reimers et al., 1982
\cite reimers1982body).

The model ignores the contribution of catabolizing protein altogether. This is
a simplification as several studies have shown that the contribution of
mobilized protein can play a considerable role in meeting energy requirements.
Reimers et al. (1982, pp. 1813, 1819) \cite reimers1982body observe that
reindeer lost about 31% of their body protein during winter. Torbit et al.
(1988) \cite torbit1988calibration calculate and discuss the energetic value of
catabolizing protein. Parker et al. (1993) \cite parker1993seasonal note that
Sitka black-tailed deer lost 10–15% of their protein reserves during winter.
Nontheless fat is unquestionably by far the most important energy reserve.
Fluctuating body fat is convenient to model as an energy pool that can be
filled to a maximum and emptied to zero. The interactions for protein synthesis
and depletion are far more complex and less studied.

Any forage energy that is ingested beyond maintenance needs is converted to
body fat. Section \ref sec_energy_content details the efficiency of fat
anabolism. When forage intake is not enough to meet energy needs, fat reserves
are catabolized, and the energy is directly available as net energy to balance
any energy “debts” on a daily basis.
Illius & O’Connor (2000) \cite illius2000resource assumed an efficiency of 100%
to mobilize fat reserves in cattle when they directly converted the combustion
(gross) energy of fat tissue to net energy.
Armstrong & Robertson (2000) \cite armstrong2000energetics use a factor of 80%
in their sheep model, citing a 1990 publication by the Australian Standing
Committee on Agriculture (SCA) \cite corbett1990feeding.
The megafauna model allows the catabolism efficiency to be specified by the
user. This efficiency factor is multiplied with the fat gross energy to derive
the net energy gain (MJ) from burning one kg of body fat.

### Ontogenetic Growth

\todo
- What does the growth curve look like?
    - Compare price1985growth [pp. 187-190] and blaxter1989energy
      [p. 242-244].

\todo Explain how growth is linear in \ref Fauna::HerbivoreBase::get_bodymass()

Blaxter (1989)\cite blaxter1989energy, p. 242f:

> Growth in weight is characteristically sigmoid; it accelerates during a short
> initial period and then declines until, as maturity approaches, it approaches
> zero. A large number of different functions have been used to describe this
> relationship between weight and time. Virtually all of them state that dW/dt,
> the rate of change in weight with time (or *rate of growth*) is a function of
> weight at the time that dW/dt is measured. Such functions include the
> logistic equation, the Gompertz function and the Bertalanffy function. These
> are derived algebraically in most textbooks of biomathematics (see Causto
> 1977). They were generalised by F.J. Richards (1959) and have been reviewed
> and critically analysed by Parts (1982).

## Energy Household {#sec_energy_household}

![Energy household for a ruminant or hindgut fermenter. Modified after Minson (1990), Fig. 5.1.](images/energy_household.svg)

### Energy Content of Forage {#sec_energy_content}

The model for energy content in herbivore forage presented here is based on the
partitioning of metabolizable energy. A historical overview of the model
framework is given by Ferrell & Oltjen (2008) \cite ferrell2008asas. Its
conceptual shortcomings and difficulties in practical methodology are
summarized by Birkett & de Lange (2001) \cite birkett2001limitations.

The diagram on the energy household shows how energy from the forage is used by
an herbivore: **Gross energy** (GE) is the heat that could be produced from
complete combustion of the feedstuff. From that, the part which is not excreted
in feces is the **digestible energy** (DE). Some proportion of it is then lost
to urine and gas production, but the rest is **metabolizable energy** (ME).
After deducing now the losses due to heat increment, the remaining **net
energy** (NE) is effectively utilizable for all physiological processes.

Gross energy depends only on the physical properties of the forage and measured
in a combustion chamber. It is therefore independent of the animal. McDonald et
al. (2010, p. 259)\cite mcdonald2010animal provide an overview of gross energy
in different feedstuffs for livestock: It typically ranges between 18 to 20
MJ/kgDM. The measurements by Golley (1961) \cite golley1961energy suggest that
there is some seasonal variation in gross energy of leaves. However, the model
assumes a constant value.

The proportional **dry-matter digestibility** (DMD) of the forage is a central
variable in the model. It measures the fraction of the gross energy that is
usable by the animal. The rest gets excreted in the feces because it is
undigestible fiber: protected cellulose and hemicellulose, silica, and cutin.
Agricultural research has shown that the digestibility is closely correlated
with metabolizable energy and net energy (Minson, 1990, p. 7
\cite minson1990forage). Therefore digestibility is modeled as the *one*
indicator for forage quality, i.e. forage energy density, and must be given by
the vegetation model. This neglects any other effects on the digestibility,
like interactions of different forages or effects of the individual animal on
the digestibility. Digestibility is best measured *in vivo* in the rumen of a
living ruminant, but there exist various indirect methods with reliable
conversions. For an overview see Minson (1990) \cite minson1990forage and
McDonald (2010) \cite mcdonald2010animal. Formulas in the megafauna model
assume *in vivo* digestibility.

Research has focused mainly on the digestion of ruminant livestock, and so the
megafauna model works primarily with the well-established formulas for
ruminants. Digestibility is also defined for ruminants. To account for the less
efficient digestion of hindgut fermenters, the user can define a
**digestibility multiplier** to convert ruminant digestibility to hindgut
digestibility. This approach is taken from by  and Pachzelt et al. (2015)
\cite pachzelt2015potential, who cite Illius & Gordon (1992)
\cite illius1992modelling

Ruminants typically lose a relatively constant fraction of about 19% of
digestible energy in urine and methane (López et al. 2000
\cite lopez2000prediction, McDonald et al. 2010 \cite mcdonald2010animal,
p. 258). The difference between cattle and sheep is very small here (McDonald
et al. 2010, p. 260). McDonald et al. (2010, p. 258) specify that 11–13 percent
of digestible energy is lost as methane. The 19% loss to urine and gases is
often expressed as the ratio of metabolizable energy to digestible energy,
ME/DE=0.81. This ratio is also known as the **metabolizable energy
coefficient** (e.g. in Robbins, 1983\cite robbins1983wildlife).With a gross
energy of about 19 MJ/kg, metabolizable energy in the digestible fraction of
the forage is then about 15–16 MJ/kg. Various herbivore models work with these
numbers, for instance: Givens et al. (1989)\cite givens1989digestibility,
Illius and Gorden (1991)\cite illius1991prediction, Parker et al. (1991)
\cite parker1996foraging, Illius and Gordon (1999)\cite illius1999scaling,
Smallegange and Brinsting (2002)\cite smallegange2002food.

\warning Some publication, like Minson (1990)\cite minson1990forage, use the
term “metabolizability of energy” or “metabolizable energy coefficient” to
refer to the ME/GE ratio: the metabolizable fraction of the *gross* energy.
This includes fecal losses and has the dry-matter digestibility already
calculated in. However, the modular megafauna model works with explicit
digestibility values and the ME/DE ratio. You could divide the ME/GE ratio by
the fractional digestibility to get ME/DE.

A unitless **net energy coefficient** (k) defines the efficiency of using the
metabolizable energy for meeting maintenance energy needs, i.e. for converting
metabolizable energy content to **net energy** content (NE) of the forage. (In
Robbins (1983)\cite robbins1983wildlife it is called *NEC*.) Many livestock
models differentiate between different k values to reflect different conversion
efficiencies: for meeting maintenance needs (k<sub>m</sub>), for growth and
fattening (k<sub>f</sub>), and for lactation (k<sub>l</sub>) (Blaxter 1989,
p. 254ff\cite blaxter1989energy; Minson 1990\cite minson1990forage, p. 151).

In the Modular Megafauna Model, the energy budget calculates with the
“currency” net energy, which broadly represents the available oxidizable
metabolic fuels—glucose and fatty acid. Basal and field metabolic rate and
other energy expenditures are directly “paid” with net energy. Therefore the
efficiency factor k<sub>m</sub> is used to convert from metabolizable energy to
net energy. Body fat is anabolized from metabolizable forage energy with the
efficiency factor k<sub>f</sub>.

![Model of energy retention in an herbivore. km and kf denote the slope of the line, i.e. the efficiency of utilizing metabolizable energy. When fed maintenance requirements, the animal will neither gain nor lose weight. Below that point it will starve (i.e. catabolize reserves) and above it will build reserves (i.e. anabolize fat). After McDonald et al. (2010), Fig. 11.5.](images/retention_over_intake.svg)

Feeding trials have shown that the net energy coefficient can linearly depend
on the metabolizable energy content of the forage (Robbins, 1983, p. 296f;
Minson, 1990, p. 93, 155). However, this effect seems to be mostly related to
very high levels of feeding and by pelleting the feed. In this model, the
energy coefficients k<sub>m</sub> and k<sub>f</sub> are assumed to be constant.

\remark
Internally the model converts first from metabolizable energy to net energy to
pay energy expenditures. If there is excess net energy, this gets converted
*afterwards* to body fat. The amount of net energy required to build up one
kilogram of body fat is given by the product of fat gross energy content,
k<sub>m</sub> and k<sub>f</sub>⁻¹. Note that Illius & O’Connor (2000)
\cite illius2000resource probably took the same approach when they specify an
anabolism coefficient of 54.6 MJ/kg, citing Blaxter (1989)
\cite blaxter1989energy. Namely, 54.6 MJ/kg is the product of 39 MJ/kg,
k<sub>m</sub>=0.70, and the inverse of k<sub>f</sub>=0.50; probably Illius &
O’Connor (2000) took the latter two figures from Table 12.1 on page 259 in
Blaxter (1989) for oxen on an “average diet.”

In summary: Net energy content, NE in MJ/kgDM, depends on variable dry-matter
digestibility, DMD, as the key variable. Gross energy content, GE, is
user-specified for each forage type. Only the digestible fraction of the gross
energy in dry matter is counted as digestible energy, DE. How much
metabolizable energy can be extracted from the digested part of the forage is
species-specific and defined by the user as the metabolizable energy
coefficient or ME/DE ratio. A user-defined factor, k<sub>m</sub>, defines how
efficient the metabolizable energy is used to meet net energy needs for
maintenance and other activities. The factor k<sub>f</sub> denotes the
efficiency for converting from ME to body fat (anabolism), but burning of fat
reserves happens without energy loss. The net energy content is given by:

NE = ME * k<sub>m</sub> = DE * ME/DE * k<sub>m</sub> = GE * DMD * ME/DE * k<sub>m</sub>

### Maximum Daily Forage Intake {#sec_daily_forage_intake}

### Death of Starvation

In the process of starvation, different fat depots are mobilized in a typical
sequence: rump fat, subcutaneous fat, visceral fat, and, finally, marrow fat
(Hanks, 2004/1981 \cite hanks2004characterization). When the energy reserves of
an animal are exhausted, it will die of starvation. Body fat, i.e. lipid in the
ingesta-free body, is then zero. Different studies have found that the
carcasses of large herbivores that have starved to death contain virtually no
body fat anymore (Reimers et al., 1982 \cite reimers1982body; Depperschmidt et
al., 1987 \cite depperschmidt1987body), but chemical analysis of fat content in
carcass samples can be imprecise (Depperschmidt et al., 1987).

### Thermoregulation by Conductance {#sec_thermoregulation}

This model of thermoregulation is often called the **Scholander-Irving model**
and was published in two seminal papers in 1950:
\cite scholander1950adaptation \cite scholander1950heat. The more detailed
implementation is taken from Peters (1983) \cite peters1983ecological.

Homeothermic animals have extra energy costs to maintain their body core
temperature. Through basal metabolism and other ways of energy burning, heat is
already passively created. Thermoregulatory costs arise when the ambient
temperature drops below the *lower critical temperature*: the passive heat from
thermoneutral metabolism is not counterbalance heat loss to the environment.
The rate of heat loss depends on the *thermal conductance* of the whole animal
(energy flow per temperature difference), which in turn depends on the *thermal
conductivity* (energy flow per temperature difference and per thickness) of fur
and skin and the body surface. Conductance is the inverse of resistance or
insulation, and conductivity is the inverse of resistivity.

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

Body surface in m² scales roughly as \f$0.09*BM^{0.66}\f$ ([Hudson & White 1985](\cite hudson1985bioenergetics)).

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

New herbivore offspring is created in the model every year in the **breeding
season.** This term refers to the time of parturition, not the time of mating.
The onset and the length of the breeding season is defined with constant
parameters by the user. A flexible breeding season depending on the primary
production like in Pachzelt et al. (2013) \cite pachzelt2013coupling is
currently not implemented.

The user-selected **reproduction model** defines how many young a female will
produce on average per year, i.e. the probability of producing one newborn per
year. The **logistic** reproduction model correlates this probability with the
body fat at the time of conception.

\remark The *lipostatic model of fertility* states that the estrus is
suppressed below a critical body fat level. It was originally formulated for
humans by Frisch & McArthur (1974) \cite frisch1974menstrual. The hypothesis
was heavily criticized by Bronson & Manning (1991) \cite bronson1991energetic
and Wade & Schneider (1992) \cite wade1992metabolic. Even though they concede
that there are *correlations,* they argue that there is *no causal connection*
between body fat and ovulation and that the momentary energy supply in form of
metabolic fuels is directly or indirectly (e.g. through insulin) regulating
ovulation. This is the *metabolic hypothesis.* The discovery of the hormone
leptin in 1994, which is produced by adipose cells, made it again conceivable
that there might potentially be a direct link between body fat and reproductive
functions. However, the hormonal interactions are too complex to decipher the
underlying mechanisms leading to suppression of ovulation or altered sexual
behavior (for reviews see Friggens et al. (2010) \cite friggens2010nutritional
and Schneider et al. (2012) \cite schneider2012sense). Therefore the herbivore
model works with correlations only.

In some publications, e.g. Cook et al. (2004) \cite cook2004effects or
Tollefson et al. (2010) \cite tollefson2010influence, the probability of
pregnancy, as a dependent of percentage of total body fat, is expressed in a
logistic function of this form:

\f[
y = \frac{e^{a+b*x}}{x + e^{a+b*x}}
\f]

This can be simplified to:

\f[
y = \frac{x}{x + e^{-(a+b*x)}}
\f]

Illius & O’Connor (2000) \cite illius2000resource show their breeding rate
model in Formula (2). It has more variables, but comes down to the same formula
as above. Instead of total body fat percentage they use **body condition,**
which is defined as the ratio of current fat mass, FM, to the maximum fat mass,
$FM_{max}$:

\f[
y = \frac{1}{1 + e^{-b(FM/FM_{max} - c)}}
\f]

The exponent makes the parameters b and c explicit. Parameter *c* is called the
**midpoint** value of a generalized logistic function. It is the body condition
at which 50% of the maximum annual growth rate is reached. Parameter *b* is
called **growth rate** and defines the slope of the curve—the softness of the
threshold, one could say.

![Logistic model of correlating number of offspring in the breeding season with body condition of females in the mating season.](images/reproduction_illius2000resource.png)

Parametrizing the logistic reproduction model based on field data is difficult
for the following reasons:

1. Field studies correlating body fat in the mating season with breeding
   success are rare.
1. The regressions don’t always have good predictive value (e.g.
   Garrott et al., 2003 \cite garrott2003climateinduced;
   Tollefson et al., 2010 \cite tollefson2010influence).
1. The absolute or relative body fat values from field studies are difficult to
   translate to the “body condition” of
   Illius & O’Connor (2000) \cite illius2000resource, i.e. fraction of maximum
   fat mass. Not only is total body fat hard to measure in the first place (and
   only indirectly), but also the maximum body fat needs to be defined as well.
1. Each large herbivore species has a unique response to body fat during mating
   season, depending on their reproductive strategy (Adamczewski et al.,
   1998 \cite adamczewski1998influence). It might not be possible
   to transfer results from one species to another.

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
  N_{bound} = N_{guts} + N_{body} = I_N * MRT * P + 0.03 * BM * P
\f]

\f$P\f$ is the population density (ind/km²) and \f$BM\f$ is the body mass (kg/ind).
Mean retention time in hours is calculated according to Clauss et al. (2007)\cite clauss2007case, Fig. 2:

\f[
  MRT = 32.8 * BM^{0.07}
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
