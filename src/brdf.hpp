#pragma once

#include <glm/glm.hpp>

#include "sampler.hpp"

class BRDF
{
public:
	virtual ~BRDF() = default;

	virtual glm::vec3 sample(Sampler& sampler) const = 0;

	virtual glm::vec3 evaluate(glm::vec3 const& wi, glm::vec3 const& wo, glm::vec3 const& N) const = 0;

	virtual float pdf(glm::vec3 const& wo, glm::vec3 const& N) const = 0;
};

class LambertianBRDF : public BRDF
{
public:
	LambertianBRDF(glm::vec3 const& baseColor);
	~LambertianBRDF() = default;
	
	LambertianBRDF(LambertianBRDF const&) = default;
	LambertianBRDF& operator=(LambertianBRDF const&) = default;

	glm::vec3 sample(Sampler& sampler) const override;

	glm::vec3 evaluate(glm::vec3 const& wi, glm::vec3 const& wo, glm::vec3 const& N) const override;

	float pdf(glm::vec3 const& wo, glm::vec3 const& N) const override;

private:
	glm::vec3 m_baseColor;
};
