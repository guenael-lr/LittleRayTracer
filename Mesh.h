#pragma once

#include "Utils.h"

#pragma pack(push,1)
typedef struct {
	glm::vec3 position;
	glm::vec3 normal; 
	glm::vec2 texcoords;
}vertexDescriptor;

typedef struct {
	glm::vec3 v_position;
	glm::vec3 v_normal;
	glm::vec3 v_texcoords;

}faceDescriptor;

#pragma pack(pop)


class Mesh
{
protected:
	Mesh();

public:
	~Mesh();

	static Mesh* loadFromOBJ(const char* p_path);

	

	std::vector<vertexDescriptor> m_vertices;
	std::vector<faceDescriptor> m_faces;

};
