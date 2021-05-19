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

The Modular Megafauna Model (MMM) simulates populations of large, terrestrial herbivores (“megafauna”) through space and time.
Herbivores feed, grow, reproduce, and die in daily simulation cycles.
These lower-level mechanisms let the herbivore numbers dynamically rise and fall.
The model thus does not prescribe carrying capacity, but instead simulates herbivore densities in a mechanistic, bottom-up approach.

Mechanistic modeling can help us understand the drivers behind real-world population dynamics.
Bottom-up models can point out which physiological processes are understudied and which lower-level mechanisms appear most important for higher-level population effects [@deangelis2003praise].
Once a model is sufficiently matured, its predictions can inform nature conservation management—an application that is urgently needed in light of ongoing global defaunation [@dirzo2014defaunation].

The currently implemented model concepts originate in large parts from @pachzelt2013coupling and the earlier works by @illius2000resource and @illius1999scaling.
While @pachzelt2013coupling integrated African herbivores into LPJ-GUESS, a dynamic global vegetation model (DGVM), later studies have implemented conceptually similar herbivore models for other DGVMs: @dangal2017integrating for DLEM and @zhu2018large for ORCHIDEE.
To my knowledge, none of these implementations is reusable across different DGVMs, though.

MMM is a C++ library that is supposed to be coupled with a dynamic vegetation model into a complete ecosystem model.
The vegetation model provides forage, which the herbivores consume, and information about environmental conditions like air temperature.
This way, herbivores and vegetation dynamically influence each other, namely through forage removal and nutrient cycling.

Herbivores are simulated in distinct spatial units, which have no absolute area size because all calculations are done on a per-area basis.
That means that MMM itself is not spatially explicit and makes no assumptions about the actual size of the area inhabited by herbivores.
It is up to the vegetation model to give these spatial units meaning by linking them to spatially explicit entities like grid cells.
With this flexibility, MMM can be used for studies on different scales, from local to continental.

Modularity is a primary design goal of the library.
Through the instruction file, users can turn mechanisms on or off and parametrize herbivore species or herbivore functional types.
There are no hard-coded parameters; all are exposed in the instruction file.
MMM’s flexible framework allows developers to easily implement new mechanisms, for example a more detailed energy budget model or mortality from hunting or predation.

Developing mechanistic ecosystem models is typically an exploratory, iterative process.
For a specific study, the modeler has to adjust parameters and mechanisms of a given model, either manually or programmatically.
In this process it is crucial to be able to move step-by-step from a simple to a more complex model design, as necessary.
While monolithic ecosystem models can easily become “black boxes,” whose the internal mechanisms have grown too complex to be understood intuitively, a modular model is more transparent.
The modeler can configure the appropriate model complexity for the question at hand and this way learns about which mechanisms are likely driving the real-world ecosystem under study.

Thanks to its modular design and its stable library interface, MMM can stay backward-compatible in future versions, and the same codebase can be used for different studies and different vegetation models.
That benefits reproducibility in two ways.
First, after bugs have been fixed, previous analyses can easily be reexecuted.
Second, simulations can be repeated with other vegetation models in order to understand how their different assumptions impact plant–herbivore dynamics.

At this point, MMM is being used by the author to simulate potential densities of mammoths in the last ice age.
Here, MMM is coupled with the global dynamic vegetation model LPJ-GUESS [@smith2001representation, @smith2014foundations, @bokeolen2018estimating].

# Acknowledgements
I thank my PhD supervisor Thomas Hickler and my colleagues Adrian Pachzelt, Matthew Forrest, and Theresa Stratmann for their support in model development and implementation.

# References
