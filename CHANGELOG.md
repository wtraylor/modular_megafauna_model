# Changelog
All notable changes to this project will be documented in this file.

The format is based on [Keep a Changelog](https://keepachangelog.com/en/1.0.0/) by Olivier Lacan, and this project adheres to [Semantic Versioning](https://semver.org/spec/v2.0.0.html).

## Unreleased
### Added
- New instruction file parameters, which were constants before:
    - `hft.digestion.anabolism_coefficient` (formerly `Fauna::FatMassEnergyBudget::FACTOR_ANABOLISM`)
    - `hft.digestion.catabolism_coefficient` (formerly `Fauna::FatMassEnergyBudget::FACTOR_CATABOLISM`)
    - `hft.digestion.efficiency` (formerly `Fauna::DIGESTION_EFFICIENCY_HINDGUTS`)
    - `hft.digestion.i_g_1992_ijk` (formerly constants in the function object `Fauna::GetDigestiveLimitIlliusGordon1992`)
    - `simulation.metabolizable_energy` (formerly `Fauna::ME_COEFFICIENT_GRASS`)
    - `hft.reproduction.logistic.growth_rate` and `.midpoint` (formerly constant parameters in `Fauna::ReprIlliusOConnor2000`).
- Default HFT groups “ruminants” and “hindguts” (replacing parameter `hft.digestion.type`)

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

## 0.1.0 - 2019-11-07
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
