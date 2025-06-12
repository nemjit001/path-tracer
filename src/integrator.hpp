#pragma once

#include <memory>

#include "bvh.hpp"
#include "ray.hpp"
#include "sampler.hpp"

/// @brief The Integrator class can be used to sample scenes using different rendering equation integration algorithms.
class Integrator
{
public:
	Integrator() = default;
	virtual ~Integrator() = default;

	Integrator(Integrator const&) = default;
	Integrator& operator=(Integrator const&) = default;

	/// @brief Trace a ray through the integrator scene.
	/// @param ray Ray to trace.
	/// @param sampler Sampler to use for random sampling during integration.
	/// @return An RGB color sample for the scene.
	virtual glm::vec3 trace(Ray const& ray, Sampler& sampler) const = 0;
};

/// @brief The PathTracedIntegrator used one-directional path tracing to integrate a scene.
class PathTracedIntegrator : public Integrator
{
public:
	PathTracedIntegrator() = default;
	PathTracedIntegrator(uint32_t maxBounceDepth);
	~PathTracedIntegrator() = default;

	PathTracedIntegrator(PathTracedIntegrator const&) = default;
	PathTracedIntegrator &operator=(PathTracedIntegrator const&) = default;

	glm::vec3 trace(Ray const& ray, Sampler& sampler) const override;

private:
	uint32_t				m_maxBounceDepth		= 5;
	std::shared_ptr<BVH>	m_accelerationStructure = {};
};
