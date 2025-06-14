#include <cstdio>

#include "camera.hpp"
#include "integrator.hpp"
#include "renderer.hpp"
#include "scene.hpp"

int main(int argc, char **argv)
{
	// Dump CLI args
	for (int i = 0; i < argc; i++) {
		printf("arg %d: %s\n", i, argv[i]);
	}

	// Set up default config
	// FIXME(nemjit001): load this from either CLI args or scene format
	RendererConfig config{};
	config.filename = "render.png";
	config.resolutionX = 1024;
	config.resolutionY = 1024;
	config.sampleCount = 128;

	printf("Render config\n");
	printf("  Resolution X: %u\n", config.resolutionX);
	printf("  Resolution Y: %u\n", config.resolutionY);
	printf("  Sample count: %u\n", config.sampleCount);
	printf("  Output file:  %s\n", config.filename.c_str());

	// Set up camera
	// FIXME(nemjit001): load this from either CLI args or scene format
	Camera camera{};
	camera.FOVy = 60.0F;
	camera.aspectRatio = static_cast<float>(config.resolutionX) / static_cast<float>(config.resolutionY);
	camera.position = { 0.0F, 1.0F, 3.0F };
	camera.forward = { 0.0F, 0.0F, -1.0F };

	// Set up scene
	Scene scene = Scene::fromFile("./assets/CornellBox.obj");

	// Set up integrator
	PathTracedIntegrator integrator(10 /* max bounce depth */);
	integrator.setSceneData(scene);

	// Render scene
	Renderer().render(config, camera, integrator);
	return 0;
}
