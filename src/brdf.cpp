#include "brdf.hpp"

static constexpr float PI		= 3.14159265358979F;
static constexpr float TWO_PI	= 2.0F * PI;
static constexpr float INV_PI	= 1.0F / PI;
static constexpr float INV_2PI	= 1.0F / TWO_PI;

/// @brief Calculate the Luma value of a color (linear RGB to luma)
/// @param color 
/// @return 
float luma(glm::vec3 const& color)
{
	return glm::dot(color, glm::vec3(0.299F, 0.587F, 0.114F));
}

/// @brief Sample a cosine weighted hemisphere.
/// @param sampler 
/// @return 
glm::vec3 sampleCosineWeightedHemisphere(Sampler& sampler)
{
	glm::vec2 const eta = sampler.sample2D();
	float const theta = glm::acos(glm::sqrt(eta.x));
	float const phi = TWO_PI * eta.y;

	return glm::vec3(glm::sin(theta) * glm::cos(phi), glm::sin(theta) * glm::sin(phi), glm::cos(theta));
}

/// @brief Sample GTR2 GGX lobe as described in Physically Based Shading at Disney.
/// @param sampler 
/// @param wi Incoming view direction.
/// @param alpha Roughness of distribution.
/// @return A microfacet normal m from the GGX distribution.
glm::vec3 sampleGGX(Sampler& sampler, float alpha)
{
	// Calculate microfacet normal polar coordinates using Disney's GTR2 sampling
	glm::vec2 const eta = sampler.sample2D();
	float const a2 = alpha * alpha;
	float const r = (1.0F - eta.x) / (1.0F + (a2 - 1.0F) * eta.x);
	float const theta = glm::acos(glm::sqrt(r));
	float const phi = TWO_PI * eta.y;

	// Transform polar to vector
	glm::vec3 const m = glm::vec3(glm::sin(theta) * glm::cos(phi), glm::sin(theta) * glm::sin(phi), glm::cos(theta));
	return m;
}

/// @brief The GGX microfacet distribution as given by Physically Based Shading at Disney.
/// @param m Microfacet normal.
/// @param n Shading normal.
/// @param alpha Distribution roughness.
/// @return Desnitry of microfacet normals for direction m.
float DGGX(glm::vec3 const& m, glm::vec3 const& n, float alpha)
{
	float const a2 = alpha * alpha;
	float const cosTheta = glm::dot(m, n);
	float const d = 1.0F + (a2 - 1.0F) * cosTheta * cosTheta;
	return a2 / (PI * d * d);
}

/// @brief Schlick's Fresnel approximation.
/// @param v Light vector.
/// @param n Fresnel normal.
/// @param F Fresnel response, e.g. F0.
/// @return Fresnel reflectivity.
glm::vec3 FSchlick(glm::vec3 const& v, glm::vec3 const& n, glm::vec3 const& F)
{
	float c = 1.0F - glm::clamp(glm::dot(n, v), 0.0F, 1.0F);
	float c5 = c * c * c * c * c;
	return F + (1.0F - F) * c5;
}

/// @brief Schlick's G1 term for Smith's G term eval.
/// @param v Vector direction to eval G1 for.
/// @param n G term normal.
/// @param alpha G term roughness.
/// @return 
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

glm::vec3 evaluateDisneyBRDF(Sampler& sampler, Material const& material, glm::vec3 const& wi, glm::vec3 const& n, glm::vec3& wo)
{
	// Set up microfacet model parameters for Disney BSDF
	float const alpha = material.roughness * material.roughness;
	float const alpha_g = glm::pow(0.5F + 0.5F * alpha, 2.0F);

	// Sample GGX lobe
	glm::vec3 const m = sampleGGX(sampler, alpha);

	// Calculate F0 constants for dielectric material
	float const eta1 = material.IOR - 1.0F;
	float const eta2 = material.IOR + 1.0F;
	float const iorRatio = eta1 / eta2;

	// Lerp between dielectric and metallic F0
	glm::vec3 const F0 = glm::mix(glm::vec3(iorRatio * iorRatio), material.baseColor, material.metallic);

	// Calculate Microfacet Fresnel
	glm::vec3 const F = FSchlick(wi, m, F0);

	// Set up sampling weights
	// TODO(nemjit001): Check why this works as MIS without PDF eval in weight calc
	float diffW = 1.0F - material.metallic;
	float specW = luma(F);
	float const invW = 1.0F / (diffW + specW);
	diffW *= invW;
	specW *= invW;

	glm::vec3 brdf{};
	float pdf{};
	if (sampler.sample() < diffW)
	{
		// Sample diffuse lobe
		wo = sampleCosineWeightedHemisphere(sampler);

		// Calculate retroreflective highlight
		float const F90 = 0.5F + 2.0F * material.roughness * glm::dot(wi, m) * glm::dot(wi, m);
		glm::vec3 const a = FSchlick(wi, m, glm::vec3(F90));
		glm::vec3 const b = FSchlick(wo, m, glm::vec3(F90));

		// Lambertian diffuse w/ highlight
		brdf = material.baseColor * INV_PI * a * b;
		pdf = glm::dot(wo, n) * INV_PI;
		pdf *= diffW; // Apply diffuse MIS weight
	}
	else
	{
		// Evaluate specular direction
		wo = glm::reflect(-wi, m);

		// Calculate D
		float const D = DGGX(m, n, alpha);

		// Calculate Smith's G term
		float const G = G1Schlick(wi, m, alpha_g) * G1Schlick(wo, m, alpha_g);

		// Cook-Torrance BSDF using Walter et al. formulation
		brdf = (D * G * F) / (4.0F * glm::abs(glm::dot(wi, n)) * glm::abs(glm::dot(wo, n)));
		pdf = (D * glm::dot(m, n)) / (4.0F * glm::abs(glm::dot(wo, m)));
		pdf *= specW; // Apply specular MIS weight
	}

	return ((brdf * glm::dot(wo, n)) / pdf);
}
