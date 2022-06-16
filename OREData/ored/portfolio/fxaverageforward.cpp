/*
 Copyright (C) 2022 Quaternion Risk Management Ltd
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

#include <boost/make_shared.hpp>
#include <ored/portfolio/builders/swap.hpp>
#include <ored/portfolio/enginefactory.hpp>
#include <ored/portfolio/fxaverageforward.hpp>
#include <ored/utilities/log.hpp>
#include <ored/utilities/parsers.hpp>
#include <ored/utilities/marketdata.hpp>
#include <ored/utilities/xmlutils.hpp>
#include <ql/cashflows/simplecashflow.hpp>
#include <qle/cashflows/fxlinkedcashflow.hpp>

namespace ore {
namespace data {

void FxAverageForward::build(const boost::shared_ptr<EngineFactory>& engineFactory) {
    LOG("FxAverageForward::build() called");
    QL_REQUIRE(!settlementCurrency_.empty(), "settlement currency must not be blank");
    QL_REQUIRE(!referenceCurrency_.empty(), "reference currency must not be blank");
    QL_REQUIRE(!fxIndex_.empty(), "fx index must not be blank");
    Currency refCcy = parseCurrency(referenceCurrency_);
    Currency payCcy = parseCurrency(settlementCurrency_);
    Date payDate = parseDate(paymentDate_);
    Schedule observationSchedule = makeSchedule(observationDates_);
    QL_REQUIRE(payDate >= observationSchedule.dates().back(),
	       "payment date >= last observation date required");
    
    QL_REQUIRE(!fxIndex_.empty(), "FX settlement index must be specified for averaging forwards");
    boost::shared_ptr<QuantExt::FxIndex> fxIndex = buildFxIndex(fxIndex_, payCcy.code(), refCcy.code(), engineFactory->market(),
								engineFactory->configuration(MarketContext::pricing));
    for (const auto& date : observationSchedule.dates()) {
	if (date <= Settings::instance().evaluationDate())
	    requiredFixings_.addFixingDate(date, fxIndex_, payDate);
    }

    // Set up Legs
    legs_ = {{boost::make_shared<SimpleCashFlow>(settlementNotional_, payDate)},
             {boost::make_shared<QuantExt::FXLinkedCashFlow>(payDate, observationSchedule.dates(), referenceNotional_, fxIndex)}}; 
    legCurrencies_ = {settlementCurrency_, settlementCurrency_};
    legPayers_ = {fixedPayer_, !fixedPayer_};

    // Set up instrument and engine
    boost::shared_ptr<QuantLib::Swap> swap(new QuantLib::Swap(legs_, legPayers_));
    boost::shared_ptr<EngineBuilder> builder = engineFactory->builder("Swap");
    boost::shared_ptr<SwapEngineBuilderBase> swapBuilder = boost::dynamic_pointer_cast<SwapEngineBuilderBase>(builder);
    QL_REQUIRE(swapBuilder, "No Builder found for Swap " << id());
    swap->setPricingEngine(swapBuilder->engine(payCcy));
    instrument_.reset(new VanillaInstrument(swap));
    
    npvCurrency_ = settlementCurrency_;
    notional_ = settlementNotional_;
    notionalCurrency_ = settlementCurrency_;
    maturity_ = payDate;

    additionalData_["settlementNotional"] = settlementNotional_;
    additionalData_["settlementCurrency"] = settlementCurrency_;
    additionalData_["referenceCurrency"] = referenceCurrency_;
    additionalData_["referenceNotional"] = referenceNotional_;
    LOG("FxAverageForward::build() done");
}

void FxAverageForward::fromXML(XMLNode* node) {
    Trade::fromXML(node);
    XMLNode* fxNode = XMLUtils::getChildNode(node, "FxAverageForwardData");
    QL_REQUIRE(fxNode, "No FxAverageForwardData Node");
    paymentDate_ = XMLUtils::getChildValue(fxNode, "PaymentDate", true);
    XMLNode* tmp = XMLUtils::getChildNode(fxNode, "ObservationDates");
    QL_REQUIRE(tmp, "No ObservationDates provided in FxAverageForwardData");
    observationDates_.fromXML(tmp);
    fixedPayer_ = XMLUtils::getChildValueAsBool(fxNode, "FixedPayer", true);
    referenceCurrency_ = XMLUtils::getChildValue(fxNode, "ReferenceCurrency", true);
    referenceNotional_ = XMLUtils::getChildValueAsDouble(fxNode, "ReferenceNotional", true);
    settlementCurrency_ = XMLUtils::getChildValue(fxNode, "SettlementCurrency", true);
    settlementNotional_ = XMLUtils::getChildValueAsDouble(fxNode, "SettlementNotional", true);
    fxIndex_ = XMLUtils::getChildValue(fxNode, "FXIndex", true);
    settlement_ = XMLUtils::getChildValue(fxNode, "Settlement", false);
    if (settlement_ == "")
        settlement_ = "Cash";
}

XMLNode* FxAverageForward::toXML(XMLDocument& doc) {
    XMLNode* node = Trade::toXML(doc);
    XMLNode* fxNode = doc.allocNode("FxAverageForwardData");
    XMLUtils::appendNode(node, fxNode);

    XMLNode* schedNode = observationDates_.toXML(doc);
    XMLUtils::setNodeName(doc, schedNode, "ObservationDates");
    XMLUtils::appendNode(fxNode, schedNode);

    XMLUtils::addChild(doc, fxNode, "PaymentDate", paymentDate_);
    XMLUtils::addChild(doc, fxNode, "FixedPayer", std::to_string(fixedPayer_));
    XMLUtils::addChild(doc, fxNode, "ReferenceCurrency", referenceCurrency_);
    XMLUtils::addChild(doc, fxNode, "ReferenceNotional", referenceNotional_);
    XMLUtils::addChild(doc, fxNode, "SettlementCurrency", settlementCurrency_);
    XMLUtils::addChild(doc, fxNode, "SettlementNotional", settlementNotional_);
    XMLUtils::addChild(doc, fxNode, "FXIndex", fxIndex_);
    XMLUtils::addChild(doc, fxNode, "Settlement", settlement_);

    return node;
}
} // namespace data
} // namespace ore
