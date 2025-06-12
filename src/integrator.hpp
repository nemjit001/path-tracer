#pragma once

#include <memory>
#include <tiny_bvh.h>

#include "ray.hpp"
#include "sampler.hpp"
#include "scene.hpp"

/// @brief The Integrator class can be used to sample scenes using different rendering equation integration algorithms.
class Integrator
{
public:
	Integrator() = default;
	virtual ~Integrator() = default;

	Integrator(Integrator const&) = default;
	Integrator& operator=(Integrator const&) = default;

	/// @brief Set the integrator scene data.
	/// @param scene 
	virtual void setSceneData(Scene const& scene) = 0;

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

	void setSceneData(Scene const& scene) override;

	glm::vec3 trace(Ray const& ray, Sampler& sampler) const override;

private:
	struct RenderInstance
	{
		uint32_t object;
		uint32_t material;
	};

private:
	uint32_t									m_maxBounceDepth	= 5;
	Scene const*								m_pScene			= nullptr;

	// -- Scene Data --
	std::vector<RenderInstance>					m_instances			= {};

	// -- Acceleration Structures --
	std::vector<std::shared_ptr<tinybvh::BVH>>	m_blasses			= {};
	std::vector<tinybvh::BVHBase*>				m_blasPointers		= {}; //< required for tinybvh blas instancing :/
	std::vector<tinybvh::BLASInstance>			m_blasInstances		= {};
	std::shared_ptr<tinybvh::BVH>				m_tlas				= {};
};
