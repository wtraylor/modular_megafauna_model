<!--
SPDX-FileCopyrightText: 2020 Wolfgang Traylor <wolfgang.traylor@senckenberg.de>

SPDX-License-Identifier: CC-BY-4.0
-->

[![LGPL logo](docs/images/lgpl.svg)](https://choosealicense.com/licenses/lgpl-3.0/)
[![REUSE-compliant](docs/images/reuse-compliant.svg)][REUSE]
[![Documentation Status](https://readthedocs.org/projects/modular-megafauna-model/badge/?version=latest)](https://modular-megafauna-model.readthedocs.io/en/latest/?badge=latest)

[REUSE]: https://reuse.software

Modular Megafauna Model
=======================

Overview
--------

The Modular Megafauna Model (MMM) is a dynamic, mechanistic, and process-based abstraction of large herbivore populations through space and time.
Herbivores feed, grow, reproduce, and die on a daily simulation cycle.
The amount of forage as well as relevant environment variables need to be supplied by the calling program, for instance by a dynamic global vegetation model (DGVM).

The megafauna model itself is spatially inexplicit, but operates in different singular spatial units, which can be mapped to spatially meaningful cells by the calling framework.
Likewise, the megafauna model is ignorant of calendar dates, but is only aware of the Julian day of the year.

As the name suggests, modularity is a primary design goal.
Processes and model components can be switched on and off, replaced, and expanded.
Since this C++ library is Free Software, the scientific community is encouraged to use, study, change, and redistribute the source code.

Come join the MMM user and developer room on Matrix: <https://matrix.to/#/!rnevkLtJTORmvyzFHD:matrix.org?via=matrix.org>

Repository Structure
--------------------

This project follows the [Pitchfork Layout](https://github.com/vector-of-bool/pitchfork).

- `docs/`: Doxygen documentation.
- `examples/`: Exemplary instruction files.
- `external/`: Embedded external projects, which remain unmodified.
- `include/`: Public API header files.
- `LICENSES/`: Folder with licenses, compliant with [REUSE][].
- `src/`: Source and (private) header files of the project. Subdirectories correspond to C++ namespaces.
- `tests/`: Unit tests.
    - `tests/bad_insfiles/`: Example erroneous example files, each of which should throw an error.
- `tools/`: Different helper tools for the developer.

Usage
-----

The first step in working with the model is to create the documentation and read the introductory pages.
On the Quickstart page you will find instructions for compiling the source code.

The compiled documentation of the latest release is currently available for download as a ZIP file from Open Science Framework [here](https://osf.io/jzn5a/download).
As soon as this repository is public, the documentation will be available on [Read the Docs](http://readthedocs.io/) (issue [#22](https://github.com/wtraylor/modular_megafauna_model/issues/22)).

### Compile the Doxygen Documentation

As a bare minimum, you will need to have [CMake](https://cmake.org) (version 3.10 or higher) and [Doxygen](https://www.doxygen.nl) installed.

Open a Unix shell (terminal) in the root directory of the megafauna library, and execute the following lines.
On Windows, you can use the [Windows Subsystem for Linux]() <!--TODO-->(Windows 10 or higher) or try to compile it with the CMake GUI and/or an IDE.

```bash
mkdir build
cd build
cmake -DBUILD_DOC=ON ..
make megafauna_docs
```

Don’t worry if warning messages appear. Usually, most of the documentation
will be fine.
Now open the created file `docs/index.html` in a web browser.

#### Optional Build Requirements for the Documentation
- Java Runtime Environment (JRE) and [Graphviz](www.graphviz.org) to compile [PlantUML](http://plantuml.com) diagrams. See [here](http://plantuml.com/graphviz-dot) for details.
- [LaTeX](www.latex-project.org) to render mathematical formulas offline, and [BibTeX](www.bibtex.org) for the bibliography.

### Existing Integrations

Originally this megafauna model was developed for the dynamic global vegetation model [LPJ-GUESS](http://iis4.nateko.lu.se/lpj-guess/).
On the Lund subversion server there exists a branch `megafauna` that integrates this library into LPJ-GUESS.
LPJ-GUESS is proprietary and closed-source.
Please contact the maintainers of LPJ-GUESS to kindly ask for access.

Other dynamic vegetation models can include the megafauna model as an external library, too.
Learn more in the Doxygen documentation.

### Changing the Codebase

Flexibility and extensibility were high design goals for developing the megafauna model.
Hopefully you will find it possible to implement the necessary code changes/extensions for your particular research questions.
You will need basic skills with Git and C++ (C++11 standard) in order to contribute.

On the index/main page of the Doxygen documentation you will be directed to the resources you need to contribute.
Please also read through the file [`CONTRIBUTING.md`][].

Note that for running the model, you don’t need to change the source code.
Most parameters can be set in the instruction file.

Known Bugs and Issues
---------------------

Known bugs and improvements are collected in the [GitHub issue tracker](https://github.com/wtraylor/modular_megafauna_model/issues/).
If you discover a new bug, please use the issue tracker to [report](https://github.com/wtraylor/modular_megafauna_model/issues/new) it.
If you can fix it yourself, fork this repository, fix the bug, and request a pull into the main repository.
Compare [Understanding the GitHub flow](https://guides.github.com/introduction/flow/).

Authors
-------

- Wolfgang Traylor (wolfgang.traylor@senckenberg.de) ![ORCID][orcid-logo] <https://orcid.org/0000-0002-4813-1072>, Senckenberg Biodiversity and Climate Research Centre ([SBiK-F][])

[orcid-logo]: <https://info.orcid.org/wp-content/uploads/2020/12/ORCIDiD_icon16x16.png>
[SBiK-F]: <https://www.senckenberg.de/en/institutes/sbik-f/>

Similar Projects
----------------

- Dangal, Shree R. S., Hanqin Tian, Chaoqun Lu, Wei Ren, Shufen Pan, Jia Yang, Nicola Di Cosmo, and Amy Hessl. 2017. “Integrating Herbivore Population Dynamics into a Global Land Biosphere Model: Plugging Animals into the Earth System.” Journal of Advances in Modeling Earth Systems 9 (8): 2920–45. <https://doi.org/10.1002/2016MS000904>.
- Illius, A. W., and T. G. O’Connor. 2000. “Resource Heterogeneity and Ungulate Population Dynamics.” Oikos 89 (2): 283–94. <https://doi.org/10.1034/j.1600-0706.2000.890209.x>.
- Pachzelt, Adrian, Anja Rammig, Steven Higgins, and Thomas Hickler. 2013. “Coupling a Physiological Grazer Population Model with a Generalized Model for Vegetation Dynamics.” Ecological Modelling 263: 92–102. <https://doi.org/http://dx.doi.org/10.1016/j.ecolmodel.2013.04.025>.
- Zhu, Dan, Philippe Ciais, Jinfeng Chang, Gerhard Krinner, Shushi Peng, Nicolas Viovy, Josep Peñuelas, and Sergey Zimov. 2018. “The Large Mean Body Size of Mammalian Herbivores Explains the Productivity Paradox During the Last Glacial Maximum.” Nature Ecology & Evolution. <https://doi.org/10.1038/s41559-018-0481-y>.

License
-------

- This project follows the [REUSE][] standard:
    - Every file has its copyright/license information either in a comment at the top or in a separate text file with the extension `.license`.
    - All license texts can be found in the directory `LICENSES/`.
    - Project information and licenses for Git submodules can be found in the text file `.reuse/dep5`.
- The Modular Megafauna Model is Free Software under the [GNU Lesser General Public License v3.0 or later][lgpl].
- The software documentation, the accompanying images, and configuration files are licensed under the [Creative Commons Attribution 4.0 International][cc-by-4.0].

[cc-by-4.0]: https://creativecommons.org/licenses/by/4.0
[lgpl]: https://www.gnu.org/licenses/lgpl-3.0-standalone.html

### Third-Party Work

- The [Catch2](https://github.com/catchorg/Catch2) test framework (`tests/catch.hpp`) is licensed under the [Boost Software License](http://www.boost.org/LICENSE_1_0.txt).

- The version of the [PlantUML](http://plantuml.com) file (`docs/plantuml.jar`), which is used to render UML diagrams in the Doxygen documentation, is under the [MIT license](http://opensource.org/licenses/MIT).

- The [cpptoml](https://github.com/skystrife/cpptoml) library (`external/cpptoml/`) is licensed under the [MIT license](http://opensource.org/licenses/MIT).
