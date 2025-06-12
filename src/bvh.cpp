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

	buildTriangles(m_geometry.data.vertices);
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

void BVH::buildTriangles(StridedVertexPositions positions)
{
	// Check if positions were passed
	assert(positions.count > 0 && positions.positions != nullptr);
	if (positions.count == 0 || positions.positions == nullptr) {
		return;
	}

	// Set stride if defaulted
	if (positions.stride == 0) {
		positions.stride = sizeof(glm::vec3);
	}

	// Set up initial geometry state
	assert(positions.stride >= sizeof(glm::vec3));
	m_nodeCount = 2 * positions.count;
	m_usedNodes = 2;
	m_nodes.resize(m_nodeCount);

	// Set up root node
	m_nodes[0].left_or_first = 0;
	m_nodes[0].count = positions.count;

	// Recursively subdivide nodes
}
