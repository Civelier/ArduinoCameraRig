#ifndef _Curves_h
#define _Curves_h

#if defined(ARDUINO) && ARDUINO >= 100
#include "arduino.h"
#else
#include "WProgram.h"
#endif


typedef float(*curve_t)(float);

float Clamp(float v);

float LinearCurve(float v);

float QuadraticInCurve(float v);

float QuadraticOutCurve(float v);

float QuadraticInOutCurve(float v);

float SinInCurve(float v);

float SinOutCurve(float v);

float SinInOutCurve(float v);

float ExponentialInCurve(float v);

float ExponentialOutCurve(float v);

float ExponentialInOutCurve(float v);

#endif