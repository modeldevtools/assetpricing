# -*- coding: utf-8 -*-
"""
Monte Carlo pricer
"""

import numpy as np

def heston_esg(nscens, nsteps, dt, kappa, theta, xi, mu, rho, v0, p0):
    """
    Generate scenarios based on Heston model.
    @param paths
    @param dt
    @param kappa
    @param theta
    @param xi
    @param mu
    @param v0
    """
    prices = np.zeros((nscens, nsteps))
    vols = np.zeros((nscens, nsteps))
    vols[:, 0] = v0
    prices[:, 0] = p0
    
    from numpy.linalg import cholesky
    corr = np.array([[1, rho], [rho, 1]])
    U = cholesky(corr)
    
    for j in xrange(1, nsteps):
        noises = np.random.normal(0., 1., (nscens, 2))
        shocks = np.dot(noises, U)
        
        vec_vols = vols[:, j-1]
        vec_prices = prices[:, j-1]
        
        vols[:, j] = vec_vols + kappa * (theta - vec_vols) * dt + xi * np.sqrt(vec_vols) * np.sqrt(dt) * shocks[:, 1]
        prices[:, j] = vec_prices + mu * vec_prices * dt + np.sqrt(vec_vols) * vec_prices * np.sqrt(dt) * shocks[:, 0]

    return vols, prices

if __name__ == "__main__":
    nscens = 100
    nsteps = 40
    dt = 1
    kappa = .2
    theta = .3
    xi = .2
    mu = .03
    v0 = .2
    p0 = 2000
    
    import matplotlib.pyplot as plt
    
    x = range(0, 40)
    vols, prices = heston_esg(nscens, nsteps, dt, kappa, theta, xi, mu, 0.5, v0, p0)
    
    plt.plot(x, prices.T)
    