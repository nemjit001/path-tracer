#pragma once

#include <string>
#include <glm/glm.hpp>

class Material
{
public:
	std::string name		= "Material";
	glm::vec3	baseColor	= { 0.5F, 0.5F, 0.5F };
	glm::vec3	emission	= { 0.0F, 0.0F, 0.0F };
	float		metallic	= 0.0F;
	float		roughness	= 0.5F;
};
