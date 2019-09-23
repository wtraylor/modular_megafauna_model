Modular Megafauna Model
=======================

Overview
--------

The modular megafauna model (M³ or “Triple-M”) is a dynamic, mechanistic, and process-based abstraction of large herbivore populations through space and time.
Herbivores feed, grow, reproduce, and die on a daily simulation cycle.
The amount of forage as well as relevant environment variables need to be supplied by the calling program, for instance by a dynamic global vegetation model (DGVM).

The megafauna model itself is spatially inexplicit, but operates in different singular spatial units, which can be mapped to spatially meaningful cells by the calling framework.
Likewise, the megafauna model is ignorant of calendar dates, but is only aware of the Julian day of the year.

As the name suggests, modularity is a primary design goal.
Processes and model components can be switched on and off, replaced, and expanded.
Since this C++ library is Free Software, the scientific community is encouraged to use, study, change, and redistribute the source code.

<!--TODO: Is the name “Triple-M” or M³ appropriate?-->
<!--TODO: Is Free Software applicable after the license is chosen?-->

Repository Structure
--------------------

This project follows the [Pitchfork Layout](https://github.com/vector-of-bool/pitchfork).

- `docs/`: Doxygen documentation.
- `examples/`: Exemplary instruction files.
- `external/`: Embedded external projects, which remain unmodified.
- `include/`: Public API header files.
- `src/`: Source and (private) header files of the project. Subdirectories correspond to C++ namespaces.
- `tests/`: Unit tests.
- `tools/`: A dummy vegetation simulator that integrates the megafauna model.

Usage
-----

The first step in working with the model is to create the documentation and read the introductory pages.
On the Quickstart page you will find instructions for compiling the source code.

If you don’t want or are unable to compile the documentation, you can also open the file [docs/quickstart.md](docs/quickstart.md) and the other markdown files in `docs/` directly.

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
Please contact the maintainers of LPJ-GUESS to kindly ask for access.

Other dynamic vegetation models can include the megafauna model as an external library, too.
Have a look at the demo simulator application in `tools/demo_simulator/` for an example of how to use the library.
<!--TODO: Add advice on dynamic vs. static library to comply with library license.-->

Known Bugs and Issues
---------------------

Currently no bugs are known to the author.
If you discover a bug, please use the issue tracker to report it, or fork this repository, fix the bug, and request a pull into the main repository.
<!--TODO: link to instruction for GitHub workflow-->

Authors
-------

- Wolfgang Traylor (wolfgang.traylor@senckenberg.de) <!--TODO: Add ORCID-->

License
-------

- The [Catch2](https://github.com/catchorg/Catch2) test framework (`tests/catch.hpp`) is licensed under the [Boost Software License](http://www.boost.org/LICENSE_1_0.txt).

- The version of the [PlantUML](http://plantuml.com) file (`docs/plantuml.jar`), which is used to render UML diagrams in the Doxygen documentation, is under the [MIT license](http://opensource.org/licenses/MIT).

- The [cpptoml](https://github.com/skystrife/cpptoml) library (`external/cpptoml/`) is licensed under the [MIT license](http://opensource.org/licenses/MIT).
