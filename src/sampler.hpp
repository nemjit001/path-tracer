#pragma once

#include <cstdint>
#include <glm/glm.hpp>

/// @brief The Sampler interface exposes random sampling methods for rendering.
class Sampler
{
public:
	virtual ~Sampler() = default;

	virtual float sample() = 0;

	float		sampleRange(float min, float max)	{ return (max - min) * sample() + min; };
	glm::vec2	sample2D()							{ return { sample(), sample()}; }
	glm::vec2	sampleRange2D(float min, float max)	{ return { sampleRange(min, max), sampleRange(min, max) }; }
};

/// @brief The WhiteNoiseSampler implements a white noise random source for sampling.
class WhiteNoiseSampler : public Sampler
{
public:
	WhiteNoiseSampler(uint32_t seed)
		: m_state(seed) {}

	float sample() override;

private:
	uint32_t m_state;
};
