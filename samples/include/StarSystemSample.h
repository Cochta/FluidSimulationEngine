#pragma once

#include "PhysicsSample.h"
#include "Random.h"

class StarSystemSample : public PhysicsSample
{
private:
    std::vector<SphereF> _spheres;

    BodyRef _sunRef;

    static constexpr float G = 6.67f;
    static constexpr std::size_t PLANET_NBR = 1000;
public:
    std::string GetName() noexcept override;
    std::string GetDescription() noexcept override;
protected:
    void SampleSetUp() noexcept override;

    void SampleUpdate() noexcept override;

    void SampleTearDown() noexcept override;

private:
    static void CalculateGravitationalForce(const Body &sun, Body &body) noexcept;

};
