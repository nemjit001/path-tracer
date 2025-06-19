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

glm::vec3 LambertianBRDF::sample(Sampler& sampler, glm::vec3 const& wi, glm::vec3 const& n) const
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

glm::vec3 LambertianBRDF::evaluate(glm::vec3 const& wi, glm::vec3 const& wo, glm::vec3 const& n) const
{
	return m_baseColor * INV_PI;
}

float LambertianBRDF::pdf(glm::vec3 const& wi, glm::vec3 const& wo, glm::vec3 const& n) const
{
#if COSINE_WEIGHTED_PDF
	return glm::dot(wo, n) * INV_PI;
#else
	return INV_2PI;
#endif
}

SmoothMetallicBRDF::SmoothMetallicBRDF(glm::vec3 const& baseColor)
	:
	m_baseColor(baseColor)
{
	//
}

glm::vec3 SmoothMetallicBRDF::sample(Sampler& sampler, glm::vec3 const& wi, glm::vec3 const& n) const
{
	return glm::reflect(-wi, n);
}

glm::vec3 SmoothMetallicBRDF::evaluate(glm::vec3 const& wi, glm::vec3 const& wo, glm::vec3 const& n) const
{
	return m_baseColor;
}

float SmoothMetallicBRDF::pdf(glm::vec3 const& wi, glm::vec3 const& wo, glm::vec3 const& n) const
{
	return 1.0F;
}

DielectricBRDF::DielectricBRDF(glm::vec3 const& baseColor, float roughness, float IOR)
	:
	m_albedo(baseColor),
	m_roughness(roughness),
	m_alpha(roughness * roughness),
	m_ior(IOR)
{
	// Clamp alpha to avoid NaNs during sampling at 0 roughness
	m_alpha = glm::clamp(m_alpha, 1e-3F, 1.0F);
}

glm::vec3 DielectricBRDF::sample(Sampler& sampler, glm::vec3 const& wi, glm::vec3 const& n) const
{
	// Calculate microfacet normal polar coordinates using Disney's GTR2 sampling
	glm::vec2 const eta = sampler.sample2D();
	float const a2 = m_alpha * m_alpha;
	float const r = (1.0F - eta.x) / (1.0F + (a2 - 1.0F) * eta.x);
	float const theta = glm::acos(glm::sqrt(r));
	float const phi = TWO_PI * eta.y;

	// Transform polar to vector
	glm::vec3 const& m = glm::vec3(glm::sin(theta) * glm::cos(phi), glm::sin(theta) * glm::sin(phi), glm::cos(theta));
	return glm::reflect(-wi, glm::normalize(m));
}

glm::vec3 DielectricBRDF::evaluate(glm::vec3 const& wi, glm::vec3 const& wo, glm::vec3 const& n) const
{
	// Calculate microfacet normal
	glm::vec3 const& m = glm::normalize(wi + wo);

	// Air-Material transition
	float const eta1 = m_ior - 1.0F;
	float const eta2 = m_ior + 1.0F;
	float const iorRatio = (eta1 / eta2);
	glm::vec3 const F0 = glm::vec3(iorRatio * iorRatio);

	// Calculate Smith G term
	float const GSmith = G1Schlick(wi, m) * G1Schlick(wo, m);

	// Calculate diffuse & specular components
	glm::vec3 const diffuse = m_albedo * INV_PI;
	glm::vec3 const specular = (DGGX(m, n) * FSchlick(wi, m, F0) * GSmith) / (4.0F * glm::abs(glm::dot(wi, n)) * glm::abs(glm::dot(wo, n)));

	return diffuse + specular;
}

float DielectricBRDF::pdf(glm::vec3 const& wi, glm::vec3 const& wo, glm::vec3 const& n) const
{
	glm::vec3 const m = glm::normalize(wi + wo);
	return (DGGX(m, n) * glm::dot(m, n)) / (4.0F * glm::abs(glm::dot(wo, m)));
}

float DielectricBRDF::G1Schlick(glm::vec3 const& n, glm::vec3 const& v) const
{
	float const NoV = glm::clamp(glm::dot(v, n), 0.0F, 1.0F);
	float const a2 = m_alpha * m_alpha;
	float const k = glm::sqrt(2.0F * a2 * INV_PI);

	return NoV / (NoV - (k * NoV) + k);
}

glm::vec3 DielectricBRDF::FSchlick(glm::vec3 const& v, glm::vec3 const& n, glm::vec3 const& F0) const
{
	float c = 1.0F - glm::clamp(glm::dot(v, n), 0.0F, 1.0F);
	float c5 = c * c * c * c * c;
	return 1.0F + (F0 - 1.0F) * c5;
}

float DielectricBRDF::DGGX(glm::vec3 const& m, glm::vec3 const& n) const
{
	float const a2 = m_alpha * m_alpha;
	float const cosTheta2 = glm::dot(n, m) * glm::dot(n, m);
	float const d = 1.0F + (a2 - 1.0F) * cosTheta2;
	return a2 / (PI * d * d);
}
