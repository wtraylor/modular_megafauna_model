#! /bin/Rscript

# SPDX-FileCopyrightText: 2021 W. Traylor <wolfgang.traylor@senckenberg.de>
#
# SPDX-License-Identifier: MIT

# Plot shift of body condition after applying the starvation mortality
# algorithm from Illius & O’Connor (2000).
# See the Doxygen documentation of the function in the megafauna code for
# details.


#' Get new body condition after applying mortality.
#'
#' @param b The old average body condition in the cohort.
#' @param d The fraction that died.
get_new_body_condition <- function(b, d){
  return(b / (1 - d))
}

#' Get fraction of cohort that dies.
#'
#' This just follows Illis & O’Connor (2000)
#' @param b Current average body condition.
#' @param sd Standard deviation in normal distribution of body condition.
get_mortality <- function(b, sd){
  # `pnorm() is the cumulative normal distribution function.
  return(pnorm(-b / sd))
}

png(
  "starvation_body_condition_shift_abs.png",
  res    = 150,
  unit   = "cm",
  width  = 7,
  height = 7
)
SD <- 0.125
plot(
  function(b){
    d     <- get_mortality(b, SD)
    b_new <- get_new_body_condition(b, d)
    return(b_new - b)
  },
  xlim = c(0,0.2),
  xlab = "Body Condition (b)",
  ylab = "Abs. Change in Body Condition (Δb)"
)
dev.off()

png(
  "starvation_body_condition_shift_rel.png",
  res    = 150,
  unit   = "cm",
  width  = 7,
  height = 7
)
SD <- 0.125
plot(
  function(b){
    d     <- get_mortality(b, SD)
    b_new <- get_new_body_condition(b, d)
    return((b_new - b) / b)
  },
  xlim = c(0,0.2),
  xlab = "Body Condition (b)",
  ylab = "Rel. Change in Body Condition (Δb/b)"
)
dev.off()

######################################################################
# References:
# Illius, A. W. & O'Connor, T. G. (2000). Resource heterogeneity and ungulate
# population dynamics. Oikos, 89, 283-294.
