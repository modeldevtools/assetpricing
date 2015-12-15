/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

#include <ql/quantlib.hpp>
#include <boost/timer.hpp>
#include <iostream>
#include <iomanip>

using namespace QuantLib;

// function declaration
PiecewiseYieldCurve<Discount, LogLinear>* create_yield_curve();

int calc_equityoption() {

    try {

        boost::timer timer;
        std::cout << std::endl;

        // set up dates
        Calendar calendar = UnitedStates();
        Date todaysDate(30, Dec, 2011);
        Date settlementDate(3, Jan, 2012);
        //Settings::instance().evaluationDate() = todaysDate;

        // set up option instrument
        Option::Type type(Option::Put);
        Real underlying = 1257.60;              // SPX index value as of 12/30/2011
        Real strike = 1248.00;
        Spread dividendYield = 0.02494672428;   // SPX dividend yield btw 12/31/2012 and 12/30/2013
        Volatility volatility = 0.77;           // Volatility taken from David's analysis spreadsheet
        Date maturity(19, Sep, 2013);
        DayCounter dayCounter = Actual365Fixed();

        boost::shared_ptr<Exercise> europeanExercise(
            new EuropeanExercise(maturity));

        boost::shared_ptr<StrikedTypePayoff> payoff(
                                        new PlainVanillaPayoff(type, strike));

        Handle<Quote> underlyingH(
            boost::shared_ptr<Quote>(new SimpleQuote(underlying)));

        VanillaOption europeanOption(payoff, europeanExercise);

        std::cout << "Option type = "               << type << std::endl;
        std::cout << "Maturity = "                  << maturity << std::endl;
        std::cout << "Underlying price = "          << underlying << std::endl;
        std::cout << "Strike = "                    << strike << std::endl;
        std::cout << "Dividend yield = "            << io::rate(dividendYield)
                  << std::endl;
        std::cout << "Volatility = "                << io::volatility(volatility)
                  << std::endl;
        std::cout << std::endl;
        std::string method;
        std::cout << std::endl;

        auto depoFutSwapTermStructure = create_yield_curve();

        // bootstrap the yield/dividend/vol curves
        Handle<YieldTermStructure> liborYieldCurve(depoFutSwapTermStructure);
        Handle<YieldTermStructure> flatDividendTS(
            boost::shared_ptr<YieldTermStructure>(
                new FlatForward(settlementDate, dividendYield, dayCounter)));
        Handle<BlackVolTermStructure> flatVolTS(
            boost::shared_ptr<BlackVolTermStructure>(
                new BlackConstantVol(settlementDate, calendar, volatility,
                                     dayCounter)));
        
        boost::shared_ptr<BlackScholesMertonProcess> bsmProcess(
                 new BlackScholesMertonProcess(underlyingH, flatDividendTS,
                                               liborYieldCurve, flatVolTS));

        // write column headings
        Size widths[] = { 35, 14, 14 };
        std::cout << std::setw(widths[0]) << std::left << "Method"
                  << std::setw(widths[1]) << std::left << "NPV"
                  << std::setw(widths[2]) << std::left << "Delta"
                  << std::endl;

        // Analytic formulas:

        // Black-Scholes for European
        method = "Black-Scholes";
        europeanOption.setPricingEngine(boost::shared_ptr<PricingEngine>(
                                     new AnalyticEuropeanEngine(bsmProcess)));
        std::cout << std::setw(widths[0]) << std::left << method
                  << std::fixed
                  << std::setw(widths[1]) << std::left << europeanOption.NPV()
                  << std::setw(widths[2]) << std::left << europeanOption.delta()
                  << std::endl;

        // semi-analytic Heston for European
        /*method = "Heston semi-analytic";
        boost::shared_ptr<HestonProcess> hestonProcess(
            new HestonProcess(liborYieldCurve, flatDividendTS,
                              underlyingH, volatility*volatility,
                              1.0, volatility*volatility, 0.001, 0.0));
        boost::shared_ptr<HestonModel> hestonModel(
                                              new HestonModel(hestonProcess));
        europeanOption.setPricingEngine(boost::shared_ptr<PricingEngine>(
                                     new AnalyticHestonEngine(hestonModel)));
        std::cout << std::setw(widths[0]) << std::left << method
                  << std::fixed
                  << std::setw(widths[1]) << std::left << europeanOption.NPV()
                  << std::setw(widths[2]) << std::left << europeanOption.delta()
                  << std::endl;*/

        // semi-analytic Bates for European
        /*method = "Bates semi-analytic";
        boost::shared_ptr<BatesProcess> batesProcess(
            new BatesProcess(liborYieldCurve, flatDividendTS,
                             underlyingH, volatility*volatility,
                             1.0, volatility*volatility, 0.001, 0.0,
                             1e-14, 1e-14, 1e-14));
        boost::shared_ptr<BatesModel> batesModel(new BatesModel(batesProcess));
        europeanOption.setPricingEngine(boost::shared_ptr<PricingEngine>(
                                                new BatesEngine(batesModel)));
        std::cout << std::setw(widths[0]) << std::left << method
                  << std::fixed
                  << std::setw(widths[1]) << std::left << europeanOption.NPV()
                  << std::setw(widths[2]) << std::left << europeanOption.delta()
                  << std::endl;*/

        // Integral
        method = "Integral";
        europeanOption.setPricingEngine(boost::shared_ptr<PricingEngine>(
                                             new IntegralEngine(bsmProcess)));
        std::cout << std::setw(widths[0]) << std::left << method
                  << std::fixed
                  << std::setw(widths[1]) << std::left << europeanOption.NPV()
                  << std::setw(widths[2]) << std::left << europeanOption.delta()
                  << std::endl;

        // Finite differences
        Size timeSteps = 801;
        method = "Finite differences";
        europeanOption.setPricingEngine(boost::shared_ptr<PricingEngine>(
                 new FDEuropeanEngine<CrankNicolson>(bsmProcess,
                                                     timeSteps,timeSteps-1)));
        std::cout << std::setw(widths[0]) << std::left << method
                  << std::fixed
                  << std::setw(widths[1]) << std::left << europeanOption.NPV()
                  << std::setw(widths[2]) << std::left << europeanOption.delta()
                  << std::endl;

        // Binomial method: Jarrow-Rudd
        method = "Binomial Jarrow-Rudd";
        europeanOption.setPricingEngine(boost::shared_ptr<PricingEngine>(
                new BinomialVanillaEngine<JarrowRudd>(bsmProcess,timeSteps)));
        std::cout << std::setw(widths[0]) << std::left << method
                  << std::fixed
                  << std::setw(widths[1]) << std::left << europeanOption.NPV()
                  << std::setw(widths[2]) << std::left << europeanOption.delta()
                  << std::endl;

        method = "Binomial Cox-Ross-Rubinstein";
        europeanOption.setPricingEngine(boost::shared_ptr<PricingEngine>(
                      new BinomialVanillaEngine<CoxRossRubinstein>(bsmProcess,
                                                                   timeSteps)));
        std::cout << std::setw(widths[0]) << std::left << method
                  << std::fixed
                  << std::setw(widths[1]) << std::left << europeanOption.NPV()
                  << std::setw(widths[2]) << std::left << europeanOption.delta()
                  << std::endl;

        // Binomial method: Additive equiprobabilities
        method = "Additive equiprobabilities";
        europeanOption.setPricingEngine(boost::shared_ptr<PricingEngine>(
                new BinomialVanillaEngine<AdditiveEQPBinomialTree>(bsmProcess,
                                                                   timeSteps)));
        std::cout << std::setw(widths[0]) << std::left << method
                  << std::fixed
                  << std::setw(widths[1]) << std::left << europeanOption.NPV()
                  << std::setw(widths[2]) << std::left << europeanOption.delta()
                  << std::endl;

        // Binomial method: Binomial Trigeorgis
        method = "Binomial Trigeorgis";
        europeanOption.setPricingEngine(boost::shared_ptr<PricingEngine>(
                new BinomialVanillaEngine<Trigeorgis>(bsmProcess,timeSteps)));
        std::cout << std::setw(widths[0]) << std::left << method
                  << std::fixed
                  << std::setw(widths[1]) << std::left << europeanOption.NPV()
                  << std::setw(widths[2]) << std::left << europeanOption.delta()
                  << std::endl;

        // Binomial method: Binomial Tian
        method = "Binomial Tian";
        europeanOption.setPricingEngine(boost::shared_ptr<PricingEngine>(
                      new BinomialVanillaEngine<Tian>(bsmProcess,timeSteps)));
        std::cout << std::setw(widths[0]) << std::left << method
                  << std::fixed
                  << std::setw(widths[1]) << std::left << europeanOption.NPV()
                  << std::setw(widths[2]) << std::left << europeanOption.delta()
                  << std::endl;

        // Binomial method: Binomial Leisen-Reimer
        method = "Binomial Leisen-Reimer";
        europeanOption.setPricingEngine(boost::shared_ptr<PricingEngine>(
              new BinomialVanillaEngine<LeisenReimer>(bsmProcess,timeSteps)));
        std::cout << std::setw(widths[0]) << std::left << method
                  << std::fixed
                  << std::setw(widths[1]) << std::left << europeanOption.NPV()
                  << std::setw(widths[2]) << std::left << europeanOption.delta()
                  << std::endl;

        // Binomial method: Binomial Joshi
        method = "Binomial Joshi";
        europeanOption.setPricingEngine(boost::shared_ptr<PricingEngine>(
                    new BinomialVanillaEngine<Joshi4>(bsmProcess,timeSteps)));
        std::cout << std::setw(widths[0]) << std::left << method
                  << std::fixed
                  << std::setw(widths[1]) << std::left << europeanOption.NPV()
                  << std::setw(widths[2]) << std::left << europeanOption.delta()
                  << std::endl;

        // Monte Carlo Method: MC (crude)
        timeSteps = 1;
        method = "MC (crude)";
        Size mcSeed = 42;
        boost::shared_ptr<PricingEngine> mcengine1;
        mcengine1 = MakeMCEuropeanEngine<PseudoRandom>(bsmProcess)
            .withSteps(timeSteps)
            .withAbsoluteTolerance(0.02)
            .withSeed(mcSeed);
        europeanOption.setPricingEngine(mcengine1);
        // Real errorEstimate = europeanOption.errorEstimate();
        std::cout << std::setw(widths[0]) << std::left << method
                  << std::fixed
                  << std::setw(widths[1]) << std::left << europeanOption.NPV()
                  << std::setw(widths[2]) << std::left << europeanOption.delta()
                  << std::endl;

        // Monte Carlo Method: QMC (Sobol)
        method = "QMC (Sobol)";
        Size nSamples = 32768;  // 2^15

        boost::shared_ptr<PricingEngine> mcengine2;
        mcengine2 = MakeMCEuropeanEngine<LowDiscrepancy>(bsmProcess)
            .withSteps(timeSteps)
            .withSamples(nSamples);
        europeanOption.setPricingEngine(mcengine2);
        std::cout << std::setw(widths[0]) << std::left << method
                  << std::fixed
                  << std::setw(widths[1]) << std::left << europeanOption.NPV()
                  << std::setw(widths[2]) << std::left << europeanOption.delta()
                  << std::endl;

        // End test
        Real seconds = timer.elapsed();
        Integer hours = int(seconds/3600);
        seconds -= hours * 3600;
        Integer minutes = int(seconds/60);
        seconds -= minutes * 60;
        std::cout << " \nRun completed in ";
        if (hours > 0)
            std::cout << hours << " h ";
        if (hours > 0 || minutes > 0)
            std::cout << minutes << " m ";
        std::cout << std::fixed << std::setprecision(0)
                  << seconds << " s\n" << std::endl;
        return 0;

    } catch (std::exception& e) {
        std::cerr << e.what() << std::endl;
        return 1;
    } catch (...) {
        std::cerr << "unknown error" << std::endl;
        return 1;
    }
}

