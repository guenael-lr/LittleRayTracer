#pragma once
#include "Utils.h"
#include "Material.h"

class Collider;

struct RaycastHit {
	float t;
	glm::vec3 hitPosition;
	glm::vec3 hitNormal;
	Collider* hitCollider;
	Material hitMaterial;
};

class Collider
{
public:
	virtual bool raycast(glm::vec3 p_originWorld, glm::vec3 p_dirWorld, glm::vec2 p_interval, RaycastHit& p_hit) = 0;
	
};

