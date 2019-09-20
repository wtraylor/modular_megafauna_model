# Quickstart Guide for Using the Modular Megafauna Model {#page_quickstart}
<!-- For doxygen, this is the *page* header -->
\brief How to get the first simulations running.

# Quickstart Guide for Using the Modular Megafauna Model {#sec_quickstart}
<!-- For doxygen, this is the *section* header -->
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

A very simple [RMarkdown]()<!--TODO--> file to visualize the demo output is provided in the `build` folder.
It is called `demo_results.Rmd`.
You can open and render it in RStudio.
Alternatively, to produce an HTML file (`demo_results.html`) to open it in a web browser, execute these commands in an R console inside the `build/` directory:

```r
library(knitr)
library(markdown)
knit("demo_results.Rmd", "demo_results.md")
markdownToHTML("demo_results.md", "demo_results.html")
```

## Constant Population {#sec_constant_pop}

Prescribing a constant herbivore density to the model may be used to simulate the effects of different stocking regimes.
Reproduction should then be disabled.
The herbivores may be immortal so that the stocking rate remains constant even if the animals are starving.
This is a minimal example herbivore instruction file for cattle:

```fortran
ifherbivory 1
digestibility_model "npp"
free_herbivory_years 100
herbivore_type "cohort"
hft "cattle" (
	include 1
	bodyfat_max   0.3 ! fractional
	bodyfat_max_daily_gain 0.0 ! fractional
	bodymass_female 400 ! kg
	bodymass_male   400 ! kg
	diet_composer "pure_grazer"
	digestion_type "ruminant"
	digestive_limit "illius_gordon_1992"
	establishment_age_range 1 1 ! arbitrary
	establishment_density 1.0 ! ind/km²
	expenditure_components "taylor_1981"
	foraging_limits "illius_oconnor_2000"
	half_max_intake_density  40 ! gDM/m²
)
```
The parameter values are mostly taken from Illius & O’Connor (2000)\cite illius2000resource.
As this only defines the parameters for the herbivory model, you will need to `import` the LPJ-GUESS instruction files, specify the driving data, and select the herbivory output files you are interested in.

With this instruction file, one immortal cohort of adult body mass will be established.
The mass density may fluctuate because of fat mass, but individual density will remain constant.

The `establishment_density` is probably the most crucial parameter in this setup.
Currently, the density can only prescribed globally and is constant.

