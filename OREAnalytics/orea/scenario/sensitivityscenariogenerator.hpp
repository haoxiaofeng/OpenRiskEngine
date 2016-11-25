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

namespace ore {
using namespace data;
namespace analytics {

//! Sensitivity Scenario Generator
/*!

  \ingroup scenario
 */
class SensitivityScenarioGenerator : public ScenarioGenerator {
public:
    enum class ShiftType { Absolute, Relative };

    //! Constructor
    SensitivityScenarioGenerator(boost::shared_ptr<ScenarioFactory> scenarioFactory,
                                 boost::shared_ptr<SensitivityScenarioData> sensitivityData,
                                 boost::shared_ptr<ScenarioSimMarketParameters> simMarketData, QuantLib::Date today,
                                 boost::shared_ptr<ore::data::Market> initMarket,
                                 const std::string& configuration = Market::defaultConfiguration);
    //! Default destructor
    ~SensitivityScenarioGenerator(){};

    //! Scenario Generator interface
    //@{
    boost::shared_ptr<Scenario> next(const Date& d);
    void reset() { counter_ = 0; }
    //@}

    //! Inspectors
    //@{
    //! Number of sensitivity scenarios
    Size samples() { return scenarios_.size(); }
    //! Return the base scenario, i.e. cached initial values of all relevant market points
    const boost::shared_ptr<Scenario>& baseScenario() { return baseScenario_; }
    //! Return vector of sensitivity scenarios, scenario 0 is the base scenario
    const std::vector<boost::shared_ptr<Scenario> >& scenarios() { return scenarios_; }
    //@}

    //! Clear the caches for base scenario keys and values
    void clear();

    //! Set up the "base" scenario and all shift scenarios using market points/values from the market object
    void init(boost::shared_ptr<Market> market);

    //! Apply 1d triangular shift to 1d data such as yield curves, public to allow test suite access
    /*!
      Apply triangular shaped shifts to the underlying curve where the triangle
      reaches from the previous to the next shift tenor point with peak at the
      current shift tenor point.
      At the initial and final shift tenor the shape is replaced such that the
      full shift is applied to all curve grid points to the left of the first
      shift point and to the right of the last shift point, respectively.
      The procedure guarantees that no sensitivity to original curve points is
      "missed" when the shift curve is less granular, e.g.
      original curve |...|...|...|...|...|...|...|...|...|
      shift curve    ......|...........|...........|......
     */
    void applyShift( //! Number of the shift curve tenor point to be shifted here
        Size j,
        //! Shift size interpreted as either absolute or relative shift
        Real shiftSize,
        //! Absolute: newValue = oldValue + shiftSize. Relative: newValue = oldValue * (1 + shiftSize)
        ShiftType type,
        //! Shift tenors expressed as times
        const vector<Time>& shiftTimes,
        //! Input curve values such as zero rates
        const vector<Real>& values,
        //! Tenor points of the input curve, expressed as times
        const vector<Time>& times,
        //! Resulting shifted curve with same tenor structure as the input curve
        vector<Real>& shiftedValues);

    //! Apply 2d shift to 2d matrix such as swaption volatilities, public to allow test suite access
    /*! This is the 2d generalisation of the 1d version of applyShift()
     */
    void applyShift(Size j, Size k, Real shiftSize, ShiftType type, const vector<Time>& shiftX,
                    const vector<Time>& shiftY, const vector<Time>& dataX, const vector<Time>& dataY,
                    const vector<vector<Real> >& data, vector<vector<Real> >& shiftedData);

private:
    void addCacheTo(boost::shared_ptr<Scenario> scenario);
    void generateYieldCurveScenarios(boost::shared_ptr<Market> market);
    void generateDiscountCurveScenarios(boost::shared_ptr<Market> market);
    void generateIndexCurveScenarios(boost::shared_ptr<Market> market);
    void generateFxScenarios(boost::shared_ptr<Market> market);
    void generateSwaptionVolScenarios(boost::shared_ptr<Market> market);
    void generateFxVolScenarios(boost::shared_ptr<Market> market);
    void generateCapFloorVolScenarios(boost::shared_ptr<Market> market);

    boost::shared_ptr<ScenarioFactory> scenarioFactory_;
    boost::shared_ptr<SensitivityScenarioData> sensitivityData_;
    boost::shared_ptr<ScenarioSimMarketParameters> simMarketData_;
    Date today_;
    boost::shared_ptr<ore::data::Market> initMarket_;
    const std::string configuration_;
    std::vector<RiskFactorKey> discountCurveKeys_, indexCurveKeys_, fxKeys_, swaptionVolKeys_, fxVolKeys_,
        optionletVolKeys_;
    std::map<RiskFactorKey, Real> discountCurveCache_, indexCurveCache_, fxCache_, swaptionVolCache_, fxVolCache_,
        optionletVolCache_;
    std::vector<boost::shared_ptr<Scenario> > scenarios_;
    boost::shared_ptr<Scenario> baseScenario_;
    Size counter_;
};
}
}
