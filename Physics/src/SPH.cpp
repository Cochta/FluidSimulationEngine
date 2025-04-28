#include "SPH.h"

float SPH::SmoothingKernelPoly6(float radius, float dst) {
    if (dst < radius) {
        float v = radius * radius - dst * dst;
        return v * v * v * Poly6ScalingFactor;
    }
    return 0.0f;
}

float SPH::SpikyKernelPow3(float radius, float dst) {
    if (dst < radius) {
        float v = radius - dst;
        return v * v * v * SpikyPow3ScalingFactor;
    }
    return 0.0f;
}

float SPH::SpikyKernelPow2(float radius, float dst) {
    if (dst < radius) {
        float v = radius - dst;
        return v * v * SpikyPow2ScalingFactor;
    }
    return 0.0f;
}

float SPH::DerivativeSpikyPow3(float radius, float dst) {
    if (dst < radius && dst > 0.0001f) {  // Prevents division by zero
        float v = radius - dst;
        return (v * v) * SpikyPow3DerivativeScalingFactor / dst;
    }
    return 0.0f;
}

float SPH::DerivativeSpikyPow2(float radius, float dst) {
    if (dst < radius && dst > 0.0001f) {
        float v = radius - dst;
        return v * SpikyPow2DerivativeScalingFactor / dst;
    }
    return 0.0f;
}

float SPH::DensityKernel(float radius, float dst) {
    return SpikyKernelPow2(radius, dst);
}

float SPH::NearDensityKernel(float radius, float dst) {
    return SpikyKernelPow3(radius, dst);
}

float SPH::DensityDerivative(float radius, float dst) {
    return DerivativeSpikyPow2(radius, dst);
}

float SPH::NearDensityDerivative(float radius, float dst) {
    return DerivativeSpikyPow3(radius, dst);
}

float SPH::ViscosityKernel(float radius, float dst)
{
	return SmoothingKernelPoly6(radius, dst);
}

