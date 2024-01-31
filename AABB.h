#pragma once
#include "Utils.h"
#include "Collider.h"

class AABB : public Collider
{
	//AABB is the bounding box of a list of vertices (or a mesh)
	public:
		AABB(glm::vec3 p_min, glm::vec3 p_max) : m_min(p_min), m_max(p_max) {}
		~AABB() {}

		bool raycast(glm::vec3 p_originWorld, glm::vec3 p_dirWorld, glm::vec2 p_interval, RaycastHit& p_hit);
		
		glm::vec3 m_min;
		glm::vec3 m_max;

		bool isPointInside(glm::vec3 p_point);
};

