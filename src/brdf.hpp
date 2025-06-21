#pragma once

#include <glm/glm.hpp>

#include "material.hpp"
#include "sampler.hpp"

glm::vec3 evaluateLambertianDiffuseBRDF(Sampler& sampler, Material const& material, glm::vec3 const& wi, glm::vec3 const& n, glm::vec3& wo);

glm::vec3 evaluateDisneyBRDF(Sampler& sampler, Material const& material, glm::vec3 const& wi, glm::vec3 const& n, glm::vec3& wo);
