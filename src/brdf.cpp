#include "brdf.hpp"

static constexpr float PI		= 3.14159265358979F;
static constexpr float TWO_PI	= 2.0F * PI;
static constexpr float INV_PI	= 1.0F / PI;
static constexpr float INV_2PI	= 1.0F / TWO_PI;

glm::vec3 sampleCosineWeightedHemisphere(Sampler& sampler)
{
	glm::vec2 const eta = sampler.sample2D();
	float const theta = glm::acos(glm::sqrt(eta.x));
	float const phi = TWO_PI * eta.y;

	return glm::vec3(glm::sin(theta) * glm::cos(phi), glm::sin(theta) * glm::sin(phi), glm::cos(theta));
}

glm::vec3 sampleGGX(Sampler& sampler, glm::vec3 const& wi, float alpha)
{
	// Calculate microfacet normal polar coordinates using Disney's GTR2 sampling
	glm::vec2 const eta = sampler.sample2D();
	float const a2 = alpha * alpha;
	float const r = (1.0F - eta.x) / (1.0F + (a2 - 1.0F) * eta.x);
	float const theta = glm::acos(glm::sqrt(r));
	float const phi = TWO_PI * eta.y;

	// Transform polar to vector
	glm::vec3 const m = glm::vec3(glm::sin(theta) * glm::cos(phi), glm::sin(theta) * glm::sin(phi), glm::cos(theta));
	return glm::reflect(-wi, m);
}

float DGGX(glm::vec3 const& m, glm::vec3 const& n, float alpha)
{
	float const a2 = alpha * alpha;
	float const cosTheta = glm::dot(m, n);
	float const d = 1.0F + (a2 - 1.0F) * cosTheta * cosTheta;
	return a2 / (PI * d * d);
}

float FSchlick(glm::vec3 const& v, glm::vec3 const& n, float F)
{
	float c = 1.0F - glm::clamp(glm::dot(n, v), 0.0F, 1.0F);
	float c5 = c * c * c * c * c;
	return 1.0F + (F - 1.0F) * c5;
}

float G1Schlick(glm::vec3 const& v, glm::vec3 const& n, float alpha)
{
	float const NoV = glm::clamp(glm::dot(n, v), 0.0F, 1.0F);
	float const a2 = alpha * alpha;
	float const k = glm::sqrt(2.0F * a2 * INV_PI);

	return NoV / (NoV - (k * NoV) + k);
}

glm::vec3 evaluateLambertianDiffuseBRDF(Sampler& sampler, Material const& material, glm::vec3 const& wi, glm::vec3 const& n, glm::vec3& wo)
{
	wo = sampleCosineWeightedHemisphere(sampler);
	glm::vec3 const brdf = material.baseColor * INV_PI;
	float const pdf = glm::dot(wo, n) * INV_PI;

	return (brdf * glm::dot(wo, n)) / pdf;
}

glm::vec3 evaluateDielectricMicrofacetBRDF(Sampler& sampler, Material const& material, glm::vec3 const& wi, glm::vec3 const& n, glm::vec3& wo)
{
	// Set up microfacet model parameters
	float const alpha = material.roughness * material.roughness;

	// Sample Diffuse & GGX lobes
	glm::vec3 const m = sampleGGX(sampler, wi, alpha);
	glm::vec3 const woDiffuse = sampleCosineWeightedHemisphere(sampler);
	glm::vec3 const woSpecular = glm::reflect(-wi, m);

	// Calculate F0 for dielectric material
	float const eta1 = material.IOR - 1.0F;
	float const eta2 = material.IOR + 1.0F;
	float const iorRatio = eta1 / eta2;
	float const F0 = iorRatio * iorRatio;

	// Calculate Microfacet terms
	float const D = DGGX(m, n, alpha);
	float const G = G1Schlick(wi, m, alpha) * G1Schlick(woSpecular, m, alpha); // Smith's G term
	float const F = FSchlick(wi, m, F0);

	// Calculate MIS PDF weights
	float const diffuseWeight = material.roughness;		// Diffuse chance
	float const specularWeight = 1.0F - diffuseWeight;	// Specular chance

	float const pdfDiffuse = glm::dot(woDiffuse, n) * INV_PI;
	float const pdfSpecular = (D * glm::dot(m, n)) / (4.0F * glm::abs(glm::dot(woSpecular, m)));

	// Calculate composite PDF for MIS
	float const compositePDF = (diffuseWeight * pdfDiffuse) + (specularWeight * pdfSpecular);

	// Perform MIS for lambertian & GGX lobes
	glm::vec3 brdf{};
	float pdf{};
	float weightMIS{};
	if (sampler.sample() < diffuseWeight)
	{
		// Evaluate diffuse
		wo = woDiffuse;

		float const F90 = 0.5F + 2.0F * material.roughness * glm::dot(wi, m) * glm::dot(wi, m);
		float const a = FSchlick(wi, n, F90);
		float const b = FSchlick(wo, n, F90);

		brdf = material.baseColor * INV_PI * a * b;
		pdf = pdfDiffuse;
		weightMIS = (diffuseWeight * pdfDiffuse) / compositePDF;
	}
	else
	{
		// Evaluate specular
		wo = woSpecular;

		brdf = glm::vec3((D * G * F) / (4.0F * glm::abs(glm::dot(wi, n)) * glm::abs(glm::dot(wo, n))));
		pdf = pdfSpecular;
		weightMIS = (specularWeight * pdfSpecular) / compositePDF;
	}

	return weightMIS * ((brdf * glm::dot(wo, n)) / pdf);
}
