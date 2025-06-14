#include "brdf.hpp"

static constexpr float PI		= 3.14159265358979F;
static constexpr float TWO_PI	= 2.0F * PI;
static constexpr float INV_PI	= 1.0F / PI;
static constexpr float INV_2PI	= 1.0F / TWO_PI;

#define COSINE_WEIGHTED_PDF	1

LambertianBRDF::LambertianBRDF(glm::vec3 const& baseColor)
	:
	m_baseColor(baseColor)
{
	//
}

glm::vec3 LambertianBRDF::sample(Sampler& sampler, glm::vec3 const& wi, glm::vec3 const& N) const
{
#if COSINE_WEIGHTED_PDF
	glm::vec2 const r = sampler.sample2D();
	float const theta = glm::acos(glm::sqrt(r.x));
	float const phi = TWO_PI * r.y;

	return glm::vec3(glm::sin(theta) * glm::cos(phi), glm::sin(theta) * glm::sin(phi), glm::cos(theta));
#else
	glm::vec3 dir{};
	do {
		dir.x = sampler.sampleRange(-1.0F, 1.0F);
		dir.y = sampler.sampleRange(-1.0F, 1.0F);
		dir.z = sampler.sampleRange(-1.0F, 1.0F);
	} while (glm::dot(dir, dir) > 1.0F);

	if (dir.z < 0.0F) {
		dir = -dir;
	}
	return glm::normalize(dir);
#endif
}

glm::vec3 LambertianBRDF::evaluate(glm::vec3 const& wi, glm::vec3 const& wo, glm::vec3 const& N) const
{
	return m_baseColor * INV_PI;
}

float LambertianBRDF::pdf(glm::vec3 const& wo, glm::vec3 const& N) const
{
#if COSINE_WEIGHTED_PDF
	return glm::dot(wo, N) * INV_PI;
#else
	return INV_2PI;
#endif
}
