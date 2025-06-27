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
	return glm::vec3(glm::sin(theta) * glm::cos(phi), glm::sin(theta) * glm::sin(phi), glm::cos(theta));
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
/// @param F0 Fresnel response.
/// @return Fresnel reflectivity.
glm::vec3 FSchlick(glm::vec3 const& v, glm::vec3 const& n, glm::vec3 const& F0)
{
	float c = 1.0F - glm::clamp(glm::dot(n, v), 0.0F, 1.0F);
	float c5 = c * c * c * c * c;
	return F0 + (1.0F - F0) * c5;
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

glm::vec3 evaluateDisneyDiffuseBRDF(glm::vec3 const& baseColor, float alpha, glm::vec3 const& wi, glm::vec3 const& wo, glm::vec3 const& m, glm::vec3 const& n)
{
	float const F90 = 0.5F + 2.0F * alpha * glm::dot(wi, m) * glm::dot(wi, m);
	float const ci = 1.0F - glm::dot(wi, m);
	float const co = 1.0F - glm::dot(wo, m);

	float const a = 1.0F + (F90 - 1.0F) * ci * ci * ci * ci * ci;
	float const b = 1.0F + (F90 - 1.0F) * co * co * co * co * co;

	return baseColor * INV_PI * a * b;
}

glm::vec3 evaluateDisneySpecularBRDF(float alpha, glm::vec3 const& F0, glm::vec3 const& wi, glm::vec3 const& wo, glm::vec3 const& m, glm::vec3 const& n)
{
	float const alpha_g = glm::pow(0.5F + 0.5F * alpha, 2.0F);

	float const D = DGGX(m, n, alpha);
	float const G = G1Schlick(wi, m, alpha_g) * G1Schlick(wo, m, alpha_g);
	glm::vec3 const F = FSchlick(wi, m, F0);

	return (D * G * F) / (4.0F * glm::abs(glm::dot(wi, n)) * glm::abs(glm::dot(wo, n)));
}

float evaluateCosineWeightedPDF(glm::vec3 const& n, glm::vec3 const& wo)
{
	return glm::dot(wo, n) * INV_PI;
}

float evaluateGGXPDF(float alpha, glm::vec3 const& m, glm::vec3 const& n, glm::vec3 const& wo)
{
	float const D = DGGX(m, n, alpha);
	return (D * glm::dot(m, n)) / (4.0F * glm::abs(glm::dot(wo, m)));
}

glm::vec3 sampleLambertianDiffuseBRDF(Sampler& sampler, Material const& material, glm::vec3 const& wi, glm::vec3 const& n, glm::vec3& wo)
{
	(void)(wi); // wi is unused for lambertian diffuse

	wo = sampleCosineWeightedHemisphere(sampler);
	glm::vec3 const brdf = material.baseColor * INV_PI;
	float const pdf = glm::dot(wo, n) * INV_PI;

	return (brdf * glm::dot(wo, n)) / pdf;
}

glm::vec3 sampleDisneyBRDF(Sampler& sampler, Material const& material, glm::vec3 const& wi, glm::vec3 const& n, glm::vec3& wo)
{
	// Set up microfacet model parameters for Disney BSDF & sample specular lobe
	float const alpha = glm::max(material.roughness * material.roughness, 1e-3F);
	glm::vec3 const m = sampleGGX(sampler, alpha);

	// Calculate F0 constants for dielectric material
	float const eta1 = material.IOR - 1.0F;
	float const eta2 = material.IOR + 1.0F;
	float const iorRatio = eta1 / eta2;

	// Lerp between dielectric and metallic F0 & calculate microfacet Fresnel
	glm::vec3 const F0 = glm::mix(glm::vec3(iorRatio * iorRatio), material.baseColor, material.metallic);
	glm::vec3 const F = FSchlick(wi, m, F0);

	// Evaluate diffuse & sepcular directions
	glm::vec3 const diffwo = sampleCosineWeightedHemisphere(sampler);
	glm::vec3 const specwo = glm::reflect(-wi, m);

	// Calculate lobe PDFs
	float const diffPDF = evaluateCosineWeightedPDF(n, diffwo);
	float const specPDF = evaluateGGXPDF(alpha, m, n, specwo);

	// Calculate normalized lobe sample weights
	float diffWeight = 1.0F - material.metallic;	// Only sample diffuse when dielectric is non-zero
	float specWeight = luma(F);						// Sample specular based on Fresnel luma
	float const invWeights = 1.0F / (diffWeight + specWeight);
	diffWeight *= invWeights;
	specWeight *= invWeights;

	glm::vec3 brdf{};
	float pdf = 0.0F;
	float pdfWeight = 0.0F;
	if (sampler.sample() < diffWeight)
	{
		// Lambertian diffuse w/ highlight
		brdf = evaluateDisneyDiffuseBRDF(material.baseColor, alpha, wi, diffwo, m, n);
		pdf = diffPDF;
		pdfWeight = diffWeight;

		wo = diffwo;
	}
	else
	{
		// Cook-Torrance BSDF using Walter et al. formulation
		brdf = evaluateDisneySpecularBRDF(alpha, F0, wi, specwo, m, n);
		pdf = specPDF;
		pdfWeight = specWeight;

		wo = specwo;
	}

	// Calculate composite MIS PDF & this MIS sample's weight
	float const compositePDF = (diffWeight * diffPDF + specWeight * specPDF);
	float const MISWeight = (pdfWeight * pdf) / compositePDF;
	return MISWeight * ((brdf * glm::abs(glm::dot(wo, n))) / pdf);
}
