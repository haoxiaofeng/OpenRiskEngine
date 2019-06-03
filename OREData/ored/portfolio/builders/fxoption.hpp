/*
 Copyright (C) 2016 Quaternion Risk Management Ltd
 All rights reserved.

 This file is part of ORE, a free-software/open-source library
 for transparent pricing and risk analysis - http://opensourcerisk.org

 ORE is free software: you can redistribute it and/or modify it
 under the terms of the Modified BSD License.  You should have received a
 copy of the license along with this program.
 The license is also available online at <http://opensourcerisk.org>

 This program is distributed on the basis that it will form a useful
 contribution to risk analytics and model standardisation, but WITHOUT
 ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 FITNESS FOR A PARTICULAR PURPOSE. See the license for more details.
*/

/*! \file portfolio/builders/fxoption.hpp
    \brief
    \ingroup builders
*/

#pragma once

#include <boost/make_shared.hpp>
#include <ored/portfolio/builders/cachingenginebuilder.hpp>
#include <ored/portfolio/enginefactory.hpp>
#include <ql/methods/finitedifferences/expliciteuler.hpp>
#include <ql/methods/finitedifferences/impliciteuler.hpp>
#include <ql/pricingengines/vanilla/analyticeuropeanengine.hpp>
#include <ql/pricingengines/vanilla/fdamericanengine.hpp>
#include <ql/pricingengines/vanilla/baroneadesiwhaleyengine.hpp>
#include <ql/processes/blackscholesprocess.hpp>

namespace ore {
namespace data {

//! Abstract Engine Builder for FX Options
/*! Pricing engines are cached by currency pair

    \ingroup builders
 */
class FxOptionEngineBuilder : public CachingPricingEngineBuilder<string, const Currency&, const Currency&> {
public:
    FxOptionEngineBuilder(const string& model, const string& engine, const set<string>& tradeTypes)
        : CachingEngineBuilder(model, engine, tradeTypes) {}

protected:
    virtual string keyImpl(const Currency& forCcy, const Currency& domCcy) override {
        return forCcy.code() + domCcy.code();
    }

    boost::shared_ptr<GeneralizedBlackScholesProcess> getBlackScholesProcess(const Currency& forCcy, const Currency& domCcy) {
        string pair = keyImpl(forCcy, domCcy);
        return boost::make_shared<GeneralizedBlackScholesProcess>(
            market_->fxSpot(pair, configuration(ore::data::MarketContext::pricing)),
            market_->discountCurve(forCcy.code(),
                                   configuration(ore::data::MarketContext::pricing)), // dividend yield ~ foreign yield
            market_->discountCurve(domCcy.code(), configuration(ore::data::MarketContext::pricing)),
            market_->fxVol(pair, configuration(ore::data::MarketContext::pricing))
            );
    }
};

//! Engine Builder for European FX Options
/*! Pricing engines are cached by currency pair

    \ingroup builders
 */
class FxEuropeanOptionEngineBuilder : public FxOptionEngineBuilder {
public:
    FxEuropeanOptionEngineBuilder() : FxOptionEngineBuilder("GarmanKohlhagen", "AnalyticEuropeanEngine", {"FxOption"}) {}

protected:
    virtual boost::shared_ptr<PricingEngine> engineImpl(const Currency& forCcy, const Currency& domCcy) override {
        boost::shared_ptr<GeneralizedBlackScholesProcess> gbsp = getBlackScholesProcess(forCcy, domCcy);
        return boost::make_shared<AnalyticEuropeanEngine>(gbsp);
    }
};

//! Engine Builder for FX American Options
/*! Pricing engines are cached by currency pair

    \ingroup portfolio
 */
class FxAmericanOptionEngineBuilder : public FxOptionEngineBuilder {
protected:
    FxAmericanOptionEngineBuilder(const string& model, const string& engine)
        : FxOptionEngineBuilder(model, engine, {"FxAmericanOption"}) {}
};

class FxAmericanOptionFDEngineBuilder
    : public FxAmericanOptionEngineBuilder {
public:
    FxAmericanOptionFDEngineBuilder()
        : FxAmericanOptionEngineBuilder("GarmanKohlhagen", "FDAmericanEngine") {}

protected:

    virtual boost::shared_ptr<PricingEngine> engineImpl(const Currency& forCcy, const Currency& domCcy) override {
        boost::shared_ptr<GeneralizedBlackScholesProcess> gbsp = getBlackScholesProcess(forCcy, domCcy);
        std::string scheme = engineParameter("Scheme");
        Size tGrid = ore::data::parseInteger(engineParameter("TimeGrid"));
        Size xGrid = ore::data::parseInteger(engineParameter("XGrid"));
        bool timeDependent = ore::data::parseBool(engineParameter("TimeDependent"));

        if (scheme == "CrankNicolson")
                return boost::make_shared<FDAmericanEngine<CrankNicolson>>(gbsp, tGrid, xGrid, timeDependent);
        if (scheme == "ExplicitEuler")
                return boost::make_shared<FDAmericanEngine<ExplicitEuler>>(gbsp, tGrid, xGrid, timeDependent);
        if (scheme == "ImplicitEuler")
                return boost::make_shared<FDAmericanEngine<ImplicitEuler>>(gbsp, tGrid, xGrid, timeDependent);
        QL_FAIL("unknown scheme for finite difference method: " << scheme);
    }

};

class FxAmericanOptionBAWApproxEngineBuilder
    : public FxAmericanOptionEngineBuilder {
public:
    FxAmericanOptionBAWApproxEngineBuilder()
        : FxAmericanOptionEngineBuilder("GarmanKohlhagen", "BaroneAdesiWhaleyApproximationEngine") {}

protected:

    virtual boost::shared_ptr<PricingEngine> engineImpl(const Currency& forCcy, const Currency& domCcy) override {
        boost::shared_ptr<GeneralizedBlackScholesProcess> gbsp = getBlackScholesProcess(forCcy, domCcy);
        return boost::make_shared<BaroneAdesiWhaleyApproximationEngine>(gbsp);
    }

};

} // namespace data
} // namespace ore
