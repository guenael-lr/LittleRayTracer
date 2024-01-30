#pragma once

#include "Utils.h"

#pragma pack(push,1)

typedef struct {
	int v_position[3];
	int v_normal[3];
	int v_texcoord[3];

}faceDescriptor;

#pragma pack(pop)


class Mesh
{
protected:
	Mesh();

public:
	~Mesh();

	static Mesh* loadFromOBJ(const char* p_path);

	std::vector<glm::vec3> v_Positions;
	std::vector<glm::vec3> v_Normals;
	std::vector<glm::vec3> v_TexCoords;
	std::vector<faceDescriptor> m_faces;

};
