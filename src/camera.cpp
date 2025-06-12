#include "camera.hpp"

ViewPyramid Camera::generateViewPyramid() const
{
	// Set up viewport size
	float const viewportHeight = 2.0F * glm::tan(glm::radians(FOVy / 2.0F));
	float const viewportWidth = viewportHeight * aspectRatio;

	// Set up viewport vectors
	glm::vec3 const viewU = right * viewportWidth;
	glm::vec3 const viewV = -up * viewportHeight;

	// Set up viewport positions
	glm::vec3 const px00 = position + forward - 0.5F * (viewU + viewV);
	glm::vec3 const px10 = px00 + viewU;
	glm::vec3 const px01 = px00 + viewV;

	return ViewPyramid{
		position,
		px00,
		px10,
		px01,
	};
}
