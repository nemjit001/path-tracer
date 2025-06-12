#pragma once

#include <cstdint>
#include <vector>
#include <glm/glm.hpp>

#include "ray.hpp"

constexpr uint32_t INVALID_PRIMITIVE	= ~0U;
constexpr uint32_t INVALID_INSTANCE		= ~0U;

/// @brief Supported geometry types for a BVH.
enum class GeometryType : uint8_t
{
	Undefined = 0x00,
	Triangles = 0x01,
};

/// @brief Strided vertex position pointer, allows packed vertex data to be accessed as float3 positions.
struct StridedVertexPositions
{
	void* positions;
	size_t stride;
	size_t count;
};

/// @brief Geometry data for a BVH, allows setting the geometry type and geometry pointers.
struct Geometry
{
	GeometryType type;
	union GeometryData {
		StridedVertexPositions vertices;
	} data;
};

/// @brief Intersection result for a BVH trace operation.
struct Intersection
{
	float t;			//< hit depth.
	float u, v;			//< hit UV coords for primitive.
	uint32_t primitive;	//< hit primitive, will be INVALID_PRIMTIVE on miss.
	uint32_t instance;	//< hit instance, may be INVALID_INSTANCE.
};

/// @brief Wald 32-byte BVH optimized for CPU ray tracing, supports 3float geometry with arbitrary strides.
class BVH
{
public:
	/// @brief Set the BVH geometry to use for building.
	/// Pointers MUST remain valid for the entire lifetime of the BVH, will NOT take ownership of geometry data.
	/// @param geometry 
	void setGeometry(Geometry const& geometry);

	/// @brief Build the BVH using the stored geometry data.
	void build();

	/// @brief Update the BVH node bounds using the stored geometry.
	void update();

	/// @brief Intersect a ray with the BVH.
	/// @param ray Ray to intersect.
	/// @param tMin Min distance to use for intersection.
	/// @param tMax Max distance to use for intersection.
	/// @return 
	Intersection intersect(Ray const& ray, float tMin, float tMax) const;

private:
	void buildTriangles(StridedVertexPositions positions);

private:
	/// @brief 32 byte BVH node layout for tightly packed of BVH node data.
	/// Aligned to 32 bytes for good cache utilization.
	struct alignas(32) Node
	{
		glm::vec3 aabbMin;
		glm::vec3 aabbMax;
		uint32_t left_or_first; //< left + 1 is right
		uint32_t count;			//< if count == 0 then this is an internal node
	};

private:
	Geometry			m_geometry	= {};
	uint32_t			m_usedNodes	= 0;
	uint32_t			m_nodeCount	= 0;
	std::vector<Node>	m_nodes		= {};
};
