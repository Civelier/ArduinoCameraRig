#include "Curves.h"


float Clamp(float v)
{
	if (v < 0.001f) return 0;
	if (v > 0.999f) return 1;
	return v;
}

float LinearCurve(float v)
{
	return Clamp(max(0, min(1, v)));
}

float QuadraticInCurve(float v)
{
	return Clamp(pow(LinearCurve(v), 2.0f));
}

float QuadraticOutCurve(float v)
{
	v = LinearCurve(v);
	return Clamp(-v * (v - 2));
}

float QuadraticInOutCurve(float v)
{
	v = max(0, min(1, v));
	if (v < 0.5f)
		return Clamp(2.0f * (v * v));
	return Clamp(-2.0f * powf(v - 1.0f, 2.0f) + 1.0f);
}

float SinInCurve(float v)
{
	v = LinearCurve(v);
	return Clamp((1.0f - sinf(0.5f * (v * PI + PI))));
}

float SinOutCurve(float v)
{
	v = LinearCurve(v);
	return Clamp(sinf((v * PI) / 2));
}

float SinInOutCurve(float v)
{
	v = LinearCurve(v);
	float result = 0.5f * (1.0f + sinf(PI * (v - 0.5f)));
	return Clamp(result);
}

float ExponentialInCurve(float v)
{
	v = LinearCurve(v);
	return Clamp(powf(2.0f, 10.0f * (v - 1)) - 0.001f);
}

float ExponentialOutCurve(float v)
{
	return Clamp(1.0f - (powf(2.0f, 10.0f * -v)) - 0.001f);
}

float ExponentialInOutCurve(float v)
{
	v = LinearCurve(v);
	if (v < 0.5f) return Clamp(0.5f * (powf(2.0f, 20.0f * (v - 0.5f)) - 0.001));
	return Clamp(0.5f * (2.0f - powf(2.0f, -20.0f * (v - 0.5f)) - 0.001f));
}
