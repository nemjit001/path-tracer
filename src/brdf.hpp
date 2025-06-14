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
	/// @param N Shading normal.
	/// @return A normalized outgoing direction.
	virtual glm::vec3 sample(Sampler& sampler, glm::vec3 const& wi, glm::vec3 const& N) const = 0;

	/// @brief Evaluate the BRDF for the reflectance.
	/// @param wi Incoming direction, assumed to be normalized.
	/// @param wo Outgoing direction, assumed to be normalized.
	/// @param N Shading normal.
	/// @return A 3 component reflectance color.
	virtual glm::vec3 evaluate(glm::vec3 const& wi, glm::vec3 const& wo, glm::vec3 const& N) const = 0;

	/// @brief Sample the PDF associated with this BRDF.
	/// @param wo Sampled outgoing direction/
	/// @param N Shading normal.
	/// @return The PDF for this sampled direction.
	virtual float pdf(glm::vec3 const& wo, glm::vec3 const& N) const = 0;
};

/// @brief Perfectly diffuse Lambertian BRDF.
class LambertianBRDF : public BRDF
{
public:
	LambertianBRDF(glm::vec3 const& baseColor);
	~LambertianBRDF() = default;
	
	LambertianBRDF(LambertianBRDF const&) = default;
	LambertianBRDF& operator=(LambertianBRDF const&) = default;

	glm::vec3 sample(Sampler& sampler, glm::vec3 const& wi, glm::vec3 const& N) const override;

	glm::vec3 evaluate(glm::vec3 const& wi, glm::vec3 const& wo, glm::vec3 const& N) const override;

	float pdf(glm::vec3 const& wo, glm::vec3 const& N) const override;

private:
	glm::vec3 m_baseColor;
};
