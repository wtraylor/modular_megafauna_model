---
title: 'MMM: A C++ library for simulating large herbivores'
tags:
  - C++
  - DGVM
  - LPJ-GUSS
  - mammoth
  - megafauna
  - paleoecology
authors:
  - name: Traylor, Wolfgang
    orcid: 0000-0002-4813-1072
    affiliation: "1"
affiliations:
 - name: Senckenberg Biodiversity and Climate Research Centre, Frankfurt am Main, Germany
   index: 1
date: DAY May 2021 # TODO
bibliography: paper.bib
link-citations: true
---

<!--
SPDX-FileCopyrightText: 2021 Wolfgang Traylor <wolfgang.traylor@senckenberg.de>

SPDX-License-Identifier: CC-BY-4.0
-->

<!--
From the JOSS website (https://joss.readthedocs.io/en/latest/submitting.html):

The paper should be between 250-1000 words.

Your paper should include:

• A list of the authors of the software and their affiliations, using the correct format (see the example below).
• A summary describing the high-level functionality and purpose of the software for a diverse, non-specialist audience.
• A clear Statement of Need that illustrates the research purpose of the software.
• A list of key references, including to other software addressing related needs.
• Mention (if applicable) a representative set of past or ongoing research projects using the software and recent scholarly publications enabled by it.
• Acknowledgement of any financial support.
-->

# Summary

The Modular Megafauna Model (MMM) simulates populations of large, terrestrial herbivores (megafauna) through space and time.

Herbivore cohorts feed, grow, reproduce, and die in daily simulation cycles.
Based on these physiological processes herbivore numbers rise and fall dynamically.
The model thus does not prescribe carrying capacity, but instead simulates herbivore densities in a mechanistic, bottom-up approach.

# Statement of Need

Mechanistic modeling can help us understand the drivers behind real-world population dynamics.
Bottom-up models can point out which physiological processes are understudied and which mechanisms appear most important for emerging population effects [@deangelis2003praise].
Once a model is sufficiently mature, its predictions can inform nature conservation management—an application that could prove useful in light of ongoing global defaunation [@dirzo2014defaunation].

The currently implemented model concepts originate in large parts from @pachzelt2013coupling and the earlier works by @illius2000resource and @illius1999scaling.
@pachzelt2013coupling integrated African grazers into LPJ-GUESS, a dynamic global vegetation model (DGVM) [@smith2001representation].
Later studies have implemented conceptually similar grazer models for other DGVMs: @dangal2017integrating for DLEM and @zhu2018large for ORCHIDEE.
However, to my knowledge, none of these implementations are reusable across different vegetation models.

The author is currently using MMM integrated with LPJ-GUESS [@smith2001representation; @smith2014implications], using daily grass growth [@bokeolen2018estimating].[^lpj_guess]
Here, LPJ-GUESS simulates grass as forage for herbivores.
This particular project is concerned with simulating potential densities of mammoths, steppe bison, and horse in the last ice age.
However, MMM could also be parametrized for extant herbivores, such as cattle, Scandinavian reindeer, or Mongolian gazelles.
MMM can also be coupled with dynamic vegetation models other than LPJ-GUESS.

[^lpj_guess]:
The proprietary LPJ-GUESS source code, including the MMM-coupled version, may be available upon request at Lund University (<https://iis4.nateko.lu.se/lpj-guess/>).

# Features

MMM is a C++ library meant to be coupled with a dynamic vegetation model into a complete ecosystem model.
Currently, the only forage is grass.
The vegetation model simulates the amount of available forage, which the herbivores then consume.
In addition, the vegetation model provides information about environmental conditions, such as air temperature.
This way, herbivores and vegetation dynamically influence each other, namely through forage removal and nutrient cycling.
MMM ships with a very simple vegetation model that demonstrates how the coupling can be implemented.

Herbivores are simulated in distinct spatial units, which have no absolute area size because all calculations are done on a per-area basis.
That means that MMM itself is not spatially explicit and makes no assumptions about the actual size of the area inhabited by herbivores.
It is up to the vegetation model to give these spatial units meaning by linking them to spatially explicit entities, such as grid cells.
With this flexibility, MMM can be used for studies on different scales, from local to continental.

Modularity is a primary design goal of the library.
Through the instruction file, users can turn mechanisms on or off and parametrize herbivore species or herbivore functional types.
Parameters include, for example, body mass, components of energy expenditure, maximum feed intake, background mortality, body fat reserves, etc.
There are no hard-coded parameters; all can be defined in the instruction file.
MMM’s flexible framework allows developers to integrate new mechanisms, such as a more detailed energy budget model, mortality from hunting and predation, or a new forage type.

While monolithic ecosystem models can easily become “black boxes,” whose internal mechanisms have grown too complex to be understood intuitively, a modular model is more transparent.
Developing mechanistic ecosystem models is typically an exploratory, iterative process.
For a specific study, the modeler has to adjust parameters and mechanisms of a given model, either manually or programmatically.
In this process it is crucial that the modeler can increase model complexity step by step, just enough to represent the mechanisms important for the research question.

Thanks to its modular design and its stable library interface, MMM can stay backward-compatible in future versions, and the same codebase can be used for different studies and different vegetation models.
This benefits reproducibility in two ways.
First, after bugs have been fixed, previous analyses can easily be reexecuted.
Second, simulations can be repeated with other vegetation models in order to understand how their different assumptions impact plant–herbivore dynamics.

# Acknowledgements
I thank my PhD supervisor Thomas Hickler and my colleagues Adrian Pachzelt, Matthew Forrest, and Theresa Stratmann for their support in model development and implementation.

# References
