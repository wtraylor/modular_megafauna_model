#!/bin/Rscript

# SPDX-FileCopyrightText: 2021 W. Traylor <wolfgang.traylor@senckenberg.de>
#
# SPDX-License-Identifier: MIT

library(ggplot2)

#' Increase fractional dry matter intake for young animals.
#'
#' @param digestive_limit_fraction Maximum dry matter intake for adults as
#'        fraction of body mass.
#' @param current_bodymass Current body mass [kg/ind].
#' @param adult_bodymass Body mass of a mature animal [kg/ind].
scale_intake_fraction_for_juveniles <- function(digestive_limit_fraction,
                                                current_bodymass,
                                                adult_bodymass)
{
  stopifnot(current_bodymass <= adult_bodymass)
  stopifnot(current_bodymass > 0)
  stopifnot(adult_bodymass > 0)
  stopifnot(digestive_limit_fraction >= 0)
  stopifnot(digestive_limit_fraction < 1)

  # The exponent is Kleiber’s ¾ exponent minus 1 because the digestive limit is
  # a *fraction* of body mass. So M^{0.75} / M = M^{-0.25}.
  return(digestive_limit_fraction
         * adult_bodymass^(0.25)
         * current_bodymass^(-0.25))
}


# Digestive limit for adult
diglimit_ad <- .026

bodymass_ad    <- 810
bodymass_birth <- 23

diglimit.tbl <- data.frame(bodymass = c(seq(bodymass_birth,
                                            bodymass_ad,
                                            bodymass_ad / 100), bodymass_ad))

diglimit.tbl$diglimit <- mapply(scale_intake_fraction_for_juveniles,
                                diglimit_ad,
                                diglimit.tbl$bodymass,
                                bodymass_ad)
diglimit.tbl$index <- seq.int(nrow(diglimit.tbl))

svg("scale_diglimit.svg", height = 4, width = 6)
ggplot(diglimit.tbl,
       aes(index, diglimit)) +
geom_line() +
expand_limits(x = c(0, max(diglimit.tbl$index) * 1.1), y = 0) +
theme(axis.ticks = element_blank()) +
scale_x_continuous(name   = "M, Body Mass",
                   # Get indices of first occurrences of birth/adult body mass.
                   breaks = c(match(bodymass_birth, diglimit.tbl$bodymass),
                              match(bodymass_ad, diglimit.tbl$bodymass)),
                   labels = c("Birth", "Adult ♂"),
                   expand = c(0,0)) +
scale_y_continuous(name   = "DMI, Maximum Daily Intake\n(as fraction of body mass)",
                   breaks = c(0, diglimit_ad),
                   labels = c("0%", expression('DMI'[ad])),
                   expand = c(0,0)) +
theme_bw()
dev.off()
