/*-----------------------------------------------------------------------------
 This source file is part of Hopsan NG

 Copyright (c) 2011 
    Mikael Axin, Robert Braun, Alessandro Dell'Amico, Björn Eriksson,
    Peter Nordin, Karl Pettersson, Petter Krus, Ingo Staack

 This file is provided "as is", with no guarantee or warranty for the
 functionality or reliability of the contents. All contents in this file is
 the original work of the copyright holders at the Division of Fluid and
 Mechatronic Systems (Flumes) at Linköping University. Modifying, using or
 redistributing any part of this file is prohibited without explicit
 permission from the copyright holders.
-----------------------------------------------------------------------------*/

//!
//! @file   HydraulicOverCenterValve.hpp
//! @author Mikael Axin <mikael.axin@liu.se>
//! @date   2010-01-13
//!
//! @brief Contains a hydraulic over center valve with first order dynamics
//!
//$Id$

#ifndef HYDRAULICOVERCENTERVALVE_HPP_INCLUDED
#define HYDRAULICOVERCENTERVALVE_HPP_INCLUDED

#include <iostream>
#include <sstream>
#include "ComponentEssentials.h"
#include "ComponentUtilities.h"

namespace hopsan {

    //!
    //! @brief
    //! @ingroup HydraulicComponents
    //!
    class HydraulicOverCenterValve : public ComponentQ
    {
    private:
        double *mpPref, *mpPh, *mpArat;
        double x0, x0max, tao, Kcs, Kcf, Cs, Cf, pnom, qnom;
        double mPrevX0;
        TurbulentFlowFunction mTurb;
        ValveHysteresis mHyst;
        FirstOrderTransferFunction mFilterLP;

        double *mpND_p1, *mpND_q1, *mpND_c1, *mpND_Zc1, *mpND_p2, *mpND_q2, *mpND_c2, *mpND_Zc2,
               *mpND_p_control, *mpND_c_control;

        Port *mpP1, *mpP2, *mpP_CONTROL;

    public:
        static Component *Creator()
        {
            return new HydraulicOverCenterValve();
        }

        void configure()
        {
            mpP1 = addPowerPort("P1", "NodeHydraulic");
            mpP2 = addPowerPort("P2", "NodeHydraulic");
            mpP_CONTROL = addPowerPort("P_CONTROL", "NodeHydraulic");

            addInputVariable("p_ref", "Reference Opening Pressure", "Pa", 2000000, &mpPref);
            addInputVariable("p_h", "Hysteresis Width", "Pa", 500000, &mpPh);
            addInputVariable("a_ratio", "Area ratio", "-", 5.0, &mpArat);

            addConstant("tao", "Time Constant of Spool", "s", 0.01, tao);
            addConstant("k_cs", "Steady State Characteristic due to Spring", "(m^3/s)/Pa", 0.00000001, Kcs);
            addConstant("k_cf", "Steady State Characteristic due to Flow Forces", "(m^3/s)/Pa", 0.00000001, Kcf);
            addConstant("q_nom", "Flow with Fully Open Valve and pressure drop p_nom", "m^3/s", 0.001, qnom);
            addConstant("p_nom", "Nominal pressure drop", "Pa", 7e6, pnom);
        }


        void initialize()
        {
            mpND_p1 = getSafeNodeDataPtr(mpP1, NodeHydraulic::Pressure);
            mpND_q1 = getSafeNodeDataPtr(mpP1, NodeHydraulic::Flow);
            mpND_c1 = getSafeNodeDataPtr(mpP1, NodeHydraulic::WaveVariable);
            mpND_Zc1 = getSafeNodeDataPtr(mpP1, NodeHydraulic::CharImpedance);

            mpND_p2 = getSafeNodeDataPtr(mpP2, NodeHydraulic::Pressure);
            mpND_q2 = getSafeNodeDataPtr(mpP2, NodeHydraulic::Flow);
            mpND_c2 = getSafeNodeDataPtr(mpP2, NodeHydraulic::WaveVariable);
            mpND_Zc2 = getSafeNodeDataPtr(mpP2, NodeHydraulic::CharImpedance);

            mpND_p_control = getSafeNodeDataPtr(mpP_CONTROL, NodeHydraulic::Pressure);
            mpND_c_control = getSafeNodeDataPtr(mpP_CONTROL, NodeHydraulic::WaveVariable);

            x0 = 0.00001;           //Why not 0.0? (probably doesnt matter though)
            mPrevX0 = 0.0;
            Cs = sqrt(pnom)/Kcs;
            Cf = 1/(Kcf * sqrt(pnom));
            x0max = qnom/sqrt(pnom);

            double wCutoff = 1 / tao;
            double num[2] = {1.0, 0.0};
            double den[2] = {1.0, 1.0/wCutoff};
            mFilterLP.initialize(mTimestep, num, den, 0.0, 0.0, 0, x0max);
        }


        void simulateOneTimestep()
        {
            //Declare local variables
            double p1, q1, c1, Zc1, p2, q2, c2, Zc2, p_control, c_control;
            double b1, xs, xh, xsh, pref, ph, arat;
            bool cav = false;

            //Get variable values from nodes
            p1 = (*mpND_p1);
            q1 = (*mpND_q1);
            c1 = (*mpND_c1);
            Zc1 = (*mpND_Zc1);
            p2 = (*mpND_p2);
            q2 = (*mpND_q2);
            c2 = (*mpND_c2);
            Zc2 = (*mpND_Zc2);
            p_control = (*mpND_p_control);
            c_control = (*mpND_c_control);
            pref = (*mpPref);
            ph = (*mpPh);
            arat = (*mpArat);

            /* Equations */

            b1 = Cs+Cf*(p1-p2);                                 //Help Variable, equals sqrt(p1-p2)/Kctot
            xs = (p1 / arat - pref + p_control) / b1;                // Spool position calculation
            xh = ph/b1;
            xsh = mHyst.getValue(xs, xh, mPrevX0);              //Hysteresis
            x0 = mFilterLP.update(xsh);                         //Dynamics
            mTurb.setFlowCoefficient(x0);                       // Turbulent Flow Calculation
            q2 = mTurb.getFlow(c1, c2, Zc1, Zc2);
            q1 = -q2;

            p1 = c1 + Zc1 * q1;                                 // Pressure Calulation
            p2 = c2 + Zc2 * q2;
            p_control = c_control;

            p_control = std::max(0.0, p_control);

            if (p1 < 0.0)                                       // Check for cavitation
            {
                c1 = 0.0;
                Zc1 = 0.0;
                cav = true;
            }
            if (p2 < 0.0)
            {
                c2 = 0.0;
                Zc2 = 0.0;
                cav = true;
            }
            if (cav)                                            //Cavitatiaon, redo calculations with new c and Zc
            {
                q2 = mTurb.getFlow(c1, c2, Zc1, Zc2);
                q1 = -q2;
                p1 = c1 + Zc1 * q1;
                p2 = c2 + Zc2 * q2;
                if (p1 < 0.0) { p1 = 0.0; }
                if (p2 < 0.0) { p2 = 0.0; }
            }

            mPrevX0 = x0;

            (*mpND_p1) = p1;                                    //Write new values to nodes
            (*mpND_q1) = q1;
            (*mpND_p2) = p2;
            (*mpND_q2) = q2;
            (*mpND_p_control) = p_control;
        }
    };
}

#endif // HYDRAULICPRESSURECONTROLLEDVALVE_HPP_INCLUDED
