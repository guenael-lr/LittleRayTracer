#pragma once
#include "Utils.h"
#include "Object.h"
#include "AABB.h"
#include "Mesh.h"



class ObjMesh : public Object
{
	//Use mesh.h to get the mesh data
	//Use AABB object to know if raycast near the mesh 
	//The raycast to the mesh faces to know pixel informations

	public:
		Mesh* mesh;

		//vector of colliders
		std::vector<AABB*> colliders;
		
		//table of three vertices for each collider
		std::vector<std::vector<faceDescriptor>> faces_in_colliders;

		ObjMesh(const char* p_path, glm::vec3 p_position, glm::quat p_quaternion);
		~ObjMesh();
		
		bool raycast(glm::vec3 p_origin, glm::vec3 p_dir, glm::vec2 p_interval, RaycastHit& p_hit);

	static bool rayTriangleIntersect(glm::vec3& p_origin, glm::vec3& p_dir, glm::vec3& p_v0, glm::vec3& p_v1, glm::vec3& p_v2, glm::vec3& p_n, float& p_t);
	
};