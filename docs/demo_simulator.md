<!--
SPDX-FileCopyrightText: 2020 Wolfgang Traylor <wolfgang.traylor@senckenberg.de>

SPDX-License-Identifier: CC-BY-4.0
-->

# Demo Simulator {#page_demo_simulator}
\brief Introduction to the demo vegetation model that uses the megafauna library.

\tableofcontents

## Overview

The demo simulator shall demonstrate how to integrate the megafauna library.
Moreover, it serves as a testing framework to run the megafauna model with as little overhead as possible and in a controlled environment.

The demo simulator classes are all in the namespace \ref Fauna::Demo.
The central class running the program is \ref Fauna::Demo::Framework.
It employs \ref Fauna::World to execute the megafauna simulation.

The class \ref Fauna::Demo::SimpleHabitat implements a very basic vegetation model that can be parametrized with custom parameters in the instruction file.
The `SimpleHabitat` corresponds to the LPJ-GUESS `Patch`.
Only this one kind of vegetation model is implemented.

Each “habitat group” can be considered a list of \ref Fauna::Demo::SimpleHabitat objects.
The “habitat group” corresponds conceptually to the LPJ-GUESS `Gridcell`.

@startuml "Class diagram of the megafauna demo simulator."
	!include diagrams.iuml!demo_simulator_classes
@enduml

### Parameters

An example instruction file is provided in `examples/demo_simulation.toml`.
It is completely separate from the megafauna library instruction file,
It emulates the scenario of the metaphysiological model by Norman Owen-Smith \cite owensmith2002metaphysiological during growing season.
