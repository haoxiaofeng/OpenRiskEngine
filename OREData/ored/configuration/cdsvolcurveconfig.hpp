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

/*! \file ored/configuration/cdsvolcurveconfig.hpp
    \brief CDS volatility curve configuration classes
    \ingroup configuration
*/

#pragma once

#include <ored/configuration/curveconfig.hpp>
#include <ql/types.hpp>

using std::string;
using std::vector;
using ore::data::XMLNode;
using ore::data::XMLDocument;
using QuantLib::Period;

namespace ore {
namespace data {

//! CDS volatility structure configuration
/*!
  \ingroup configuration
*/
class CDSVolatilityCurveConfig : public CurveConfig {
public:
    //! \name Constructors/Destructors
    //@{
    //! Default constructor
    CDSVolatilityCurveConfig() {}
    //! Detailed constructor
    CDSVolatilityCurveConfig(const string& curveID, const string& curveDescription, const vector<string>& expiries);
    //! Default destructor
    virtual ~CDSVolatilityCurveConfig() {}
    //@}

    //! \name Serialisation
    //@{
    virtual void fromXML(XMLNode* node);
    virtual XMLNode* toXML(XMLDocument& doc);
    //@}

    //! \name Inspectors
    //@{
    const vector<string>& expiries() const { return expiries_; }
    const vector<string>& quotes() override;
    //@}

    //! \name Setters
    //@{
    vector<string>& expiries() { return expiries_; }
    //@}
private:
    vector<string> expiries_;
};
} // namespace data
} // namespace ore
