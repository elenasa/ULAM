#ifndef BPSTAT_CC
#define BPSTAT_CC
#include <stdio.h>
#include <math.h>
#include <iostream>

#include "./bpstat.h"

#define DBL_MAX         1.7976931348623158e+308 /* max value */
#define DBL_MIN         -1.7976931348623158e+308 /* min value */

//! Constructor
bpstat::bpstat() : k(0) {}

//! Destructor
bpstat::~bpstat()
{}


void bpstat::clear()
{
  k = 0;
  //#define S32_MAX    ((s32)2147483647)   /**< Maximum value of signed 32 bit */
  //#define S32_MIN    ((s32)2147483648UL) /**< Minimum value of signed 32 bit */

  Min = DBL_MAX;
  Max = DBL_MIN;
}

void bpstat::update(double x)
{
  k++;
  if(k == 1)
    {
      M = x;
      V = 0.0;
    }
  else
    {
      double oldM = M;           //need old value for update
      double oldS = V;            //need old value for update
      M = oldM + (x - oldM)/k;   //set before VAR updated
      V = oldS + (x - oldM) * (x - M);
    }

  if(x > Max)
    Max = x;

  if(x < Min)
    Min = x;

  //cout << "(bpstat) update M=" << M << ", V=" << V << " for k==" << k << endl;
  return;
}

int bpstat::getNumValues()
{
  return k;
}

double bpstat::getMean()
{
  return (k > 0) ? M : 0.;
}

double bpstat::getSD()
{
  return sqrt( calcVar() );
}

double bpstat::calcVar()
{
  return ( (k > 1) ? V/(k-1) : 0.);
}

double bpstat::getMin()
{
  return Min;
}

double bpstat::getMax()
{
  return Max;
}

#endif
