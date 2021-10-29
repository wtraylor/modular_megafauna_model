# Recipes {#page_recipes}
<!--
SPDX-FileCopyrightText: 2020 Wolfgang Traylor <wolfgang.traylor@senckenberg.de>

SPDX-License-Identifier: CC-BY-4.0
-->

\brief Instruction file snippets and templates.

\tableofcontents

## Constant Population {#sec_recipe_constant_population}

Prescribing a constant herbivore density to the model may be used to simulate the effects of different stocking regimes.
Reproduction should then be disabled.
The herbivores may be immortal so that the stocking rate remains constant even if the animals are starving.

This is a minimal example herbivore instruction file for cattle:

```toml
[[hft]]
name = "Cattle"
[hft.body_fat]
deviation          = 0.125 # body condition
maximum            = 0.3   # kg/kg
maximum_daily_gain = 0.0   # kg/kg/day
[hft.body_mass]
female = 400 # kg
male   = 400 # kg
[hft.digestion]
type = "Ruminant"
limit = "IlliusGordon1992"
[hft.establishment]
age_range = { first = 1, last = 1 } # years
density   = 1.0 # ind/km²
[hft.expenditure]
components = [ "Taylor1981" ]
[hft.foraging]
diet_composer           = "PureGrazer"
limits                  = [ "IlliusOConnor2000" ]
half_max_intake_density = 40 # gDM/m²
[hft.reproduction]
model = "None"
```

The parameter values are mostly taken from Illius & O’Connor (2000)\cite illius2000resource.

With this instruction file, one immortal cohort of adult body mass will be established.
The mass density may fluctuate because of fat mass, but individual density will remain constant.

The `establishment.density` is probably the most crucial parameter in this setup.
Currently, the density can only prescribed globally and is constant.

-------------------------------------------------

\copyright <a rel="license" href="http://creativecommons.org/licenses/by/4.0/"><img alt="Creative Commons License" style="border-width:0" src="https://i.creativecommons.org/l/by/4.0/80x15.png" /></a> This software documentation is licensed under a <a rel="license" href="http://creativecommons.org/licenses/by/4.0/">Creative Commons Attribution 4.0 International License</a>.
\author Wolfgang Traylor, Senckenberg BiK-F
\date 2019
