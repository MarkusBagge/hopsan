#ifndef MECHANICSPRING_HPP_INCLUDED
#define MECHANICSPRING_HPP_INCLUDED

#include <iostream>
#include "ComponentEssentials.h"
#include "ComponentUtilities.h"
#include "math.h"

//!
//! @file MechanicSpring.hpp
//! @author Petter Krus <petter.krus@liu.se>
//  co-author/auditor Peter Nordin <peter.nordin@liu.se>
//! @date Mon 18 Apr 2016 17:13:06
//! @brief Linear spring
//! @ingroup MechanicComponents
//!
//==This code has been autogenerated using Compgen==
//from 
/*{, C:, HopsanTrunk, componentLibraries, defaultLibrary, Mechanic, \
Linear}/MechanicSpring.nb*/

using namespace hopsan;

class MechanicSpring : public ComponentC
{
private:
     double Ks;
     double x0;
     double eps;
     double alpha;
     Port *mpPm1;
     Port *mpPm2;
     int mNstep;
     //Port Pm1 variable
     double fm1;
     double xm1;
     double vm1;
     double cm1;
     double Zcm1;
     double eqMassm1;
     //Port Pm2 variable
     double fm2;
     double xm2;
     double vm2;
     double cm2;
     double Zcm2;
     double eqMassm2;
//==This code has been autogenerated using Compgen==
     //inputVariables
     //outputVariables
     //InitialExpressions variables
     double fak;
     double Zexpr;
     double cm1f;
     double cm2f;
     //Expressions variables
     //Port Pm1 pointer
     double *mpPm1_f;
     double *mpPm1_x;
     double *mpPm1_v;
     double *mpPm1_c;
     double *mpPm1_Zc;
     double *mpPm1_eqMass;
     //Port Pm2 pointer
     double *mpPm2_f;
     double *mpPm2_x;
     double *mpPm2_v;
     double *mpPm2_c;
     double *mpPm2_Zc;
     double *mpPm2_eqMass;
     //Delay declarations
//==This code has been autogenerated using Compgen==
     //inputVariables pointers
     //inputParameters pointers
     double *mpKs;
     double *mpx0;
     double *mpeps;
     double *mpalpha;
     //outputVariables pointers
     EquationSystemSolver *mpSolver;

public:
     static Component *Creator()
     {
        return new MechanicSpring();
     }

     void configure()
     {
//==This code has been autogenerated using Compgen==

        mNstep=9;

        //Add ports to the component
        mpPm1=addPowerPort("Pm1","NodeMechanic");
        mpPm2=addPowerPort("Pm2","NodeMechanic");
        //Add inputVariables to the component

        //Add inputParammeters to the component
            addInputVariable("Ks", "Spring constant", "N/m", 100.,&mpKs);
            addInputVariable("x0", "free length of spring", "m", 0.,&mpx0);
            addInputVariable("eps", "Num drift remove coeff", "", \
0.005,&mpeps);
            addInputVariable("alpha", "numerical damping", "", \
0.05,&mpalpha);
        //Add outputVariables to the component

//==This code has been autogenerated using Compgen==
        //Add constantParameters
     }

