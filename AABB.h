#pragma once
#include "Utils.h"
#include "Collider.h"

class AABB : public Collider
{
	//AABB is the bounding box of a list of vertices (or a mesh)
	public:
		AABB(glm::vec3 p_min, glm::vec3 p_max) : m_min(p_min), m_max(p_max) {}
		~AABB() {}

		bool raycast(glm::vec3 p_originWorld, glm::vec3 p_dirWorld, glm::vec2 p_interval, RaycastHit& p_hit) {
			// t²*(dir.dir) + t* 2 * (dir.(O-C)) + (O-C).(O-C) - r² = 0
			// delta = b²-4ac

			

			glm::vec3 dirInv = 1.0f / p_dirWorld;
			glm::vec3 t1 = (m_min - p_originWorld) * dirInv;
			glm::vec3 t2 = (m_max - p_originWorld) * dirInv;

			glm::vec3 tmin = glm::min(t1, t2);
			glm::vec3 tmax = glm::max(t1, t2);

			float tminMax = glm::max(tmin.x, glm::max(tmin.y, tmin.z));
			float tmaxMin = glm::min(tmax.x, glm::min(tmax.y, tmax.z));

			if (tminMax > tmaxMin)
				return false;

			float t = 0.0f;
			if (tminMax < 0.0f)
			{
				t = tmaxMin;
				if (t > p_interval.y || t < p_interval.x)
					return false;
			}
			else
			{
				t = tminMax;
				if (t > p_interval.y || t < p_interval.x)
				{
					t = tmaxMin;
					if (t > p_interval.y || t < p_interval.x)
						return false;
				}
			}

			p_hit.t = t;
			p_hit.hitPosition = p_originWorld + t * p_dirWorld;
			p_hit.hitNormal = glm::vec3(0, 0, 0);
			p_hit.hitCollider = this;
			p_hit.hitMaterial = new Material();
			p_hit.hitMaterial->color = //red for debug
				glm::vec3(1, 0, 0);
			return true;
		}
		
		glm::vec3 m_min;
		glm::vec3 m_max;

		bool isPointInside(glm::vec3 p_point){
			if (p_point.x <= m_min.x || p_point.x >= m_max.x)
				return false;
			if (p_point.y <= m_min.y || p_point.y >= m_max.y)
				return false;
			if (p_point.z <= m_min.z || p_point.z >= m_max.z)
				return false;
			return true;
		}

		

};

