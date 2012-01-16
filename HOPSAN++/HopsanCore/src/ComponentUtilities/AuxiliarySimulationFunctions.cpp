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
//! @file   AuxiliarySimulationFunctions.cc
//! @author Robert Braun <robert.braun@liu.se>
//! @date   2010-06-30
//!
//! @brief Contiains a second order integrator utility with provision for some damping
//!
//$Id$

#include "ComponentUtilities/AuxiliarySimulationFunctions.h"
#include <cmath>

//! @brief Limits a value so it is between min and max
//! @param &value Reference pointer to the value
//! @param min Lower limit of the value
//! @param max Upper limit of the value
void hopsan::limitValue(double &value, double min, double max)
{
    if(min>max)
    {
        double temp;
        temp = max;
        max = min;
        min = temp;
    }
    if(value > max)
    {
        value = max;
    }
    else if(value < min)
    {
        value = min;
    }
}


//! @brief Returns the sign of a double (-1.0 or +1.0)
//! @param x Value to determine sign on
double hopsan::sign(double x)
{
    if (x>=0.0)
    {
        return 1.0;
    }
    else
    {
        return -1.0;
    }
}


//! @brief Returns 1.0 if x is positive, else returns 0.0
//! @param x Value to determine if it is positive
double hopsan::onPositive(double x)
{
    if (x < 0.0) { return 0.0; }
    return 1.0;
}



double hopsan::dxOnPositive(double /*x*/)
{
    return 0.0;
}


//! @brief Returns 1.0 if x is negative, else returns 0.0
//! @param x Value to determine if it is positive
double hopsan::onNegative(double x)
{
    if (x < 0.0) { return 1.0; }
    return 0.0;
}


double hopsan::dxOnNegative(double /*x*/)
{
    return 0.0;
}

double hopsan::dxAbs(double x)
{
    if (x < 0.0) { return -1; }
    return 1;
}

//! @brief Returns y1 or y2 depending on the value of x.
//! @param x input value
//! @param y1 if x is positive
//! @param y2 otherwise
//! @returns Limited derivative of x
double hopsan::ifPositive(double x, double y1, double y2)
{
    if (x >= 0) { return y1; }
    else { return y2; }
}


//! @brief Derivative of IfPositive with respect to y1.
//! @param x input value
//! @param y1 dummy
//! @param y2 dummy
//! @returns Limited derivative of x
double hopsan::dtIfPositive(double x, double /*y1*/, double /*y2*/)
{
    if (x >= 0) { return 1.; }
    else { return 0.; }
}

//! @brief Derivative of IfPositive with respect to y1.
//! @param x input value
//! @param y1 dummy
//! @param y2 dummy
//! @returns Limited derivative of x
double hopsan::dfIfPositive(double x, double /*y1*/, double /*y2*/)
{
    if (x >= 0) { return 1.; }
    else { return 0.; }
}


double hopsan::signedSquareL(double x, double x0)
{
    return (-sqrt(x0) + sqrt(x0 + fabs(x))) * hopsan::sign(x);
}


double hopsan::dxSignedSquareL(double x, double x0)
{
    return (1.0 / (sqrt(x0 + fabs(x)) * 2.0));
}


double hopsan::squareAbsL(double x, double x0)
{
    return (-sqrt(x0) + sqrt(x0 + fabs(x)));
}

double hopsan::dxSquareAbsL(double x, double x0)
{
    return 1.0 / (sqrt(x0 + fabs(x)) * 2.0) * hopsan::sign(x);
}

//! @brief Safe variant of atan2
double hopsan::Atan2L(double y, double x)
{
    if (x >0. || x<0.)
    { return atan2(y,x);}
        else
    {return 0.;}
}

double hopsan::d1Atan2L(double y, double x)
{
    //return x/(0.001 + pow(x,2) + pow(y,2));
    return x/(0.001 + x*x + y*y);
}

//! @brief Derivative of ATAN2L with respect to x
double hopsan::d2Atan2L(double y, double x)
{
    //return -y/(0.001 + pow(x,2) + pow(y,2));
    return -y/(0.001 + x*x + y*y);
}

//! @brief Returns 1.0 if input variables have same sign, else returns 0.0
double hopsan::equalSigns(double x, double y)
{
    if (hopsan::sign(x) != hopsan::sign(y)) {
        return 0.0;
    }
    return 1.0;
}

