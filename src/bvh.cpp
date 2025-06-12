#include "bvh.hpp"

void BVH::setGeometry(Geometry const& geometry)
{
	m_geometry = geometry;
}

void BVH::build()
{
	assert(m_geometry.type != GeometryType::Undefined && "Geometry must be defined for BVH build");

	// Clear old state
	m_nodes.clear();
	m_nodeCount = 0;
	m_usedNodes = 0;

	// TODO(nemjit001): Build according to geometry type
}

void BVH::update()
{
	// Readjust node bounds according to stored geometry
	// TODO(nemjit001): actually implement this
}

Intersection BVH::intersect(Ray const& ray, float tMin, float tMax) const
{
	return Intersection{
		tMax, //< miss :)
		0.0F, 0.0F,
		INVALID_PRIMITIVE,
		INVALID_INSTANCE
	};
}
