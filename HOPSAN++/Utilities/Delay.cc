/*
 *  Delay.cc
 *  HOPSAN++
 *
 *  Created by Björn Eriksson on 2009-12-19.
 *  Copyright 2009 LiU. All rights reserved.
 *
 */

#include <math.h>
#include "Delay.h"



Delay::Delay()
{
}


Delay::Delay(const std::size_t stepDelay, const double initValue)
{
    mStepDelay = stepDelay;
    mFracStep = mStepDelay;
    mInitialValue = initValue;
    mValues.resize(mStepDelay, mInitialValue);
}


Delay::Delay(const double timeDelay, const double Ts, const double initValue)
{
    mFracStep = timeDelay/Ts;
    //avrundar uppat
    mStepDelay = (std::size_t) ceil(((double) timeDelay)/Ts); ///TODO: kolla att det verkligen ar ratt
    mInitialValue = initValue;
    mValues.resize(mStepDelay, mInitialValue);
}


void Delay::initilizeValues(const double initValue)
{
    mInitialValue = initValue;
    mValues.assign(mValues.size(), mInitialValue);
}


void Delay::update(const double value)
{
    mValues.push_front(value);
    mValues.pop_back();
}


void Delay::setStepDelay(const std::size_t stepDelay, const double initValue)
{
    mStepDelay = stepDelay;
    mFracStep = mStepDelay;
    if (initValue != 0)
    {
        mInitialValue = initValue;
    }
    mValues.resize(mStepDelay, mInitialValue);
}


void Delay::setTimeDelay(const double timeDelay, const double Ts, const double initValue)
{
    mFracStep = timeDelay/Ts;
    //avrundar uppat
    mStepDelay = (std::size_t) ceil(((double) timeDelay)/Ts); ///TODO: kolla att det verkligen ar ratt
    if (initValue != 0)
    {
        mInitialValue = initValue;
    }
    mValues.resize(mStepDelay, mInitialValue);
}


double Delay::value()
{
    if (mValues.empty())
    {
        return 0;
    }
    else if ((mFracStep < mStepDelay) && (mValues.size() >= 2))
    {
        return ((1 - (mStepDelay - mFracStep)) * mValues[mValues.size()-2] + (mStepDelay - mFracStep) * mValues.back()); //interpolerar
    }
    else
    {
        return mValues.back();
    }

}


double Delay::value(const std::size_t idx) ///TODO: interpolera värden
{
    return mValues[idx];
}
