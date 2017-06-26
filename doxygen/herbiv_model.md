\page page_herbiv_model Large Herbivore Model
\brief Structure and scientific explanation of the Large Herbivore Model.
\author Wolfgang Pappa, Senckenberg BiK-F
\date May 2017
\see The page \ref page_herbiv_tutor offers tutorials on how to use the model.
\see The page \ref group_herbivory gives implementation details.
\see The page \ref page_herbiv_tests explains the testing %framework.

\section sec_herbiv_foraging Foraging

\subsection sec_herbiv_foraging_patch Feeding on Plants in a Patch

Each \ref Individual offers an amount of forage (kgDM/m²) that is available
to herbivores (\ref Individual.get_forage_mass()).

\todo Growth happens only once per year (\ref growth()) for natural vegetation.
However, seasonal shifts of forage availability are crucial for herbivore
dynamics.
A solution for that is yet to be found.


\subsection sec_herbiv_digestibility Digestibility


\section sec_herbiv_reproduction Reproduction


\section sec_herbiv_mortality Mortality


\section sec_herbiv_nitrogen_excretion Nitrogen Excretion


\section sec_herbiv_trampling Trampling          


*/
