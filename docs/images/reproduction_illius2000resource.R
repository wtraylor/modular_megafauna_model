#!/usr/bin/env Rscript

# SPDX-FileCopyrightText: 2021 W. Traylor <wolfgang.traylor@senckenberg.de>
#
# SPDX-License-Identifier: MIT

# This script does not exactly reproduce "reproduction_illius2000resource.png"
# because the original was created for LaTeX, which created the pretty math
# notation.

png("reproduction_illius2000resource.png")

repmax <- 1.0 # maximum reproduction
b <- 15
c <- 0.3
reproduction <- function(body_cond){
  repmax / (1 + exp(-b * (body_cond-0.3)))
}

plot(
  reproduction,
  xlim = c(0,1),
  xlab = "F/Fmax, body condition",
  ylab = "m, annual reproduction",
  bty = "l",
  xaxt = "n", yaxt = "n"# don’t plot axis
)
axis(
  side = 2,
  at = c(0,repmax),
  labels = c("0", "mmax")
)
axis(
  side = 1,
  at = c(0,c,1),
  labels = c("0", paste0("c = ",c,""), "1.0")
)
abline( v = c, lty = "dotted")
text(
  "m = mmax / (1 + e^(-b * (F/Fmax - c)))",
  x = mean(c(c,1.0)),
  y = repmax/2,
  cex = .9
)

dev.off()
