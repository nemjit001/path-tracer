#pragma once

#include <cstdint>
#include <string>

#include "camera.hpp"
#include "integrator.hpp"

/// @brief Renderer configuration data.
struct RendererConfig
{
	std::string filename;
	uint32_t resolutionX;
	uint32_t resolutionY;
	uint32_t sampleCount;
};

/// @brief The Renderer class allows the rendering of scenes using different integration strategies.
class Renderer
{
public:
	/// @brief Render an image using the given parameters.
	/// @param config Render configuration.
	/// @param camera Camera to use for rendering.
	/// @param integrator Integrator with associated scene to use for rendering.
	void render(RendererConfig const& config, Camera const& camera, Integrator const& integrator);
};
