#pragma once

#include "Utils.h"
typedef struct {
	int m_position[3];
	int m_normal[3];
	int m_texcoord[3];

}faceDescriptor;

class Mesh
{
protected:
	Mesh();

public:
	~Mesh();

	static Mesh* loadFromOBJ(const char* p_path);

	std::vector<glm::vec3> m_VecPositions;
	std::vector<glm::vec3> m_VecNormals;
	std::vector<glm::vec3> m_VecTexCoords;
	std::vector<faceDescriptor> m_faces;

};
