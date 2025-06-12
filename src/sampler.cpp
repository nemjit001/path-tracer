#include "sampler.hpp"

uint32_t xorshift32(uint32_t& seed)
{
	seed ^= seed << 13;
	seed ^= seed >> 17;
	seed ^= seed << 5;
	return seed;
}

float WhiteNoiseSampler::sample()
{
	return static_cast<float>(xorshift32(m_state)) * 2.3283064e-10F; //< convert to [0, 1] range
}
