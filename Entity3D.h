#pragma once
#include "Utils.h"


class Entity3D
{
public :
	Entity3D(glm::vec3 p_position = glm::vec3(0, 0, 0), glm::quat p_quaternion = glm::quat(1, 0, 0, 0)) :
		m_position(p_position),
		m_quaternion(p_quaternion),
		m_model(1.0f),
		m_modelInverse(1.0f)
	{
		updateModel();
	}
	~Entity3D() {}

	void setPosition(glm::vec3 p_pos) { m_position = p_pos; updateModel(); }
	void setOrientation(glm::vec3 p_orientation) { m_quaternion= glm::quat(p_orientation); updateModel(); }

	glm::vec3 getPosInWorld(glm::vec3 p_posInObject) { return (m_model * glm::vec4(p_posInObject, 1.0f)).xyz(); }
	glm::vec3 getDirInWorld(glm::vec3 p_dirInObject) { return (m_model * glm::vec4(p_dirInObject, 0.0f)).xyz(); }
	glm::vec3 getPosInObject(glm::vec3 p_posInWorld) { return (m_modelInverse * glm::vec4(p_posInWorld, 1.0f)).xyz(); }
	glm::vec3 getDirInObject(glm::vec3 p_dirInWorld) { return (m_modelInverse * glm::vec4(p_dirInWorld, 0.0f)).xyz(); }

protected:
	void updateModel() { m_model = glm::translate(
											glm::rotate(
												glm::mat4(1.0f), 
												glm::angle(m_quaternion), glm::axis(m_quaternion)
											),
											m_position);
						m_modelInverse = glm::inverse(m_model);
					   }

	glm::vec3 m_position;
	glm::quat m_quaternion;

	glm::mat4 m_model;
	glm::mat4 m_modelInverse;


};

