get.option.chain.yahoo <- function (symbol) {
  library(quantmod)

  retval <- data.frame()

  # Convert option chain data pulled by quantmod's getOptionChain
  # function into the desired data.frame layout

  # Pull all available option series
  chain <- getOptionChain(symbol, NULL)

  # Extract expiration date
  for (put.call in c("puts", "calls")) {
    for (opt.series in chain) {
      opt.series[[put.call]]$exp.date <- sapply(row.names(opt.series[[put.call]]), extract.exp.date)
      opt.series[[put.call]]$exp.date <- as.Date(opt.series[[put.call]]$exp.date)
      opt.series[[put.call]]$put.call <- if (put.call == "puts") "p" else "c"
      opt.series[[put.call]]$mid <-
        if (is.na(opt.series[[put.call]]$Bid) || is.na(opt.series[[put.call]]$Ask)) {
          opt.series[[put.call]]$Last
        } else {
          (opt.series[[put.call]]$Bid + opt.series[[put.call]]$Ask) / 2
        }

      retval <- rbind(opt.series[[put.call]], retval)
    }
  }

  # Order option chain by expiration date, put call flag, and strike
  retval <- retval[with(retval, order(exp.date, put.call, Strike)), ]

  retval
}

extract.exp.date <- function (opt.ticker) {
  underlying.len <- nchar(opt.ticker) - 15
  exp.date <- as.Date(substr(opt.ticker, underlying.len + 1, underlying.len + 6), "%y%m%d")

  exp.date
}
