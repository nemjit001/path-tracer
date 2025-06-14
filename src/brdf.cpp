#include "brdf.hpp"

static constexpr float PI		= 3.14159265358979F;
static constexpr float TWO_PI	= 2.0F * PI;
static constexpr float INV_PI	= 1.0F / PI;
static constexpr float INV_2PI	= 1.0F / TWO_PI;

#define COSINE_WEIGHTED_PDF	0

LambertianBRDF::LambertianBRDF(glm::vec3 const& baseColor)
	:
	m_baseColor(baseColor)
{
	//
}

glm::vec3 LambertianBRDF::sample(Sampler& sampler) const
{
	glm::vec3 dir{};
	do {
		dir.x = sampler.sampleRange(-1.0F, 1.0F);
		dir.y = sampler.sampleRange(-1.0F, 1.0F);
		dir.z = sampler.sampleRange(-1.0F, 1.0F);
	} while (glm::dot(dir, dir) > 1.0F);

	if (dir.z < 0.0F) {
		dir = -dir;
	}

#if COSINE_WEIGHTED_PDF
	return glm::normalize(glm::vec3(0, 0, 1) + glm::normalize(dir)); //< cosine weighting
#else
	return glm::normalize(dir);
#endif
}

glm::vec3 LambertianBRDF::evaluate(glm::vec3 const& wi, glm::vec3 const& wo, glm::vec3 const& N) const
{
	return m_baseColor * INV_PI * glm::dot(wo, N);
}

float LambertianBRDF::pdf(glm::vec3 const& wo, glm::vec3 const& N) const
{
#if COSINE_WEIGHTED_PDF
	return glm::dot(wo, N) * INV_PI;
#else
	return INV_2PI;
#endif
}
