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
	RendererConfig config{};
	config.filename = "render.png";
	config.resolutionX = 1280;
	config.resolutionY = 720;
	config.sampleCount = 64;

	// Set up camera
	Camera camera{};
	camera.FOVy = 60.0F;
	camera.aspectRatio = static_cast<float>(config.resolutionX) / static_cast<float>(config.resolutionY);
	camera.position = { 0.0F, 1.0F, 3.0F };
	camera.forward = { 0.0F, 0.0F, -1.0F };

	// Set up scene & integrator
	Scene scene = Scene::fromFile("./assets/CornellBox.obj");
	PathTracedIntegrator integrator(10 /* max bounce depth */);
	integrator.setSceneData(scene);

	// Render scene
	Renderer().render(config, camera, integrator);
	return 0;
}
