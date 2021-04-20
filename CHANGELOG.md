<!--
SPDX-FileCopyrightText: 2020 Wolfgang Traylor <wolfgang.traylor@senckenberg.de>

SPDX-License-Identifier: CC-BY-4.0
-->

# Changelog
All notable changes to this project will be documented in this file.

The format is based on [Keep a Changelog][] by Olivier Lacan, and this project adheres to [Semantic Versioning][].

[Keep a Changelog]: <https://keepachangelog.com/en/1.0.0/>
[Semantic Versioning]: <https://semver.org/spec/v2.0.0.html>

## [Unreleased]

### Added
- R scripts and LibreOffice document to reproduce figures in `docs/images/`. [#31]

### Changed
- Use sward density for `Fauna::HalfMaxIntake` (instead of whole-habitat grass density). This requires the vegetation model to provide correct “FPC” values (fractional area covered by grass in the habitat). [#12]
- Use modern `rmarkdown` package to render demo results.

### Fixed
- Demo simulator has now monthly ambient air temperature.
- Replace copyrighted figure `docs/images/thermoregulation.png` with my own. [#32]

### Removed
- Snow depth. It is not used anywhere. [#14]

## [0.6.0] - 2021-04-08

### Added
- Invitation to Matrix room in README.

### Fixed
- Add licensing information. [#22]

### Removed
- Nitrogen retention by herbivores. The function `Habitat::add_excreted_nitrogen()` has been removed. The vegetation model must now handle recycling of nitrogen itself. This simplifies the code and reduces the number of parameters. Compare [#15]

## [0.5.5] - 2020-08-04

### Fixed
- Floating point imprecision. [#29]
- Too much forage got eaten in one day in the demo simulator. [#29]

## [0.5.4] - 2020-08-03

### Fixed
- Misleading error message if TOML parameter has wrong type. [#25]
- Group parameter `expenditure.components` cannot be re-used. [#28]
- No error if an HFT group is defined twice. [#27]
- No instruction file check that catabolism must be smaller than anabolism coefficient. [#26]

## [0.5.3] - 2020-06-05

### Fixed
- Crash when reading instruction file without parameter `hft.digestion.i_g_1992_ijk`.

### Changed
- Single values in TOML file can be parsed like a single-element array.

## [0.5.2] - 2020-06-02

### Fixed
- Crash if not all HFTs were established immediately at the start of the start of the simulation.

## [0.5.1] - 2020-05-29

### Added
- Executable `megafauna_insfile_linter` for checking if a TOML instruction file is valid.

### Fixed
- Herbivores are now only established if the `do_herbivores==true` flag is passed into MMM.
- Parameter `hft.digestion.i_g_1992_ijk` is not mandatory anymore.
- Fix compiling errors under GCC 10.1.0 about not finding `std` exception.

## [0.5.0] - 2020-04-10

### Added
- Revised and expanded Doxygen page on model description/discussion.
- Parameter `hft.expenditure.fmr_multiplier`. [#9]

### Changed
- Output `eaten_nitrogen_per_ind` is now in milligram, not kilogram, per day and individual.
- Remove `Fauna::Parameters` from the library’s include interface.
- The Doxygen documentation of the library’s include interface can be parsed without errors.
- Reproductive success at parturition is now based on body condition at day of conception. [#10]
- Scale maximum daily intake (DMI) allometrically from fraction of adult mass. [#8]
    - Parameter `hft.digestion.allometric.coefficient` changed to `.value_male_adult`.
- Expenditure component `Allometric` is now `BasalMetabolicRate` and `FieldMetabolicRate`. [#9]
    - Parameter `hft.expenditure.allometric` changed to `.basal_rate`.

### Removed
- Individual mode. [#7]

## [0.4.0] - 2020-03-27

### Added
- New output tables: `available_forage`, `eaten_forage_per_ind`, `eaten_nitrogen_per_ind`.

## [0.3.1] - 2020-03-26

### Fixed
- `Fauna::World` can be constructed without an instruction file. [#4]
- Correctly plot demo results with `demo_results.Rmd` if output is daily or decadal.
- Fix “First day before last” error in decadal output. [#3]

## [0.3.0] - 2020-03-17
### Added
- Net energy content model: `NetEnergyModel::GrossEnergyFraction`
    - Parameter `forage.gross_energy`
    - Parameter `hft.digestion.digestibility_multiplier`
    - Parameter `hft.digestion.k_fat`
    - Parameter `hft.digestion.k_maintenance`
    - Parameter `hft.digestion.me_coefficient`
- Model description of forage energy and digestion.
- Parameter `hft.body_mass.empty`.
- Parameter `hft.body_fat.catabolism_efficiency`.

### Changed
- Fractional body fat now refers to the empty body (i.e. without ingesta, blood, etc.)  and not the live body mass.
- HFT is made optional. New implementations of `HerbivoreInterface` don’t require an HFT. [#5]

### Fixed
- Order of HFT names in text table output is now guaranteed.
- Compiler flags specific to GCC are removed. [#1]
- Unknown TOML parameters/keys now issue an error. [#2]
- Parse TOML parameter `hft.body_fat.gross_energy`.
- Run directory is now created in script `tools/run_valgrind_memcheck`.

### Removed
- The old “default” net energy model `get_net_energy_content_default()`.
- Parameter `hft.digestion.efficiency` (less efficient digestion of hindgut
  fermenters is now in `hft.digestion.digestibility_multiplier`).

## [0.2.0] - 2019-12-06
### Added
- New instruction file parameters, which were constants before:
    - `hft.digestion.anabolism_coefficient` (formerly `Fauna::FatMassEnergyBudget::FACTOR_ANABOLISM`)
    - `hft.digestion.catabolism_coefficient` (formerly `Fauna::FatMassEnergyBudget::FACTOR_CATABOLISM`)
    - `hft.digestion.efficiency` (formerly `Fauna::DIGESTION_EFFICIENCY_HINDGUTS`)
    - `hft.digestion.i_g_1992_ijk` (formerly constants in the function object `Fauna::GetDigestiveLimitIlliusGordon1992`)
    - `simulation.metabolizable_energy` (formerly `Fauna::ME_COEFFICIENT_GRASS`)
    - `hft.reproduction.logistic.growth_rate` and `.midpoint` (formerly constant parameters in `Fauna::ReprIlliusOConnor2000`).
- Default HFT groups “ruminants” and “hindguts” (replacing parameter `hft.digestion.type`)
- Simple bash script `run_demo` produced in the build folder to execute demo simulator with results in one command.

### Changed
- Replaced `std::map` with `std::array` in `Fauna::ForageValues` to improve speed.
- Various little performance improvements.
- Turned `Fauna::GetNetEnergyContent` interface (strategy design pattern) to a function.
- Turned `Fauna::GetDigestiveLimitIlliusGordon1992` into the function `Fauna::get_digestive_limit_illius_gordon_1992()`.
- Renamed TOML parameter `hft.foraging.net_energy_model` to `hft.digestion.net_energy_model`
- Renamed `Fauna::ReproductionModel::IlliusOConnor2000` to `Logistic`.

### Fixed
- The parameter `hft.mortality.factors` was not parsed.

### Removed
- Instruction file parameter `hft.digestion.type` and the corresponding `Fauna::DigestionType` and `Fauna::Hft::digestion_type`.

## [0.1.0] - 2019-11-07
### Added
- Herbivores in cohort and individual mode.
    - Energy expenditure components:
        - Allometric
        - Based on [Taylor et al. (1981)](http://journals.cambridge.org/article_S0003356100040617)
        - Thermoregulation
    - Diet composer: only grass
    - Reproduction models:
        - Constant annual reproduction rate
        - Based on [Illius & O’Connor (2000)](http://www.jstor.org/stable/3547323)
        - Linear relationship with body condition
    - Mortality factors:
        - Constant annual background mortality
        - Death at end of lifespan
        - Starvation mortality based on [Illius & O’Connor (2000)](http://www.jstor.org/stable/3547323)
        - Starvation at a threshold value of body condition
    - Foraging limits:
        - Functional response based on [Illius & O’Connor (2000)](http://www.jstor.org/stable/3547323)
        - General Holling Type II functional response
    - Net energy in forage: formula used by [Illius & O’Connor (2000)](http://www.jstor.org/stable/3547323)
- Continuous integration (CI) for GitLab.
- Output in tab-separated text tables.
- TOML instruction file reader.
- Demo simulator with simple logistic grass growth.

[Unreleased]: https://github.com/wtraylor/modular_megafauna_model/compare/0.6.0...develop
[0.6.0]: https://github.com/wtraylor/modular_megafauna_model/compare/0.5.5...0.6.0
[0.5.5]: https://github.com/wtraylor/modular_megafauna_model/compare/0.5.4...0.5.5
[0.5.4]: https://github.com/wtraylor/modular_megafauna_model/compare/0.5.3...0.5.4
[0.5.3]: https://github.com/wtraylor/modular_megafauna_model/compare/0.5.2...0.5.3
[0.5.2]: https://github.com/wtraylor/modular_megafauna_model/compare/0.5.1...0.5.2
[0.5.1]: https://github.com/wtraylor/modular_megafauna_model/compare/0.5.0...0.5.1
[0.5.0]: https://github.com/wtraylor/modular_megafauna_model/compare/0.4.0...0.5.0
[0.4.0]: https://github.com/wtraylor/modular_megafauna_model/compare/0.3.1...0.4.0
[0.3.1]: https://github.com/wtraylor/modular_megafauna_model/compare/0.3.0...0.3.1
[0.3.0]: https://github.com/wtraylor/modular_megafauna_model/compare/0.2.0...0.3.0
[0.2.0]: https://github.com/wtraylor/modular_megafauna_model/compare/0.1.0...0.2.0
[0.1.0]: https://github.com/wtraylor/modular_megafauna_model/releases/tag/0.1.0

[#1]: https://github.com/wtraylor/modular_megafauna_model/issues/1
[#2]: https://github.com/wtraylor/modular_megafauna_model/issues/2
[#3]: https://github.com/wtraylor/modular_megafauna_model/issues/3
[#4]: https://github.com/wtraylor/modular_megafauna_model/issues/4
[#5]: https://github.com/wtraylor/modular_megafauna_model/issues/5
[#6]: https://github.com/wtraylor/modular_megafauna_model/issues/6
[#7]: https://github.com/wtraylor/modular_megafauna_model/issues/7
[#8]: https://github.com/wtraylor/modular_megafauna_model/issues/8
[#9]: https://github.com/wtraylor/modular_megafauna_model/issues/9
[#10]: https://github.com/wtraylor/modular_megafauna_model/issues/10
[#12]: https://github.com/wtraylor/modular_megafauna_model/issues/12
[#14]: https://github.com/wtraylor/modular_megafauna_model/issues/14
[#15]: https://github.com/wtraylor/modular_megafauna_model/issues/15
[#22]: https://github.com/wtraylor/modular_megafauna_model/issues/22
[#25]: https://github.com/wtraylor/modular_megafauna_model/issues/25
[#26]: https://github.com/wtraylor/modular_megafauna_model/issues/26
[#28]: https://github.com/wtraylor/modular_megafauna_model/issues/28
[#29]: https://github.com/wtraylor/modular_megafauna_model/issues/29
[#31]: https://github.com/wtraylor/modular_megafauna_model/issues/31
[#32]: https://github.com/wtraylor/modular_megafauna_model/issues/32
