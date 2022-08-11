/*
  Copyright (C) 2020 Skandinaviska Enskilda Banken AB (publ)
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

/*! \file portfolio/asianoption.hpp
    \brief Asian Option data model
    \ingroup tradedata
*/

#pragma once

#include <ored/portfolio/optionasiandata.hpp>
#include <ored/portfolio/optiondata.hpp>
#include <ored/portfolio/trade.hpp>
#include <ored/portfolio/tradestrike.hpp>
#include <ored/utilities/parsers.hpp>
#include <ql/instruments/averagetype.hpp>

namespace ore {
namespace data {
using std::string;

//! Serializable Asian Option
/*!
  \ingroup tradedata
*/
class AsianOption : public Trade {
public:
    explicit AsianOption(const string& tradeType) : Trade(tradeType) {}
    AsianOption(const Envelope& env, const string& tradeType, double quantity, const TradeStrike& strike,
                const OptionData& option, const OptionAsianData& asianData, const ScheduleData& observationDates,
                const boost::shared_ptr<Underlying>& underlying, const Date& settlementDate)
        : Trade(tradeType, env), quantity_(quantity), strike_(strike), option_(option), asianData_(asianData),
          observationDates_(observationDates), underlying_(underlying), settlementDate_(settlementDate) {}

    //! Build QuantLib/QuantExt instrument, link pricing engine
    void build(const boost::shared_ptr<EngineFactory>&) override;

    //! \name Serialisation
    //@{
    virtual void fromXML(XMLNode* node) override;
    virtual XMLNode* toXML(XMLDocument& doc) override;
    //@}

    //! \name Inspectors
    //@{
    const string& asset() const { return assetName_; }
    double strike() const { return strikeValue_; }
    double quantity() const { return quantity_; }
    const OptionData& option() const { return option_; }
    const ScheduleData& observationDates() const { return observationDates_; }
    const Date& settlementDate() const { return settlementDate_; }
    const string& payCurrency() const { return currency_; }
    const string& indexName() const { return indexName_; }
    //@}

    // underlying asset names
    std::map<AssetClass, std::set<std::string>>
    underlyingIndices(const boost::shared_ptr<ReferenceDataManager>& referenceDataManager = nullptr) const override;

protected:
    void populateIndexName() const;

    double quantity_ = 0.0;
    TradeStrike strike_;
    OptionData option_;
    OptionAsianData asianData_;
    ScheduleData observationDates_;
    boost::shared_ptr<Underlying> underlying_;
    Date settlementDate_;

    string currency_;
    string strikeStr_;
    string assetName_;
    double strikeValue_ = 0.0;

    mutable string indexName_;
};

class EquityAsianOption : public AsianOption {
public:
    EquityAsianOption() : AsianOption("EquityAsianOption") {}
    using AsianOption::AsianOption;
};

class FxAsianOption : public AsianOption {
public:
    FxAsianOption() : AsianOption("FxAsianOption") {}
    using AsianOption::AsianOption;
};

class CommodityAsianOption : public AsianOption {
public:
    CommodityAsianOption() : AsianOption("CommodityAsianOption") {}
    using AsianOption::AsianOption;
};

} // namespace data
} // namespace ore
