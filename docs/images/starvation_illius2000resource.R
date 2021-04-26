#!/usr/bin/env Rscript

# SPDX-FileCopyrightText: 2021 W. Traylor <wolfgang.traylor@senckenberg.de>
#
# SPDX-License-Identifier: MIT

# Plot starvation mortality from Illius & O’Connor (2000).

library(ggplot2)
library(dplyr)

SD     <- 0.125 # standard deviation in body condition
AVG_BC <- 0.2   # Mean body condition in the cohort.

body_condition <- seq(-0.5, 1.0, by=0.003)
dist           <- mapply(dnorm,
                         body_condition-0.2,
                         sd = SD)

df <- data.frame(body_condition = body_condition,
                 dist = dist,
                 dead_dist = dist * (body_condition < 0))

png("starvation_illius2000resource.png",
    res    = 150,
    unit   = "cm",
    width  = 15,
    height = 7.5)

ggplot(df,
       aes(x = body_condition,
           y = dist)) +
geom_line() +
geom_area(fill = "lightgray") +
geom_area(aes(y = dead_dist),
          fill = "red") +
geom_vline(xintercept = 0,
           color = "red") +
geom_vline(xintercept = AVG_BC,
           color      = "blue") +
annotate("text",
         x = AVG_BC,
         y = 0,
         hjust = 1.1,
         vjust = -.1,
         label = "mean",
         color = "blue") +
labs(x = "Body Condition (F/Fmax)",
     y = "Distribution in Cohort",
     caption = paste0("Standard Deviation σ = ", SD, ", ",
                      "Mean = ", AVG_BC))

dev.off()

######################################################################
# References:
# Illius, A. W. & O'Connor, T. G. (2000). Resource heterogeneity and ungulate population dynamics. Oikos, 89, 283-294.