//! @brief Safe variant of asin
double hopsan::ArcSinL(double x)
{
    return asin(limit(x,-0.999,0.999));
}

//! @brief derivative of AsinL
double hopsan::dxArcSinL(double x)
{
    return 1.0/sqrt(1 - pow(limit(x,-0.999,0.999),2));
}

//! @brief difference between two angles, fi1-fi2
double hopsan::diffAngle(double fi1, double fi2)
{   double output;
    double output0 = fi1-fi2;
    double output1 = fi1-fi2 + 2*3.14159;
    double output2 = fi1-fi2 - 2*3.14159;
    if (fabs(output0)> fabs(output1))
      {output = output1;}
    else if (fabs(output0)< fabs(output2))
      {output = output2;}
    else
      {output = output0;}
    //this is a fix to make it work for small angle differences. Something is wrong with the conditions
    output=fi1-fi2;
    return output;
}

//! @brief Induced drag coefficient for aircraft model
double hopsan::CLift( double alpha,double CLalpha,double ap,double an,double expclp,double expcln)
{
    return sin(2.0*alpha)/sqrt(2.0) + ((-(1.0/sqrt(2.0)) + CLalpha/2.0)*sin(2.0*alpha))/
            (1 + fabs(onNegative(sin(alpha))*pow(sin(alpha)/an,expcln) +
                      onPositive(sin(alpha))*pow(sin(alpha)/ap,expclp)));
}

//! @brief Induced drag coefficient for aircraft model
double hopsan::CDragInd(double alpha,double AR,double e,double CLalpha,double ap,double an,double expclp,double expcln)
{
    return 0.35355*(1.0 - 1.0/
                    (1.0 + fabs(onNegative(sin(alpha))*pow(sin(alpha)/an,expcln) +
                                onPositive(sin(alpha))*pow(sin(alpha)/ap,expclp))))*(1 - cos(2.0*alpha)) +
            ((1 - cos(2.0*alpha))*(sin(2.0*alpha)/sqrt(2.0) +
                                   ((-(1.0/sqrt(2.0)) + CLalpha/2.0)*sin(2.0*alpha))/
                                   (1 + fabs(onNegative(sin(alpha))*pow(sin(alpha)/an,expcln) +
                                             onPositive(sin(alpha))*pow(sin(alpha)/ap,expclp)))))/(2.0*AR*e);
}

//! @brief Overloads void hopsan::limitValue() with a return value.
//! @see void hopsan::limitValue(&value, min, max)
//! @param x Value to be limited
//! @param xmin Minimum value of x
//! @param xmax Maximum value of x
double hopsan::limit(double x, double xmin, double xmax)
{
    double output = x;
    hopsan::limitValue(output, xmin, xmax);
    return output;
}


//! @brief Sets the derivative of x to zero if x is outside of limits.
//! Returns 1.0 if x is within limits, else 0.0. Used to make the derivative of x zero if limit is reached.
//! @param x Value whos derivative is to be limited
//! @param xmin Minimum value of x
//! @param xmax Maximum value of x
//! @returns Limited derivative of x
double hopsan::dxLimit(double x, double xmin, double xmax)
{
    if (x >= xmax) { return 0.0; }
    if (x <= xmin) { return 0.0; }
    return 1.0;
}


//! @brief Overloads double hopsan::limit() to also include sx (derivative of x) as input
//! @see void hopsan::limit(&x, min, max)
//! @param x Value to be limited
//! @param sx Derivative of x
//! @param xmin Minimum value of x
//! @param xmax Maximum value of x
//! @returns Limited x value
double hopsan::limit2(double x, double /*sx*/, double xmin, double xmax)
{
    return hopsan::limit(x, xmin, xmax);
}


//! @brief Limits the derivative of x when x is outside of its limits.
//! Returns 1.0 if x is within borders, or if x is outside borders but derivative has opposite sign (so that x can only move back to the limited range).
//! @param x Value whos derivative is to be limited
//! @param xmin Minimum value of x
//! @param xmax Maximum value of x
//! @returns Limited derivative of x
double hopsan::dxLimit2(double x, double sx, double xmin, double xmax)
{
    if (x >= xmax && sx >= 0.0) { return 0.0; }
    if (x <= xmin && sx <= 0.0) { return 0.0; }
    return 1.0;
}
