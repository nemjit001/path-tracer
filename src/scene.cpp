#include "scene.hpp"

#define TINYOBJLOADER_IMPLEMENTATION

#include <cstdio>
#include <unordered_map>
#include <tiny_obj_loader.h>

Scene Scene::fromFile(std::string const& path)
{
	// Read file from disk
	tinyobj::ObjReaderConfig config{};
	config.triangulate = true;
	config.triangulation_method = "earcut";

	tinyobj::ObjReader reader{};
	if (!reader.ParseFromFile(path, config)) {
		printf("%s\n", reader.Error().c_str());
		return {};
	}

	if (!reader.Warning().empty()) {
		printf("%s\n", reader.Warning().c_str());
	}

	// Get OBJ data
	auto const& materials = reader.GetMaterials();
	auto const& shapes = reader.GetShapes();
	auto const& attrib = reader.GetAttrib();

	// Set up scene
	Scene scene{};

	// Load material data into scene
	for (auto const& objmat : materials)
	{
		printf("Parsing material %s\n", objmat.name.c_str());

		Material material{};
		material.name		= objmat.name;
		material.baseColor	= { objmat.diffuse[0], objmat.diffuse[1], objmat.diffuse[2] };
		material.emission	= { objmat.emission[0], objmat.emission[1], objmat.emission[2] };
		material.metallic	= objmat.metallic;
		material.roughness	= objmat.roughness;
		material.IOR		= objmat.ior;

		scene.materials.push_back(material);
	}

	// Load shape data into scene meshes
	for (auto const& shape : shapes)
	{
		printf("Parsing shape %s\n", shape.name.c_str());

		// Gather mesh by material ID
		std::unordered_map<int, Mesh> submeshes{};
		size_t idxOffset = 0;
		for (size_t face = 0; face < shape.mesh.num_face_vertices.size(); face++)
		{
			// Get mesh for material
			int const material = shape.mesh.material_ids[face];
			Mesh& mesh = submeshes[material];
			mesh.name = shape.name;

			// Append face data
			size_t const vertCount = shape.mesh.num_face_vertices[face];
			for (size_t v = 0; v < vertCount; v++)
			{
				auto const& index = shape.mesh.indices[idxOffset + v];
				glm::vec3 const position = { attrib.vertices[index.vertex_index * 3 + 0], attrib.vertices[index.vertex_index * 3 + 1], attrib.vertices[index.vertex_index * 3 + 2], };

				glm::vec3 normal{};
				if (index.normal_index >= 0) {
					normal = { attrib.normals[index.normal_index * 3 + 0], attrib.normals[index.normal_index * 3 + 1], attrib.normals[index.normal_index * 3 + 2], };
				}

				glm::vec2 texcoord{};
				if (index.texcoord_index >= 0) {
					texcoord = { attrib.texcoords[index.texcoord_index * 2 + 0], attrib.texcoords[index.texcoord_index * 2 + 1], };
				}

				Vertex const vertex{
					position,
					normal,
					{}, /* tangent */
					texcoord,
				};

				mesh.vertices.push_back(vertex);
				mesh.indices.push_back(static_cast<uint32_t>(mesh.indices.size()));
			}

			idxOffset += vertCount;
		}

		// Store gathered meshes in scene
		for (auto& [materialIdx, mesh] : submeshes)
		{
			if (mesh.vertices.empty() || mesh.indices.empty()) {
				continue; // Skip empty meshes, just wastes space
			}

			// Set mesh name
			mesh.name += std::to_string(materialIdx);

			// Store in scene
			size_t const meshIdx = scene.meshes.size();
			scene.meshes.push_back(mesh);
			scene.objects.push_back(SceneObject{ static_cast<uint32_t>(meshIdx), static_cast<uint32_t>(materialIdx) });
		}
	}

	// Calculate normals & tangents for meshes
	for (auto& mesh : scene.meshes)
	{
		for (size_t i = 0; i < mesh.indices.size(); i += 3)
		{
			Vertex& v0 = mesh.vertices[mesh.indices[i + 0]];
			Vertex& v1 = mesh.vertices[mesh.indices[i + 1]];
			Vertex& v2 = mesh.vertices[mesh.indices[i + 2]];

			glm::vec3 const e1 = v1.position - v0.position;
			glm::vec3 const e2 = v2.position - v0.position;
			glm::vec2 const dUV1 = v1.texcoord - v0.texcoord;
			glm::vec2 const dUV2 = v2.texcoord - v0.texcoord;
			float const f = 1.0F / (dUV1.x * dUV2.y - dUV1.y * dUV2.x);

			// Calculate tangent
			glm::vec3 const normal = glm::normalize(glm::cross(e1, e2));
			glm::vec3 const tangent = f * (dUV2.y * e1 - dUV1.y * e2);

			// Use triangle normal if normal was not loaded from OBJ file
			float const minLength = 1e-6F; //< length at which to assume normal wasn't set
			if (v0.normal.length() < minLength || v1.normal.length() < minLength || v2.normal.length() < minLength) {
				v0.normal = normal;
				v1.normal = normal;
				v2.normal = normal;
			}

			v0.tangent = tangent;
			v1.tangent = tangent;
			v2.tangent = tangent;
		}
	}

	printf("Parsed scene:\n");
	printf("  Mesh count:     %zu\n", scene.meshes.size());
	printf("  Material count: %zu\n", scene.materials.size());
	printf("  Object count:   %zu\n", scene.objects.size());
	return scene;
}
