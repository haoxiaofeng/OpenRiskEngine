/*
 Copyright (C) 2017 Quaternion Risk Management Ltd
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

/*! \file scenario/sensitivityscenariogenerator.hpp
    \brief Sensitivity scenario generation
    \ingroup scenario
*/

#pragma once

#include <ored/marketdata/market.hpp>
#include <orea/scenario/scenariogenerator.hpp>
#include <orea/scenario/scenariofactory.hpp>
#include <orea/scenario/scenariosimmarket.hpp>
#include <orea/scenario/sensitivityscenariodata.hpp>
#include <orea/scenario/shiftscenariogenerator.hpp>

namespace ore {
using namespace data;
namespace analytics {

//! Sensitivity Scenario Generator
/*!
  This class builds a vector of sensitivity scenarios based on instructions in SensitivityScenarioData
  and ScenarioSimMarketParameters objects passed.

  The ScenarioSimMarketParameters object determines the scope and structure of a "simulation"
  market (currencies, currency pairs, curve tenor points, vol matrix expiries and terms/strikes etc)
  to which sensitivity scenarios are applied in order to compute their NPV impact.

  The SensitivityScenarioData object determines the structure of shift curves (shift tenor points
  can differ from the simulation market's tenor points), as well as type (relative/absolute) and size
  of shifts applied.

  The generator then produces comprehensive scenarios that can be applied to the simulation market,
  i.e. covering all quotes in the simulation market, possibly filled with "base" scenario values.

  Both UP and DOWN shifts are generated in order to facilitate delta and gamma calculation.

  The generator currently covers the IR/FX asset class, with shifts for the following term
  structure types:
  - FX spot rates
  - FX ATM volatilities
  - Discount curve zero rates
  - Index curve zero rates
  - Yield curve zero rates
  - Swaption ATM volatility matrices
  - Cap/Floor volatility matrices (by expiry and strike)

  Note:
  - For yield curves, the class generates sensitivites in the Zero rate domain only.
  Conversion into par rate sensivities has to be implemented as a postprocessor step.
  - Likewise, Cap/Floor volatilitiy sensitivties are computed in the optionlet domain.
  Conversion into par (flat cap/floor) volatility sensis has to be implemented as a
  postprocessor step.

  \ingroup scenario
 */
class SensitivityScenarioGenerator : public ShiftScenarioGenerator {
public:
    //! Constructor
    SensitivityScenarioGenerator(boost::shared_ptr<ScenarioFactory> scenarioFactory,
				 boost::shared_ptr<SensitivityScenarioData> sensitivityData,
				 boost::shared_ptr<ScenarioSimMarketParameters> simMarketData, QuantLib::Date today,
                                 boost::shared_ptr<ore::data::Market> initMarket,
                                 const std::string& configuration = Market::defaultConfiguration);
    //! Default destructor
    ~SensitivityScenarioGenerator(){};

private:
    void generateYieldCurveScenarios(bool up, boost::shared_ptr<Market> market);
    void generateDiscountCurveScenarios(bool up, boost::shared_ptr<Market> market);
    void generateIndexCurveScenarios(bool up, boost::shared_ptr<Market> market);
    void generateFxScenarios(bool up, boost::shared_ptr<Market> market);
    void generateSwaptionVolScenarios(bool up, boost::shared_ptr<Market> market);
    void generateFxVolScenarios(bool up, boost::shared_ptr<Market> market);
    void generateCapFloorVolScenarios(bool up, boost::shared_ptr<Market> market);

    boost::shared_ptr<SensitivityScenarioData> sensitivityData_;
};
}
}
