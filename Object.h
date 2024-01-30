#pragma once
#include "Utils.h"
#include "Entity3D.h"
#include "Collider.h"
#include "Material.h"


class Object: public Entity3D, public Collider
{
public:
	Object(glm::vec3 p_position = glm::vec3(0, 0, 0), glm::quat p_quaternion = glm::quat(1, 0, 0, 0)) :Entity3D(p_position, p_quaternion) 
	{
		material = new Material();
		material->emissive = glm::vec3(0, 0, 0);
		material->color = glm::vec3(glm::linearRand(0.0f, 1.0f), glm::linearRand(0.0f, 1.0f), glm::linearRand(0.0f, 1.0f));
		material->roughness = glm::linearRand(0.0f, 1.0f);
		material->metallic = glm::linearRand(0.0f, 1.0f);
	}

	virtual bool raycast(glm::vec3 p_origin, glm::vec3 p_dir, glm::vec2 p_interval, RaycastHit& p_hit)=0;
	Material* material;
};

