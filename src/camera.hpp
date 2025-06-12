#pragma once

#include <glm/glm.hpp>

/// @brief Simple view pyaramid built from 4 positions.
struct ViewPyramid
{
	glm::vec3 origin;
	glm::vec3 pxTopLeft;
	glm::vec3 pxTopRight;
	glm::vec3 pxBottomLeft;
};

/// @brief Basic perspective camera.
class Camera
{
public:
	/// @brief Generate a view pyramid for the camera.
	/// @return 
	ViewPyramid generateViewPyramid() const;

public:
	// -- Settings --//
	float FOVy			= 60.0F;
	float aspectRatio	= 1.0F;

	// -- Transform --//
	glm::vec3 position	= { 0.0F, 0.0F, 0.0F };
	glm::vec3 forward	= { 0.0F, 0.0F, 1.0F };
	glm::vec3 up		= { 0.0F, 1.0F, 0.0F };
	glm::vec3 right		= { 1.0F, 0.0F, 0.0F };
};
