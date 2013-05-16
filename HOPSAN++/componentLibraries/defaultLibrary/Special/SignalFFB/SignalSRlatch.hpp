#ifndef SIGNALSRLATCH_HPP_INCLUDED
#define SIGNALSRLATCH_HPP_INCLUDED

#include <iostream>
#include "ComponentEssentials.h"
#include "ComponentUtilities.h"
#include "math.h"

//!
//! @file SignalSRlatch.hpp
//! @author Petter Krus <petter.krus@liu.se>
//! @date Mon 13 May 2013 18:19:16
//! @brief S-R latch
//! @ingroup SignalComponents
//!
//==This code has been autogenerated using Compgen==
//from 
/*{, C:, HopsanTrunk, HOPSAN++, CompgenModels}/SignalFFBDcomponents.nb*/

using namespace hopsan;

class SignalSRlatch : public ComponentSignal
{
private:
     int mNstep;
//==This code has been autogenerated using Compgen==
     //inputVariables
     double setCond;
     double resetCond;
     //outputVariables
     double Qstate;
     double notQstate;
     //InitialExpressions variables
     double oldQstate;
     //Expressions variables
     //Delay declarations
//==This code has been autogenerated using Compgen==
     //inputVariables pointers
     double *mpsetCond;
     double *mpresetCond;
     //outputVariables pointers
     double *mpQstate;
     double *mpnotQstate;
     EquationSystemSolver *mpSolver;

public:
     static Component *Creator()
     {
        return new SignalSRlatch();
     }

     void configure()
     {
//==This code has been autogenerated using Compgen==

        mNstep=9;

        //Add ports to the component
        //Add inputVariables to the component
            addInputVariable("setCond","On condition","",0.,&mpsetCond);
            addInputVariable("resetCond","off condition","",0.,&mpresetCond);

        //Add outputVariables to the component
            addOutputVariable("Qstate","Logical state","",0.,&mpQstate);
            addOutputVariable("notQstate","Logical inverse of \
state","",0.,&mpnotQstate);

//==This code has been autogenerated using Compgen==
        //Add constants/parameters
     }

    void initialize()
     {
        //Read port variable pointers from nodes

        //Read variables from nodes

        //Read inputVariables from nodes
        setCond = (*mpsetCond);
        resetCond = (*mpresetCond);

        //Read outputVariables from nodes
        Qstate = (*mpQstate);
        notQstate = (*mpnotQstate);

//==This code has been autogenerated using Compgen==
        //InitialExpressions
        oldQstate = Qstate;


        //Initialize delays

     }
    void simulateOneTimestep()
     {
        //Read variables from nodes

        //Read inputVariables from nodes
        setCond = (*mpsetCond);
        resetCond = (*mpresetCond);

        //LocalExpressions

          //Expressions
          Qstate = -0.5 + onPositive(-0.5 + oldQstate) - onPositive(-0.5 + \
resetCond) + onPositive(-0.5 + setCond);
          oldQstate = Qstate;
          notQstate = 1 - Qstate;

        //Calculate the delayed parts


        //Write new values to nodes
        //outputVariables
        (*mpQstate)=Qstate;
        (*mpnotQstate)=notQstate;

        //Update the delayed variabels

     }
    void deconfigure()
    {
        delete mpSolver;
    }
};
#endif // SIGNALSRLATCH_HPP_INCLUDED