#pragma once

#include <string>
#include <vector>

#include "material.hpp"
#include "mesh.hpp"

struct SceneObject
{
	uint32_t mesh;
	uint32_t material;
};

/// @brief The Scene stores host-side rendering data.
class Scene
{
public:
	/// @brief Load a scene from a filepath.
	/// @param path 
	/// @return 
	static Scene fromFile(std::string const& path);

public:
	std::vector<Mesh>			meshes		= {};
	std::vector<Material>		materials	= {};
	std::vector<SceneObject>	objects		= {};
};
