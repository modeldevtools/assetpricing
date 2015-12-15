MAXITER <- 500

Fz <- function (x) {
  rs <- exp(-x ^ 2 / 2) / sqrt(2 * pi)
  return(rs)
}

dOne <- function (s, k, tau, r, sigma) {
  d <- (log(s / k) + tau * (r + 0.5 * sigma ^ 2)) / (sigma * sqrt(tau))
  return(d)
}

is.put.call <- function (put.call) {
  if (put.call != "c" && put.call != "p" && put.call != "s")
    stop(paste("'", put.call, "' is not a valid put call flag"))
}

bs.delta <- function (put.call, s, k, tau, r, sigma) {
  is.put.call(put.call)

  delta <- switch(put.call,
                  s=1,
                  c=pnorm(dOne(s, k, tau, r, sigma)),
                  p=pnorm(dOne(s, k, tau, r, sigma)) - 1)

  return(delta)
}

bs.gamma <- function (put.call, s, k, tau, r, sigma) {
  is.put.call(put.call)

  gamma <- switch(put.call,
                  s=0,
                  p=,
                  c=Fz(dOne(s, k, tau, r, sigma)) / s / sigma / sqrt(tau))

  return(gamma)
}

bs.theta <- function (put.call, s, k, tau, r, sigma) {
  is.put.call(put.call)

  theta <- switch(put.call,
                  s=0,
                  p=-s * Fz(dOne(s, k, tau, r, sigma)) * sigma / 2 / sqrt(tau) + r * k * exp(-r * tau) * pnorm(sigma * sqrt(tau) - dOne(s, k, tau, r, sigma)),
                  c=-s * Fz(dOne(s, k, tau, r, sigma)) * sigma / 2 / sqrt(tau) - r * k * exp(-r * tau) * pnorm(dOne(s, k, tau, r, sigma) - sigma * sqrt(tau))
  )

  return(theta)
}

bs.theta.per.day <- function(put.call, s, k, tau, r, sigma) {
  return(bs.theta(put.call, s, k, tau, r, sigma) / 365)
}

bs.price <- function (put.call, s, k, tau, r, sigma) {
  is.put.call(put.call)

  if (tau == 0) {
    price <- switch(put.call,
                    s=s,
                    c=max(0, (s - k)),
                    p=max(0, -(s - k)))
    return(price)
  }

  call <- s * pnorm(dOne(s, k, tau, r, sigma)) - exp(-r * tau) * k * pnorm(dOne(s, k, tau, r, sigma) - sigma * sqrt(tau))

  price <- switch(put.call,
                  s=s,
                  c=call,
                  p=call - s + k * exp(-r * tau))

  return(price)
}

bs.imp.vol.bisect <- function (put.call, s, k, tau, r, a, b, mv) {
  is.put.call(put.call)

  TOL <- 1e-05

  lowCdif <- mv - bs.price(put.call, s, k, tau, r, a)
  highCdif <- mv - bs.price(put.call, s, k, tau, r, b)

  # Terminate if both the low and high starting points are on the same
  # side of the mv.
  if (lowCdif * highCdif > 0) {
    return(0)
  }

  for (cnt in 1:MAXITER) {
    midP <- (a + b) / 2
    midCdif <- mv - bs.price(put.call, s, k, tau, r, midP)
    if (abs(midCdif) < TOL)
      break
    if (midCdif > 0)
      a = midP
    else
      b = midP
  }
  return(midP)
}

