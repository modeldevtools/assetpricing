# Load libraries
library(quantmod)

# Specify stock symbols
# TODO load a list of symbols from file
#symbol <- c("TSLA", "MSFT", "GOOG", "KFN", "C", "BRK-B", "NOK")
symbol <- "TSLA"

# Get stock quotes
quotes <- getQuote("TSLA")

# Pull option chain data from Yahoo
opt.chains <- get.option.chain.yahoo(symbol)

# TODO: Store option chain into local storage

# Risk-free rate
rf <- 0.015

# Dividend yield
dvd <- 0

# Calculate implied volatilities

a <- 0.0001
b <- 5
s <- quotes[1, "Last"]
val.date <- as.Date("2013-11-15")

opt.chains$tau <- as.numeric(opt.chains$exp.date - val.date - 1) / 365

opt.chains$imp.vol <- mapply(bs.imp.vol.bisect,
                             opt.chains$put.call,
                             s,
                             opt.chains$Strike,
                             opt.chains$tau,
                             rf,
                             a,
                             b,
                             opt.chains$mid)

# TODO construct volatility surface
#volsurf <- data.frame()

# Specify strategy


# TODO Payoff chart of the strategy


# Calculate valuation matrix for the strategy