PiecewiseYieldCurve<Discount, LogLinear>* create_yield_curve() {

    Calendar calendar = UnitedStates();
    Date settlementDate(4, January, 2012);
    // must be a business day
    settlementDate = calendar.adjust(settlementDate);

    Integer fixingDays = 2;
    Date todaysDate = calendar.advance(settlementDate, -fixingDays, Days);

    Settings::instance().evaluationDate() = todaysDate;

    std::cout << "Today: " << todaysDate.weekday()
              << ", " << todaysDate << std::endl;

    std::cout << "Settlement date: " << settlementDate.weekday()
              << ", " << settlementDate << std::endl;

    // Data source: Bloomberg as of 12/30/2011
    // deposits
    Rate d1wQuote=0.00208200;
    Rate d2wQuote=0.00243500;
    Rate d1mQuote=0.00295300;
    Rate d2mQuote=0.00427100;
    Rate d3mQuote=0.00581000;
    // futures
    Real fut1Quote=99.2950;
    Real fut2Quote=99.2650;
    Real fut3Quote=99.2450;
    Real fut4Quote=99.2500;
    Real fut5Quote=99.2350;
    Real fut6Quote=99.2100;
    Real fut7Quote=99.1550;
    Real fut8Quote=99.1550;
    // swaps
    Rate s2yQuote=0.00726000;
    Rate s3yQuote=0.00820400;
    Rate s4yQuote=0.01006500;
    Rate s5yQuote=0.01224500;
    Rate s6yQuote=0.01449100;
    Rate s7yQuote=0.01643100;
    Rate s8yQuote=0.01802800;
    Rate s9yQuote=0.01932400;
    Rate s10yQuote=0.02027000;
    Rate s12yQuote=0.02227100;
    Rate s15yQuote=0.02401600;

    /*
    // Data source: Bloomberg as of 01/31/2012
    // deposits
    Rate d1wQuote=0.00196500;
    Rate d2wQuote=0.00225500;
    Rate d1mQuote=0.00264750;
    Rate d2mQuote=0.00394450;
    Rate d2mQuote=0.00000000;
    // futures
    Real fut1Quote=99.5450;
    Real fut2Quote=99.5400;
    Real fut3Quote=99.5200;
    Real fut4Quote=99.5000;
    Real fut5Quote=99.4950;
    Real fut6Quote=99.4750;
    Real fut7Quote=99.4450;
    Real fut8Quote=99.4000;
    // swaps
    Rate s2yQuote=0.00512500000;
    Rate s3yQuote=0.00590500000;
    Rate s4yQuote=0.00764500000;
    Rate s5yQuote=0.00988000000;
    Rate s6yQuote=0.01228000000;
    Rate s7yQuote=0.01449000000;
    Rate s8yQuote=0.01630500000;
    Rate s9yQuote=0.01782600000;
    Rate s10yQuote=0.01915000000;
    Rate s12yQuote=0.02126000000;
    Rate s15yQuote=0.02340300000;
    */


    /********************
     ***    QUOTES    ***
     ********************/

    // deposits
    boost::shared_ptr<Quote> d1wRate(new SimpleQuote(d1wQuote));
    boost::shared_ptr<Quote> d2wRate(new SimpleQuote(d2wQuote));
    boost::shared_ptr<Quote> d1mRate(new SimpleQuote(d1mQuote));
    boost::shared_ptr<Quote> d2mRate(new SimpleQuote(d2mQuote));
    boost::shared_ptr<Quote> d3mRate(new SimpleQuote(d3mQuote));
    // futures
    boost::shared_ptr<Quote> fut1Price(new SimpleQuote(fut1Quote));
    boost::shared_ptr<Quote> fut2Price(new SimpleQuote(fut2Quote));
    boost::shared_ptr<Quote> fut3Price(new SimpleQuote(fut3Quote));
    boost::shared_ptr<Quote> fut4Price(new SimpleQuote(fut4Quote));
    boost::shared_ptr<Quote> fut5Price(new SimpleQuote(fut5Quote));
    boost::shared_ptr<Quote> fut6Price(new SimpleQuote(fut6Quote));
    boost::shared_ptr<Quote> fut7Price(new SimpleQuote(fut7Quote));
    boost::shared_ptr<Quote> fut8Price(new SimpleQuote(fut8Quote));
    // swaps
    boost::shared_ptr<Quote> s2yRate(new SimpleQuote(s2yQuote));
    boost::shared_ptr<Quote> s3yRate(new SimpleQuote(s3yQuote));
    boost::shared_ptr<Quote> s4yRate(new SimpleQuote(s4yQuote));
    boost::shared_ptr<Quote> s5yRate(new SimpleQuote(s5yQuote));
    boost::shared_ptr<Quote> s6yRate(new SimpleQuote(s6yQuote));
    boost::shared_ptr<Quote> s7yRate(new SimpleQuote(s7yQuote));
    boost::shared_ptr<Quote> s8yRate(new SimpleQuote(s8yQuote));
    boost::shared_ptr<Quote> s9yRate(new SimpleQuote(s9yQuote));
    boost::shared_ptr<Quote> s10yRate(new SimpleQuote(s10yQuote));
    boost::shared_ptr<Quote> s12yRate(new SimpleQuote(s12yQuote));
    boost::shared_ptr<Quote> s15yRate(new SimpleQuote(s15yQuote));


    /*********************
     ***  RATE HELPERS ***
     *********************/

    // deposits
    DayCounter depositDayCounter = Actual360();

    boost::shared_ptr<RateHelper> d1w(new DepositRateHelper(
        Handle<Quote>(d1wRate),
        1*Weeks, fixingDays,
        calendar, ModifiedFollowing,
        true, depositDayCounter));
    boost::shared_ptr<RateHelper> d2w(new DepositRateHelper(
        Handle<Quote>(d2wRate),
        2*Weeks, fixingDays,
        calendar, ModifiedFollowing,
        true, depositDayCounter));
    boost::shared_ptr<RateHelper> d1m(new DepositRateHelper(
        Handle<Quote>(d1mRate),
        1*Months, fixingDays,
        calendar, ModifiedFollowing,
        true, depositDayCounter));
    boost::shared_ptr<RateHelper> d2m(new DepositRateHelper(
        Handle<Quote>(d2mRate),
        2*Months, fixingDays,
        calendar, ModifiedFollowing,
        true, depositDayCounter));
    boost::shared_ptr<RateHelper> d3m(new DepositRateHelper(
        Handle<Quote>(d3mRate),
        3*Months, fixingDays,
        calendar, ModifiedFollowing,
        true, depositDayCounter));

    // setup futures
    // Rate convexityAdjustment = 0.0;
    Integer futMonths = 3;
    Date imm = IMM::nextDate(settlementDate);
    std::cout << "imm1 = " << imm << std::endl;
    boost::shared_ptr<RateHelper> fut1(new FuturesRateHelper(
        Handle<Quote>(fut1Price),
        imm,
        futMonths, calendar, ModifiedFollowing,
        true, depositDayCounter));
    imm = IMM::nextDate(imm+1);
    std::cout << "imm2 = " << imm << std::endl;
    boost::shared_ptr<RateHelper> fut2(new FuturesRateHelper(
        Handle<Quote>(fut2Price),
        imm,
        futMonths, calendar, ModifiedFollowing,
        true, depositDayCounter));
    imm = IMM::nextDate(imm+1);
    std::cout << "imm3 = " << imm << std::endl;
    boost::shared_ptr<RateHelper> fut3(new FuturesRateHelper(
        Handle<Quote>(fut3Price),
        imm,
        futMonths, calendar, ModifiedFollowing,
        true, depositDayCounter));
    imm = IMM::nextDate(imm+1);
    std::cout << "imm4 = " << imm << std::endl;
    boost::shared_ptr<RateHelper> fut4(new FuturesRateHelper(
        Handle<Quote>(fut4Price),
        imm,
        futMonths, calendar, ModifiedFollowing,
        true, depositDayCounter));
    imm = IMM::nextDate(imm+1);
    std::cout << "imm5 = " << imm << std::endl;
    boost::shared_ptr<RateHelper> fut5(new FuturesRateHelper(
        Handle<Quote>(fut5Price),
        imm,
        futMonths, calendar, ModifiedFollowing,
        true, depositDayCounter));
    imm = IMM::nextDate(imm+1);
    std::cout << "imm6 = " << imm << std::endl;
    boost::shared_ptr<RateHelper> fut6(new FuturesRateHelper(
        Handle<Quote>(fut6Price),
        imm,
        futMonths, calendar, ModifiedFollowing,
        true, depositDayCounter));
    imm = IMM::nextDate(imm+1);
    std::cout << "imm7 = " << imm << std::endl;
    boost::shared_ptr<RateHelper> fut7(new FuturesRateHelper(
        Handle<Quote>(fut7Price),
        imm,
        futMonths, calendar, ModifiedFollowing,
        true, depositDayCounter));
    imm = IMM::nextDate(imm+1);
    std::cout << "imm8 = " << imm << std::endl;
    boost::shared_ptr<RateHelper> fut8(new FuturesRateHelper(
        Handle<Quote>(fut8Price),
        imm,
        futMonths, calendar, ModifiedFollowing,
        true, depositDayCounter));


    // setup swaps
    Frequency swFixedLegFrequency = Annual;
    BusinessDayConvention swFixedLegConvention = Unadjusted;
    DayCounter swFixedLegDayCounter = Thirty360(Thirty360::USA);
    RelinkableHandle<YieldTermStructure> liborTermStructure;
    boost::shared_ptr<IborIndex> swFloatingLegIndex(new USDLibor(Period(3, Months), liborTermStructure));

    boost::shared_ptr<RateHelper> s2y(new SwapRateHelper(
        Handle<Quote>(s2yRate), 2*Years,
        calendar, swFixedLegFrequency,
        swFixedLegConvention, swFixedLegDayCounter,
        swFloatingLegIndex));
    boost::shared_ptr<RateHelper> s3y(new SwapRateHelper(
        Handle<Quote>(s3yRate), 3*Years,
        calendar, swFixedLegFrequency,
        swFixedLegConvention, swFixedLegDayCounter,
        swFloatingLegIndex));
    boost::shared_ptr<RateHelper> s4y(new SwapRateHelper(
        Handle<Quote>(s4yRate), 4*Years,
        calendar, swFixedLegFrequency,
        swFixedLegConvention, swFixedLegDayCounter,
        swFloatingLegIndex));
    boost::shared_ptr<RateHelper> s5y(new SwapRateHelper(
        Handle<Quote>(s5yRate), 5*Years,
        calendar, swFixedLegFrequency,
        swFixedLegConvention, swFixedLegDayCounter,
        swFloatingLegIndex));
    boost::shared_ptr<RateHelper> s6y(new SwapRateHelper(
        Handle<Quote>(s6yRate), 6*Years,
        calendar, swFixedLegFrequency,
        swFixedLegConvention, swFixedLegDayCounter,
        swFloatingLegIndex));
    boost::shared_ptr<RateHelper> s7y(new SwapRateHelper(
        Handle<Quote>(s7yRate), 7*Years,
        calendar, swFixedLegFrequency,
        swFixedLegConvention, swFixedLegDayCounter,
        swFloatingLegIndex));
    boost::shared_ptr<RateHelper> s8y(new SwapRateHelper(
        Handle<Quote>(s8yRate), 8*Years,
        calendar, swFixedLegFrequency,
        swFixedLegConvention, swFixedLegDayCounter,
        swFloatingLegIndex));
    boost::shared_ptr<RateHelper> s9y(new SwapRateHelper(
        Handle<Quote>(s9yRate), 9*Years,
        calendar, swFixedLegFrequency,
        swFixedLegConvention, swFixedLegDayCounter,
        swFloatingLegIndex));
    boost::shared_ptr<RateHelper> s10y(new SwapRateHelper(
        Handle<Quote>(s10yRate), 10*Years,
        calendar, swFixedLegFrequency,
        swFixedLegConvention, swFixedLegDayCounter,
        swFloatingLegIndex));
    boost::shared_ptr<RateHelper> s12y(new SwapRateHelper(
        Handle<Quote>(s12yRate), 12*Years,
        calendar, swFixedLegFrequency,
        swFixedLegConvention, swFixedLegDayCounter,
        swFloatingLegIndex));
    boost::shared_ptr<RateHelper> s15y(new SwapRateHelper(
        Handle<Quote>(s15yRate), 15*Years,
        calendar, swFixedLegFrequency,
        swFixedLegConvention, swFixedLegDayCounter,
        swFloatingLegIndex));


    /*********************
     **  CURVE BUILDING **
     *********************/

    DayCounter termStructureDayCounter =
        ActualActual(ActualActual::ISDA);

    double tolerance = 1.0e-15;

    // A depo-futures-swap curve
    std::vector<boost::shared_ptr<RateHelper> > depoFutSwapInstruments;
    depoFutSwapInstruments.push_back(d1w);
    depoFutSwapInstruments.push_back(d2w);
    depoFutSwapInstruments.push_back(d1m);
    depoFutSwapInstruments.push_back(d2m);
    depoFutSwapInstruments.push_back(d3m);
    depoFutSwapInstruments.push_back(fut1);
    depoFutSwapInstruments.push_back(fut2);
    depoFutSwapInstruments.push_back(fut3);
    depoFutSwapInstruments.push_back(fut4);
    depoFutSwapInstruments.push_back(fut5);
    depoFutSwapInstruments.push_back(fut6);
    depoFutSwapInstruments.push_back(fut7);
    depoFutSwapInstruments.push_back(fut8);
    depoFutSwapInstruments.push_back(s3y);
    depoFutSwapInstruments.push_back(s4y);
    depoFutSwapInstruments.push_back(s5y);
    depoFutSwapInstruments.push_back(s6y);
    depoFutSwapInstruments.push_back(s7y);
    depoFutSwapInstruments.push_back(s8y);
    depoFutSwapInstruments.push_back(s9y);
    depoFutSwapInstruments.push_back(s10y);
    depoFutSwapInstruments.push_back(s12y);
    depoFutSwapInstruments.push_back(s15y);
    /*boost::shared_ptr<YieldTermStructure> depoFutSwapTermStructure(
        new PiecewiseYieldCurve<Discount, LogLinear>(
                                    settlementDate, depoFutSwapInstruments,
                                    termStructureDayCounter,
                                    tolerance));*/
    PiecewiseYieldCurve<Discount, LogLinear>* depoFutSwapTermStructure 
        = new PiecewiseYieldCurve<Discount, LogLinear>(
                                    settlementDate, depoFutSwapInstruments,
                                    termStructureDayCounter,
                                    tolerance);

    Date date1 = Date(29, May, 2012);
    Real df1 = depoFutSwapTermStructure->discount(date1);
    Date date2 = Date(16, Aug, 2013);
    Real df2 = depoFutSwapTermStructure->discount(date2);

    Size widths[] = { 35, 14, 14 };
    std::cout << std::setw(widths[0]) << std::left << "Date" 
              << std::setw(widths[1]) << std::left << "Disc. Factor" 
              << std::endl;
    std::cout << std::setw(widths[0]) << std::left << date1 
              << std::setw(widths[1]) << std::left << df1 
              << std::endl;
    std::cout << std::setw(widths[0]) << std::left << date2 
              << std::setw(widths[1]) << std::left << df2 
              << std::endl;

    return depoFutSwapTermStructure;
}

int main(int, char* []) {
    try {
        //auto yc = create_yield_curve();
        auto retval = calc_equityoption();

        return retval;
    } catch (std::exception& e) {
        std::cerr << e.what() << std::endl;
        return 1;
    } catch (...) {
        std::cerr << "unknown error" << std::endl;
        return 1;
    }
}
