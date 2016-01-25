/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2012 - 2015 Quaternion Risk Management Ltd.
 All rights reserved
*/

/*! \file oiccbasisswaphelper.hpp
    \brief Overnight Indexed Cross Currency Basis Swap helpers
*/

#ifndef quantlib_oiccbasisswap_helper_hpp
#define quantlib_oiccbasisswap_helper_hpp

#include <ql/termstructures/yield/ratehelpers.hpp>
#include <ql/instruments/overnightindexedswap.hpp>
#include <qle/instruments/oiccbasisswap.hpp>

namespace QuantLib {

    //! Rate helper for bootstrapping over Overnight Indexed CC Basis Swap Spreads
    /*
      The bootstrap affects the receive leg's discount curve only.
    */
    class OICCBSHelper : public RelativeDateRateHelper {
      public:
        OICCBSHelper(Natural settlementDays,
                     const Period& term, // swap maturity
                     const boost::shared_ptr<OvernightIndex>& payIndex,
                     const Period& payTenor,
                     const boost::shared_ptr<OvernightIndex>& recIndex,
                     const Period& recTenor,
                     const Handle<Quote>& spreadQuote,
                     const Handle<YieldTermStructure>& fixedDiscountCurve,
                     bool spreadQuoteOnPayLeg,
                     bool fixedDiscountOnPayLeg
                     );
        //! \name RateHelper interface
        //@{
        Real impliedQuote() const;
        void setTermStructure(YieldTermStructure*);
        //@}
        //! \name inspectors
        //@{
        boost::shared_ptr<OvernightIndexedCrossCcyBasisSwap> swap() const { return swap_; }
        //@}
        //! \name Visitability
        //@{
        void accept(AcyclicVisitor&);
        //@}
    protected:
        void initializeDates();

        Natural settlementDays_;
        Period term_;
        boost::shared_ptr<OvernightIndex> payIndex_;
        Period payTenor_;
        boost::shared_ptr<OvernightIndex> recIndex_;
        Period recTenor_;
        Handle<YieldTermStructure> fixedDiscountCurve_;
        bool spreadQuoteOnPayLeg_;
        bool fixedDiscountOnPayLeg_;

        boost::shared_ptr<OvernightIndexedCrossCcyBasisSwap> swap_;
        RelinkableHandle<YieldTermStructure> termStructureHandle_;
    };
}

#endif
