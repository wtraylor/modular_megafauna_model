# Demo Simulator {#page_demo_simulator}
<!--
SPDX-FileCopyrightText: 2020 Wolfgang Traylor <wolfgang.traylor@senckenberg.de>

SPDX-License-Identifier: CC-BY-4.0
-->

\brief Introduction to the demo vegetation model that uses the megafauna library.

\tableofcontents

## Overview

The demo simulator shall demonstrate how to integrate the megafauna library.
Moreover, it serves as a testing framework to run the megafauna model with as little overhead as possible and in a controlled environment.

The demo simulator classes are all in the namespace \ref Fauna::Demo.
The central class running the program is \ref Fauna::Demo::Framework.
It employs \ref Fauna::World to execute the megafauna simulation.

The class \ref Fauna::Demo::SimpleHabitat implements a very basic vegetation model that can be parametrized with custom parameters in the instruction file.
Only this one kind of vegetation model is implemented.
Grass growth with a logistic growth function:

\image html images/logistic_growth.svg "Logistic growth of demo grass model."

The `SimpleHabitat` class corresponds to the LPJ-GUESS `Patch`.

Each “habitat group” can be considered a list of \ref Fauna::Demo::SimpleHabitat objects.
The “habitat group” corresponds conceptually to the LPJ-GUESS `Gridcell`.

Each aggregation unit (habitat group) comprises several habitats.
Since all habitats have the same properties and there is no stochasticity, the output from all aggregation units will look the same.

@startuml "Class diagram of the megafauna demo simulator."
	!include diagrams.iuml!demo_simulator_classes
@enduml

### Parameters

An example instruction file is provided in `examples/demo_simulation.toml`.
It is completely separate from the megafauna library instruction file,
It emulates the scenario of the metaphysiological model by Norman Owen-Smith \cite owensmith2002metaphysiological during growing season.

-------------------------------------------------

\copyright <a rel="license" href="http://creativecommons.org/licenses/by/4.0/"><img alt="Creative Commons License" style="border-width:0" src="https://i.creativecommons.org/l/by/4.0/80x15.png" /></a> This software documentation is licensed under a <a rel="license" href="http://creativecommons.org/licenses/by/4.0/">Creative Commons Attribution 4.0 International License</a>.
\author Wolfgang Traylor, Senckenberg BiK-F
\date 2019
