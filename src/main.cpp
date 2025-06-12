#include "camera.hpp"
#include "integrator.hpp"
#include "renderer.hpp"

int main(int argc, char **argv)
{
	// Dump CLI args
	for (int i = 0; i < argc; i++) {
		printf("arg %d: %s\n", i, argv[i]);
	}

	// Set up default config
	RendererConfig config{};
	config.filename = "render.png";
	config.resolutionX = 1280;
	config.resolutionY = 720;
	config.sampleCount = 64;

	// Set up camera
	Camera camera{};
	camera.FOVy = 60.0F;
	camera.aspectRatio = static_cast<float>(config.resolutionX) / static_cast<float>(config.resolutionY);

	// Set up scene & integrator
	PathTracedIntegrator integrator(10 /* bounce depth */);

	// Render scene
	Renderer().render(config, camera, integrator);
	return 0;
}
