#pragma once

#include "Body.h"

struct ParticleData
{
	float Density = 1.0f;
	float NearDensity = 1.0f;
	float Pressure = 1.0f;
	float SmoothingLength = 1.0f;
	float Viscosity = 0.1f;
	XMVECTOR Position = XMVectorZero();
	//XMVECTOR Acceleration = XMVectorZero();
};