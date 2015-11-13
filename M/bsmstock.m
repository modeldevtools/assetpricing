AssetPrice = 1884.089966;
Strike = 1946.160034;
Sigma = .2083;
Rates = 0.0032;
DivYield = 0.022;
Settle = 'Nov-11-2014';
Maturity = 'Nov-11-2015';

% define the RateSpec and StockSpec
RateSpec = intenvset('ValuationDate', Settle, 'StartDates', Settle, 'EndDates',...
    Maturity, 'Rates', Rates, 'Compounding', -1, 'Basis', 13);

StockSpec = stockspec(Sigma, AssetPrice, 'continuous', DivYield);

% define the options
OptSpec = {'call', 'put'};
OutSpec = {'Delta', 'Gamma', 'Vega', 'Price'};
[Delta, Gamma, Vega, Price] = optstocksensbybls(RateSpec, StockSpec, Settle, ...
    Maturity, OptSpec, Strike, 'OutSpec', OutSpec);

Delta

Gamma

Vega

Price
