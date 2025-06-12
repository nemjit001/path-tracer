#include "integrator.hpp"

#include <cassert>

PathTracedIntegrator::PathTracedIntegrator(uint32_t maxBounceDepth)
	:
	m_maxBounceDepth(maxBounceDepth)
{
	//
}

void PathTracedIntegrator::setSceneData(Scene const& scene)
{
	m_accelerationStructure = std::make_shared<BVH>();

	// FIXME(nemjit001): Get scene geometry from actual scene
	Geometry sceneGeometry{};
	sceneGeometry.type = GeometryType::Triangles;
	sceneGeometry.data.vertices.positions = nullptr;
	sceneGeometry.data.vertices.stride = 0;
	sceneGeometry.data.vertices.count = 0;

	m_accelerationStructure->setGeometry(sceneGeometry);
	m_accelerationStructure->build();
}

glm::vec3 PathTracedIntegrator::trace(Ray const& ray, Sampler& sampler) const
{
	assert(m_accelerationStructure != nullptr && "Integrator needs scene TLAS to be set");

	Ray current = ray;
	glm::vec3 throughput{ 1.0F, 1.0F, 1.0F };
	glm::vec3 energy{};

	for (uint32_t i = 0; i < m_maxBounceDepth; i++) // max bounce depth is 7
	{
		float const tMin = 1e-3F;
		float const tMax = 1e30F;
		Intersection const intersection = m_accelerationStructure->intersect(current, tMin, tMax);
		if (intersection.primitive == INVALID_PRIMITIVE) {
			// Miss, evaluate environment
			// TODO(nemjit001): Evaluate environment color (HDRI, color, whatever)
			energy += throughput * glm::vec3(0.3F, 0.6F, 0.9F); //< just some blue color
			break;
		}
		else {
			// Hit, evaluate geometry & material
			// TODO(nemjit001): evaluate material etc. & sample new ray direction using material BRDF.
			break;
		}
	}

	return energy;
}