    void initialize()
     {
        //Read port variable pointers from nodes
        //Port Pm1
        mpPm1_f=getSafeNodeDataPtr(mpPm1, NodeMechanic::Force);
        mpPm1_x=getSafeNodeDataPtr(mpPm1, NodeMechanic::Position);
        mpPm1_v=getSafeNodeDataPtr(mpPm1, NodeMechanic::Velocity);
        mpPm1_c=getSafeNodeDataPtr(mpPm1, NodeMechanic::WaveVariable);
        mpPm1_Zc=getSafeNodeDataPtr(mpPm1, NodeMechanic::CharImpedance);
        mpPm1_eqMass=getSafeNodeDataPtr(mpPm1, NodeMechanic::EquivalentMass);
        //Port Pm2
        mpPm2_f=getSafeNodeDataPtr(mpPm2, NodeMechanic::Force);
        mpPm2_x=getSafeNodeDataPtr(mpPm2, NodeMechanic::Position);
        mpPm2_v=getSafeNodeDataPtr(mpPm2, NodeMechanic::Velocity);
        mpPm2_c=getSafeNodeDataPtr(mpPm2, NodeMechanic::WaveVariable);
        mpPm2_Zc=getSafeNodeDataPtr(mpPm2, NodeMechanic::CharImpedance);
        mpPm2_eqMass=getSafeNodeDataPtr(mpPm2, NodeMechanic::EquivalentMass);

        //Read variables from nodes
        //Port Pm1
        fm1 = (*mpPm1_f);
        xm1 = (*mpPm1_x);
        vm1 = (*mpPm1_v);
        cm1 = (*mpPm1_c);
        Zcm1 = (*mpPm1_Zc);
        eqMassm1 = (*mpPm1_eqMass);
        //Port Pm2
        fm2 = (*mpPm2_f);
        xm2 = (*mpPm2_x);
        vm2 = (*mpPm2_v);
        cm2 = (*mpPm2_c);
        Zcm2 = (*mpPm2_Zc);
        eqMassm2 = (*mpPm2_eqMass);

        //Read inputVariables from nodes

        //Read inputParameters from nodes
        Ks = (*mpKs);
        x0 = (*mpx0);
        eps = (*mpeps);
        alpha = (*mpalpha);

        //Read outputVariables from nodes

//==This code has been autogenerated using Compgen==
        //InitialExpressions
        fak = 1/(1 - alpha);
        Zexpr = fak*Ks*mTimestep;
        cm1 = Ks*(-x0 + xm1 + xm2) - vm1*Zexpr;
        cm2 = Ks*(-x0 + xm1 + xm2) - vm2*Zexpr;
        cm1f = fm1;
        cm2f = fm2;

        //Initialize delays


        simulateOneTimestep();

     }
    void simulateOneTimestep()
     {
        //LocalExpressions variables
        double cm10;
        double cm20;

        //Read variables from nodes
        //Port Pm1
        fm1 = (*mpPm1_f);
        xm1 = (*mpPm1_x);
        vm1 = (*mpPm1_v);
        eqMassm1 = (*mpPm1_eqMass);
        //Port Pm2
        fm2 = (*mpPm2_f);
        xm2 = (*mpPm2_x);
        vm2 = (*mpPm2_v);
        eqMassm2 = (*mpPm2_eqMass);

        //Read inputVariables from nodes

        //Read inputParameters from nodes
        Ks = (*mpKs);
        x0 = (*mpx0);
        eps = (*mpeps);
        alpha = (*mpalpha);

        //LocalExpressions
        fak = 1/(1 - alpha);
        Zexpr = fak*Ks*mTimestep;
        cm10 = cm2 + 2*vm2*Zexpr;
        cm20 = cm1 + 2*vm1*Zexpr;

        //Expressions
        cm1f = (1 - alpha)*cm10 + alpha*cm1f - eps*((fm1 + fm2)/2. - Ks*(-x0 \
+ xm1 + xm2));
        cm2f = (1 - alpha)*cm20 + alpha*cm2f - eps*((fm1 + fm2)/2. - Ks*(-x0 \
+ xm1 + xm2));
        cm1 = cm1f;
        cm2 = cm2f;
        Zcm1 = Zexpr;
        Zcm2 = Zexpr;

        //Calculate the delayed parts


        //Write new values to nodes
        //Port Pm1
        (*mpPm1_c)=cm1;
        (*mpPm1_Zc)=Zcm1;
        //Port Pm2
        (*mpPm2_c)=cm2;
        (*mpPm2_Zc)=Zcm2;
        //outputVariables

        //Update the delayed variabels

     }
    void deconfigure()
    {
        delete mpSolver;
    }
};
#endif // MECHANICSPRING_HPP_INCLUDED
