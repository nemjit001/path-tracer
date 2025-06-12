#pragma once

#include <glm/glm.hpp>

/// @brief Single precision ray representation.
class Ray
{
public:
	constexpr Ray(glm::vec3 const& origin, glm::vec3 const& direction)
		: O(origin), D(direction) {}

public:
	glm::vec3 O;
	glm::vec3 D;
};
