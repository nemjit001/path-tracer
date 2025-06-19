#pragma once

#include <glm/glm.hpp>

#include "sampler.hpp"

/// @brief BRDF interface for use by integrators.
class BRDF
{
public:
	virtual ~BRDF() = default;

	/// @brief Sample the BRDF using an incoming direction and sampler.
	/// @param sampler Random sampler to use for direction sampling.
	/// @param wi Incoming direction, assumed to be normalized.
	/// @param n Shading normal.
	/// @return A normalized outgoing direction.
	virtual glm::vec3 sample(Sampler& sampler, glm::vec3 const& wi, glm::vec3 const& n) const = 0;

	/// @brief Evaluate the BRDF for the reflectance.
	/// @param wi Incoming direction, assumed to be normalized.
	/// @param wo Outgoing direction, assumed to be normalized.
	/// @param n Shading normal.
	/// @return A 3 component reflectance color.
	virtual glm::vec3 evaluate(glm::vec3 const& wi, glm::vec3 const& wo, glm::vec3 const& n) const = 0;

	/// @brief Sample the PDF associated with this BRDF.
	/// @param wi Incoming light direction.
	/// @param wo Sampled outgoing direction.
	/// @param n Shading normal.
	/// @return The PDF for this sampled direction.
	virtual float pdf(glm::vec3 const& wi, glm::vec3 const& wo, glm::vec3 const& n) const = 0;
};

/// @brief Perfectly diffuse Lambertian BRDF.
class LambertianBRDF : public BRDF
{
public:
	LambertianBRDF(glm::vec3 const& baseColor);
	~LambertianBRDF() = default;
	
	LambertianBRDF(LambertianBRDF const&) = default;
	LambertianBRDF& operator=(LambertianBRDF const&) = default;

	glm::vec3 sample(Sampler& sampler, glm::vec3 const& wi, glm::vec3 const& n) const override;

	glm::vec3 evaluate(glm::vec3 const& wi, glm::vec3 const& wo, glm::vec3 const& n) const override;

	float pdf(glm::vec3 const& wi, glm::vec3 const& wo, glm::vec3 const& n) const override;

private:
	glm::vec3 m_baseColor;
};

/// @brief Perfectly smooth metallic BRDF.
class SmoothMetallicBRDF : public BRDF
{
public:
	SmoothMetallicBRDF(glm::vec3 const& baseColor);
	~SmoothMetallicBRDF() = default;

	SmoothMetallicBRDF(SmoothMetallicBRDF const&) = default;
	SmoothMetallicBRDF& operator=(SmoothMetallicBRDF const&) = default;

	glm::vec3 sample(Sampler& sampler, glm::vec3 const& wi, glm::vec3 const& n) const override;

	glm::vec3 evaluate(glm::vec3 const& wi, glm::vec3 const& wo, glm::vec3 const& n) const override;

	float pdf(glm::vec3 const& wi, glm::vec3 const& wo, glm::vec3 const& n) const override;

private:
	glm::vec3 m_baseColor;
};

/// @brief Dielectric microfacet BRDF.
class DielectricBRDF : public BRDF
{
public:
	DielectricBRDF(glm::vec3 const& baseColor, float roughness, float IOR);
	~DielectricBRDF() = default;

	DielectricBRDF(DielectricBRDF const&) = default;
	DielectricBRDF& operator=(DielectricBRDF const&) = default;

	glm::vec3 sample(Sampler& sampler, glm::vec3 const& wi, glm::vec3 const& n) const override;

	glm::vec3 evaluate(glm::vec3 const& wi, glm::vec3 const& wo, glm::vec3 const& n) const override;

	float pdf(glm::vec3 const& wi, glm::vec3 const& wo, glm::vec3 const& n) const override;

private:
	float DGGX(glm::vec3 const& m, glm::vec3 const& n) const;

	glm::vec3 FSchlick(glm::vec3 const& v, glm::vec3 const& n, glm::vec3 const& F0) const;

	float G1Schlick(glm::vec3 const& n, glm::vec3 const& v) const;

private:
	glm::vec3	m_albedo;		//< diffuse albedo
	float		m_roughness;	//< material roughness
	float		m_alpha;		//< microfacet roughness
	float		m_ior;			//< F0 ior
};
