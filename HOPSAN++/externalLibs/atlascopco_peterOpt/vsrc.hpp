/*****************************************************************

Force
Translated from old hopsan stvsxsrc.

Erik Jakobsson
20101005

Schematic image:

  -->

*****************************************************************/
#ifndef VSRC_HPP_INCLUDED
#define VSRC_HPP_INCLUDED

//#include "P:/Hopsan_ng/Hopsan_latest/include/ComponentEssentials.h"
//#include "P:/Hopsan_ng/Hopsan_latest/include/ComponentUtilities.h"
#include "../../HopsanCore/ComponentEssentials.h"
#include "../../HopsanCore/ComponentUtilities.h"

namespace hopsan {

    class vsrc : public ComponentQ
    {

    private:
        double X1S, F1S, V1S;
        double *F1_ptr, *X1_ptr, *V1_ptr, *Cx1_ptr, *Zx1_ptr, *in_ptr;
        Integrator XINT;
        Port *pP1, *pIN;

    public:
        static Component *Creator()
        {
            return new vsrc();
        }

        vsrc() : ComponentQ()
        {
            //Set member attributes
            mTypeName = "vsrc";

            //Startvalues
            X1S = 0;
            V1S = 0;
            F1S = 0;

            //Add ports to the component
            pP1 = addPowerPort("P1", "NodeMechanic");
            pIN = addReadPort("IN", "NodeSignal", Port::NOTREQUIRED);

            //Register parameters to be seen in simulation environment.
            registerParameter("Position", "startvalue", "[m]",   X1S);
            registerParameter("Velocity", "startvalue", "[m/s]",   V1S);
            registerParameter("Force", "startvalue", "[N]",   F1S);
        }


        void initialize()
        {
            //Assign node data pointeres
            F1_ptr = pP1->getNodeDataPtr(NodeMechanic::FORCE);
            X1_ptr = pP1->getNodeDataPtr(NodeMechanic::POSITION);
            V1_ptr = pP1->getNodeDataPtr(NodeMechanic::VELOCITY);
            Cx1_ptr = pP1->getNodeDataPtr(NodeMechanic::WAVEVARIABLE);
            Zx1_ptr = pP1->getNodeDataPtr(NodeMechanic::CHARIMP);

//            double V1;
//            if(pIN->isConnected())
//                V1  = pIN->readNode(NodeSignal::VALUE);
//            else
//                V1=V1S;
            if(pIN->isConnected())
            {
                in_ptr = pIN->getNodeDataPtr(NodeSignal::VALUE);
            }
            else
            {
                in_ptr = new double(V1S);
            }

            double V1 = *in_ptr;

            //Initiate the integrator
            XINT.initialize(mTimestep, V1, X1S);

            //STARTVALUEHANDLING NOT COMPLETE, SINCE WE'RE WAITING FOR LiTH!
//            pP1->writeNode(NodeMechanic::POSITION, X1S);
//            pP1->writeNode(NodeMechanic::VELOCITY, V1);
//            pP1->writeNode(NodeMechanic::FORCE, F1S);
            *X1_ptr = X1S;
            *V1_ptr = V1;
            *F1_ptr = F1S;
        }

        void simulateOneTimestep()
        {
//            double V1;

            //Get variable values from nodes
//            double Zx1  = pP1->readNode(NodeMechanic::CHARIMP);
//            double Cx1  = pP1->readNode(NodeMechanic::WAVEVARIABLE);
            double Zx1 = *Zx1_ptr;
            double Cx1 = *Cx1_ptr;

//            //If signal port is connected, read the value from the port.
//            //else use the start value (V1S never changed).
//            if(pIN->isConnected())
////                V1  = pIN->readNode(NodeSignal::VALUE);
//                V1 = *in_ptr;
//            else
//                V1=V1S;
            double V1 = *in_ptr;

            //Calculate position by integrating velocity.
            double X1 = XINT.update(V1);

            //Calculate force of source.
            double F1 = Cx1 + Zx1 * V1;

            //Write new values to nodes
//            pP1->writeNode(NodeMechanic::FORCE, F1);
//            pP1->writeNode(NodeMechanic::VELOCITY, V1);
//            pP1->writeNode(NodeMechanic::POSITION, X1);
            *F1_ptr = F1;
            *V1_ptr = V1;
            *X1_ptr = X1;
        }
    };
}

#endif // VSRC_HPP_INCLUDED

