#pragma once

#include <vector>
#include <glm/glm.hpp>

struct Vertex
{
	glm::vec3 position;
	glm::vec3 normal;
	glm::vec3 tangent;
	glm::vec2 texcoord;
};

class Mesh
{
public:
	std::string				name		= "Mesh";
	std::vector<Vertex>		vertices	= {};
	std::vector<uint32_t>	indices		= {};
};
