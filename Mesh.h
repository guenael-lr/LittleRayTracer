#pragma once

#include "Utils.h"

#pragma pack(push,1)
typedef struct {
	glm::vec3 position;
	glm::vec3 normal; 
	glm::vec2 texcoords;
}vertexDescriptor;

typedef glm::ivec3 faceDescriptor;

#pragma pack(pop)


class Mesh
{
protected:
	Mesh();

public:
	~Mesh();

	static Mesh* loadFromOBJ(const char* p_path);
	void draw();

protected:

	std::vector<vertexDescriptor> m_vertices;
	std::vector<faceDescriptor> m_faces;
	GLuint m_vbo_id;
	GLuint m_ebo_id;
	GLuint m_vao_id;
};
