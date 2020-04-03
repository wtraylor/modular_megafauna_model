# Quickstart Guide {#page_quickstart}
\brief How to get the first simulations running.

\tableofcontents

## Compile and Run Unit Tests {#sec_compile_and_run_unit_tests}

To check if the modular megafauna model works by itself correctly, you should compile it and run the unit tests.
You will need [Cmake](https://cmake.org) and a C++ compiler.
It should work well with GCC, the [GNU C++ compiler](https://gcc.gnu.org).

Open a Unix shell in the root of this repository and run:

```sh
mkdir -p build
cd build
cmake -DBUILD_TESTING=ON ..
make megafauna_unit_tests
./megafauna_unit_tests
```

If the unit tests all pass successfully, you are good to go!

## Run the Demo Simulator {#sec_run_the_demo_simulator}

With the CMake option `BUILD_DEMO_SIMULATOR=ON` you can compile the demo simulator.
This independent program is a very simple grass simulator that hosts the megafauna library.
It should work out of the box with the instructions files in the `examples/` folder.

Again, open a terminal in the repository root folder and run:

```sh
mkdir -p build
cd build
cmake -DBUILD_DEMO_SIMULATOR=ON ..
make megafauna_demo_simulator
./megafauna_demo_simulator "../examples/megafauna.toml" "../examples/demo_simulation.toml"
```

Congratulations, you have run your first simulation!

The simulator does not want to have you lose your results and will refuse to overwrite existing output files.
So if you want to run it again, you will first need to remove the previously created output tables: `rm *.tsv`

A very simple [RMarkdown](https://rmarkdown.rstudio.com/) file to visualize the demo output is provided in the `build` folder.
It is called `demo_results.Rmd`.
You can open and render it in RStudio.
Alternatively, to produce an HTML file (`demo_results.html`) to open it in a web browser, execute these commands in an R console inside the `build/` directory:

```r
library(knitr)
library(markdown)
knit("demo_results.Rmd", "demo_results.md")
markdownToHTML("demo_results.md", "demo_results.html")
```

## Customize the Instruction File {#sec_customize_the_instruction_file}

The Modular Megafauna Model library requires a single instruction file to set general simulation parameters and herbivore (HFT = Herbivore Functional Type) parameters.
The instruction file is in [TOML v0.5](https://github.com/toml-lang/toml/blob/master/versions/en/toml-v0.5.0.md) format; see there for a description of the syntax.
String parameters are generally case insensitive.

All possible options are listed in the example file under `examples/megafauna.toml`.
You will find all parameters explained in detail in the Doxygen documentation of the two classes \ref Fauna::Parameters and \ref Fauna::Hft.
The parameter names and cascaded categories are kept as consistent as possible between the C++ classes and the TOML file.

Both HFTs and HFT groups are represented as arrays of tables in the TOML syntax.
You can define any number of HFTs, but they need to have unique names, and there must be at least one HFT.
Any HFT can be assigned to any number of groups to inherit parameters from that group.
However, you cannot cascade groups, and an HFT cannot inherit from another HFT.

In general, the user is forced to specify all parameters.
This way one instruction file is always complete and self-explanatory.
Default values in the C++ code might change between model versions.
However, the model should still yield the same results with the same instruction file.

\see \ref sec_design_parameters

## Recipes {#sec_quickstart_recipes}

Here are some instruction file snippets that may help you for different scientific questions.

### Constant Population {#sec_recipe_constant_population}

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

------------------------------------------------------------

\author Wolfgang Traylor, Senckenberg BiK-F
\date 2019
\copyright ...
