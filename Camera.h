#pragma once
#include "Entity3D.h"

class Camera: public Entity3D
{
public :
	Camera(float p_focalPlaneDistance = 1.0f, glm::vec2 p_focalPlaneSize = glm::vec2(2.0f,2.0f)) :Entity3D(),
		m_focalPlaneDistance(p_focalPlaneDistance),
		m_focalPlaneSize(p_focalPlaneSize)
	{}

	void getFocalPlane(float& p_distance, glm::vec2& p_size) { p_distance = m_focalPlaneDistance; p_size = m_focalPlaneSize; }
	
	glm::vec3 getDirFromPixel(glm::ivec2 p_resolution, glm::ivec2 p_pixel) 
	{
		glm::vec2 pixelMultiSampling = glm::vec2(p_pixel) + glm::vec2(glm::linearRand(-0.5f, 0.5f), glm::linearRand(-0.5f, 0.5f));

		glm::vec3 posFocalPlaneInWorld		= getPosInWorld(glm::vec3(0, 0, -m_focalPlaneDistance));
		glm::vec3 rightDirFocalPlaneInWorld = getDirInWorld(glm::vec3(1, 0, 0));
		glm::vec3 upDirFocalPlaneInWorld	= getDirInWorld(glm::vec3(0, 1, 0));

		glm::vec3 plane00InWorld = posFocalPlaneInWorld
			- m_focalPlaneSize.x / 2.0f * rightDirFocalPlaneInWorld
			+ m_focalPlaneSize.y / 2.0f * upDirFocalPlaneInWorld;

		glm::vec3 pixelInWorld= plane00InWorld 
									+ pixelMultiSampling.x / (float)p_resolution.x * m_focalPlaneSize.x * rightDirFocalPlaneInWorld
									- pixelMultiSampling.y / (float)p_resolution.y * m_focalPlaneSize.y * upDirFocalPlaneInWorld;

		return glm::normalize( pixelInWorld - m_position);
	}


protected:
	float m_focalPlaneDistance;
	glm::vec2 m_focalPlaneSize;

};

