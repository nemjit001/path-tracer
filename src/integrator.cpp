#include "integrator.hpp"

#define TINYBVH_IMPLEMENTATION

#include <cassert>
#include <tiny_bvh.h>

#include "brdf.hpp"

PathTracedIntegrator::PathTracedIntegrator(uint32_t maxBounceDepth)
	:
	m_maxBounceDepth(maxBounceDepth)
{
	//
}

void PathTracedIntegrator::setSceneData(Scene const& scene)
{
	assert(!scene.materials.empty());
	assert(!scene.meshes.empty());

	// Set scene
	m_pScene = &scene;

	// Generate render instances
	m_instances.clear();
	for (auto const& object : scene.objects) {
		m_instances.push_back(RenderInstance{ object.mesh, object.material });
	}

	// Create BLASses for meshes in scene
	m_blasses.clear();
	m_blasses.reserve(scene.meshes.size());
	for (auto const& mesh : scene.meshes)
	{
		tinybvh::bvhvec4slice vertices{};
		vertices.data = reinterpret_cast<int8_t const*>(mesh.vertices.data());
		vertices.stride = sizeof(Vertex);
		vertices.count = static_cast<uint32_t>(mesh.vertices.size());

		std::shared_ptr<tinybvh::BVH> blas = std::make_shared<tinybvh::BVH>();
		blas->PrepareBuild(vertices, mesh.indices.data(), mesh.indices.size() / 3);
		m_blasses.push_back(blas);
	}

	// Build all BLASses & store pointers for TLAS build
	m_blasPointers.clear();
	m_blasPointers.reserve(m_blasses.size());
	for (auto const& blas : m_blasses)
	{
		blas->Build();
		m_blasPointers.push_back(blas.get());
	}

	// Build TLAS using instances (just straight up using meshes at world origin for now)
	m_blasInstances.clear();
	m_blasInstances.reserve(m_instances.size());
	for (auto const& instance : m_instances)
	{
		assert(instance.object < m_blasses.size());
		tinybvh::BLASInstance blas(static_cast<uint32_t>(instance.object));
		blas.aabbMin = m_blasses[instance.object]->aabbMin;
		blas.aabbMax = m_blasses[instance.object]->aabbMax;

		m_blasInstances.push_back(blas);
	}

	m_tlas = std::make_unique<tinybvh::BVH>();
	m_tlas->Build(m_blasInstances.data(), static_cast<uint32_t>(m_blasInstances.size()), m_blasPointers.data(), static_cast<uint32_t>(m_blasPointers.size()));
}

glm::vec3 PathTracedIntegrator::trace(Ray const& ray, Sampler& sampler) const
{
	assert(
		m_pScene != nullptr
		&& !m_blasses.empty()
		&& m_tlas != nullptr
		&& "Integrator needs scene data to be set"
	);

	// Set up ray state
	glm::vec3 throughput{ 1.0F, 1.0F, 1.0F };
	glm::vec3 energy{};

	// Set up tinybvh ray
	tinybvh::Ray current({ ray.O.x, ray.O.y, ray.O.z }, { ray.D.x, ray.D.y, ray.D.z });
	for (uint32_t i = 0; i < m_maxBounceDepth; i++) // max bounce depth is 7
	{
		float const tMin = 1e-3F;
		float const tMax = 1e30F;
		
		m_tlas->Intersect(current);
		if (current.hit.t >= BVH_FAR) {
			// Evaluate environment
			// TODO(nemjit001): Evaluate environment color (HDRI, color, whatever)
			energy += throughput * glm::vec3(0.3F, 0.6F, 0.9F); //< just some blue color
			break;
		}
		else {
			// Get hit geometry & material from scene
			RenderInstance const& instance	= m_instances[current.hit.inst];
			Mesh const& mesh				= m_pScene->meshes[instance.object];
			Material const& material		= m_pScene->materials[instance.material];

			// Get hit triangle from mesh
			uint32_t const& idx = mesh.indices[current.hit.prim * 3];
			Vertex const& v0 = mesh.vertices[idx + 0];
			Vertex const& v1 = mesh.vertices[idx + 1];
			Vertex const& v2 = mesh.vertices[idx + 2];

			// Get ray direction
			tinybvh::bvhvec3 pos = current.O + current.hit.t * current.D;
			glm::vec3 const rayDirection = glm::vec3(current.D.x, current.D.y, current.D.z);

			// Interpolate triangle data according to hit UV
			glm::vec2 const uv			= { current.hit.u, current.hit.v };
			glm::vec3 const position	= { pos.x, pos.y, pos.z };// v0.position + uv.x * (v0.position - v1.position) + uv.y * (v0.position - v1.position);
			glm::vec3 const normal		= glm::normalize(v0.normal + uv.x * v1.normal + uv.y * v2.normal);
			glm::vec3 const tangent		= glm::normalize(v0.tangent + uv.x * v1.tangent + uv.y * v2.tangent);

			// Set up TBN matrix for global/local frame conversion (also adjusts normal and tangent for backface hits)
			bool const isBackfaceHit = glm::dot(rayDirection, normal) > 0.0F;
			glm::vec3 const N = (isBackfaceHit ? -normal : normal);
			glm::vec3 const T = (isBackfaceHit ? -tangent : tangent);
			glm::vec3 const B = glm::normalize(glm::cross(N, T));
			glm::mat3 const TBN(T, B, N);

			// Shade hitpoint
			if (material.emission.x > 0.0F || material.emission.y > 0.0F || material.emission.z > 0.0F) {
				energy += throughput * material.emission;
			}

			LambertianBRDF brdf(material.baseColor);
			glm::vec3 const wi = -rayDirection;
			glm::vec3 const wo = TBN * brdf.sample(sampler);
			throughput *= brdf.evaluate(wi, wo, N) / brdf.pdf(wo, N);

			glm::vec3 const D = wo;
			glm::vec3 const O = glm::vec3(position) + D * tMin; // avoid self intersections by offsetting ray a small amount
			current = tinybvh::Ray({ O.x, O.y, O.z }, { D.x, D.y, D.z });
		}
	}

	return energy;
}
