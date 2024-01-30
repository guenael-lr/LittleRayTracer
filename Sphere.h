#pragma once
#include "Utils.h"
#include "Object.h"
#include "Collider.h"

class Sphere: public Object
{
	public:
		Sphere(glm::vec3 p_position, float p_radius) :Object(p_position),
			m_radius(p_radius)
		{
		}
		~Sphere() {}

		bool raycast(glm::vec3 p_originWorld, glm::vec3 p_dirWorld, glm::vec2 p_interval, RaycastHit& p_hit) {
			// t²*(dir.dir) + t* 2 * (dir.(O-C)) + (O-C).(O-C) - r² = 0
			// delta = b²-4ac
			glm::vec3 sphereOrigin = p_originWorld - m_position;
			float a = glm::dot(p_dirWorld, p_dirWorld);
			float b = 2.0f * glm::dot(p_dirWorld, sphereOrigin);
			float c = glm::dot(sphereOrigin, sphereOrigin) - m_radius * m_radius;

			float delta = b * b - 4.0f * a * c;
			if (delta < 0)
				return false;

			float t = 0.0f;
			if (delta == 0.0f)
			{
				t = -b / (2.0f * a);
				if (t <= 0.0f)
					return false;
			}
			else
			{
				// delta >0
				float t2 = (-b + glm::sqrt(delta)) / (2.0f * a);
				if (t2 <= 0)
					return false;

				float t1 = (-b - glm::sqrt(delta)) / (2.0f * a);
				if (t1 <= 0)
					t = t2;
				else
					t = t1;
			}
			if (t > p_interval.y|| t < p_interval.x)
				return false;


			p_hit.t = t;
			p_hit.hitPosition = p_originWorld + t * p_dirWorld;
			p_hit.hitNormal = glm::normalize(p_hit.hitPosition - m_position);
			p_hit.hitCollider = this;
			p_hit.hitMaterial = material;
			p_hit.hitUV[0] = glm::vec3(0.0f);
			p_hit.hitUV[1] = glm::vec3(0.0f);
			p_hit.hitUV[2] = glm::vec3(0.0f);
			return true;
		}

	protected:
		float m_radius;

};

