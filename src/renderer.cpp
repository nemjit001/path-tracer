#include "renderer.hpp"

#define STB_IMAGE_WRITE_IMPLEMENTATION

#include <cstdio>
#include <vector>
#include <stb_image_write.h>

#include "ray.hpp"
#include "sampler.hpp"

void Renderer::render(RendererConfig const& config, Camera const& camera, Integrator const& integrator)
{
	std::vector<glm::vec4> image(config.resolutionX * config.resolutionY);
	ViewPyramid const view = camera.generateViewPyramid();

	// Render frame
	printf("Starting render\n");
	for (uint32_t y = 0; y < config.resolutionY; y++)
	{
		printf("Scanline %u/%u\n", y + 1, config.resolutionY);
		for (uint32_t x = 0; x < config.resolutionX; x++)
		{
			uint32_t const pixelSeed = (x + y * config.resolutionX) + 0x1234;
			WhiteNoiseSampler sampler(pixelSeed);

			glm::vec3 sample{};
			for (uint32_t s = 0; s < config.sampleCount; s++)
			{
				// Get pixel as floats
				float const px = static_cast<float>(x);
				float const py = static_cast<float>(y);

				// Calculate pixel UV w/ jitter for anti aliasing
				glm::vec2 const jitter = sampler.sample2D(); //< samples in range [0, 1]
				float const u = (px + jitter.x) / static_cast<float>(config.resolutionX);
				float const v = (py + jitter.y) / static_cast<float>(config.resolutionY);

				// Set up ray
				glm::vec3 const viewPosition = view.pxTopLeft + u * (view.pxTopRight - view.pxTopLeft) + v * (view.pxBottomLeft - view.pxTopLeft);
				glm::vec3 const origin = view.origin;
				glm::vec3 const direction = glm::normalize(viewPosition - origin);
				Ray const primary(origin, direction);

				// Sample scene integrator
				sample += integrator.trace(primary, sampler);
			}

			sample /= static_cast<float>(config.sampleCount);
			image[x + y * config.resolutionX] = { sample, 1.0F };
		}
	}
	printf("Completed render\n");

	// Write out image
	std::vector<uint32_t> bytes(config.resolutionX * config.resolutionY);
	for (size_t i = 0; i < image.size(); i++)
	{
		// Do gamma conversion
		glm::vec4 const& pixel = image[i];
		glm::vec4 const gamma = glm::vec4(glm::pow(glm::vec3(pixel), glm::vec3(1.0F / 2.2F)), pixel.a);

		// Do byte packing
		uint32_t const r = static_cast<uint32_t>(gamma.r * 255.99F) & 0xFF;
		uint32_t const g = static_cast<uint32_t>(gamma.g * 255.99F) & 0xFF;
		uint32_t const b = static_cast<uint32_t>(gamma.b * 255.99F) & 0xFF;
		uint32_t const a = static_cast<uint32_t>(gamma.a * 255.99F) & 0xFF;
		bytes[i] = (a << 24) + (b << 16) + (g << 8) + r;
	}

	stbi_write_png(config.filename.c_str(), config.resolutionX, config.resolutionY, 4, bytes.data(), sizeof(uint32_t) * config.resolutionX);
}
