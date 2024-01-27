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

		//table of three vertices for each collider
		std::vector<std::vector<glm::vec3>> vertices;
		
		//vector of colliders
		std::vector<AABB*> colliders;

		ObjMesh(const char* p_path, glm::vec3 p_position, glm::quat p_quaternion) :Object(p_position, p_quaternion)
		{
			mesh = Mesh::loadFromOBJ(p_path);
			
			//get the min in each axis 
			glm::vec3 min = glm::vec3(FLT_MAX, FLT_MAX, FLT_MAX);
			glm::vec3 max = glm::vec3(-FLT_MAX, -FLT_MAX, -FLT_MAX);

			for (int i = 0; i < mesh->v_Positions.size(); i++) {
				min.x = glm::min(min.x, mesh->v_Positions[i].x);
				min.y = glm::min(min.y, mesh->v_Positions[i].y);
				min.z = glm::min(min.z, mesh->v_Positions[i].z);

				max.x = glm::max(max.x, mesh->v_Positions[i].x);
				max.y = glm::max(max.y, mesh->v_Positions[i].y);
				max.z = glm::max(max.z, mesh->v_Positions[i].z);
			}

			//set the min and max to posinworld
		
			min = getPosInWorld(min);
			max = getPosInWorld(max);
			
			int nb_w = 2;
			int nb_h = 2;

			float w_x = (max.x - min.x) / nb_w;
			float w_y = (max.y - min.y) / nb_h;

			//create colliders
			for (int i = 0; i < nb_w; i++) {
				for (int j = 0; j < nb_h; j++) {
					glm::vec3 min_collider = glm::vec3(min.x + i * w_x, min.y + j * w_y, min.z);
					glm::vec3 max_collider = glm::vec3(min.x + (i + 1) * w_x, min.y + (j + 1) * w_y, max.z);
					colliders.push_back(new AABB(min_collider, max_collider));
				}
			}

			//table of faces, add a face to a collider if a vertice is in the collider
			for (int i = 0; i < colliders.size(); i++) {
				std::vector<glm::vec3> vertices_collider;
				for (int j = 0; j < mesh->m_faces.size(); j++) {
					//get the 3 vertices of the face
					glm::vec3 v1 = mesh->v_Positions[mesh->m_faces[j].v_position[0]];
					glm::vec3 v2 = mesh->v_Positions[mesh->m_faces[j].v_position[1]];
					glm::vec3 v3 = mesh->v_Positions[mesh->m_faces[j].v_position[2]];

					//set vertices in world
					v1 = getPosInWorld(v1);
					v2 = getPosInWorld(v2);
					v3 = getPosInWorld(v3);

					//check if a vertice is in the collider
					if (colliders[i]->isPointInside(v1) || colliders[i]->isPointInside(v2) || colliders[i]->isPointInside(v3)) {
						vertices_collider.push_back(v1);
						vertices_collider.push_back(v2);
						vertices_collider.push_back(v3);
					}
				}
				vertices.push_back(vertices_collider);
			}

			
		}
		~ObjMesh() {
			//clear all 
			delete mesh;
			//delete collider
			for (int i = 0; i < colliders.size(); i++) {
				delete colliders[i];
			}
		};

		bool raycast(glm::vec3 p_origin, glm::vec3 p_dir, glm::vec2 p_interval, RaycastHit& p_hit) {
			
			//raycast all faces
			for (int i = 0; i < mesh->m_faces.size(); ++i) {
				//get the 3 vertices of the face
				glm::vec3 v1 = mesh->v_Positions[mesh->m_faces[i].v_position[0]];
				glm::vec3 v2 = mesh->v_Positions[mesh->m_faces[i].v_position[1]];
				glm::vec3 v3 = mesh->v_Positions[mesh->m_faces[i].v_position[2]];

				//set vertices in world
				v1 = getPosInWorld(v1);
				v2 = getPosInWorld(v2);
				v3 = getPosInWorld(v3);

				//get normals
				glm::vec3 n1 = mesh->v_Normals[mesh->m_faces[i].v_normal[0]];
				glm::vec3 n2 = mesh->v_Normals[mesh->m_faces[i].v_normal[1]];
				glm::vec3 n3 = mesh->v_Normals[mesh->m_faces[i].v_normal[2]];

				//get normal from cross vector
				//glm::vec3 N = glm::normalize(glm::cross(n2 - n1, n3 - n1));
				glm::vec3 N = glm::normalize(glm::cross(v2 - v1, v3 - v1));
				//check if the ray intersect the face
				if (rayTriangleIntersect(p_origin, p_dir, v1, v2, v3, N, p_hit.t)) {
					//set the hit informations
					p_hit.hitPosition = p_origin + p_hit.t * glm::normalize(p_dir);
					p_hit.hitNormal = glm::normalize(p_hit.hitPosition - m_position);
					p_hit.hitCollider = this;
					p_hit.hitMaterial = material;
					p_hit.hitUV[0] = mesh->v_TexCoords[mesh->m_faces[i].v_texcoord[0]];
					p_hit.hitUV[1] = mesh->v_TexCoords[mesh->m_faces[i].v_texcoord[1]];
					p_hit.hitUV[2] = mesh->v_TexCoords[mesh->m_faces[i].v_texcoord[2]];
					return true;
					
				}
			}


			return false;


		};

	bool rayTriangleIntersect(glm::vec3 &p_origin, glm::vec3 &p_dir,glm::vec3 &v0, glm::vec3 &v1, glm::vec3 &v2, glm::vec3 &N, float &t) 
	{
		//check if the ray is parallel to the face
		float denom = glm::dot(N, p_dir);
		if (denom > 1e-6) {
			//get the distance between the origin and the face
			glm::vec3 v0l0 = v0 - p_origin;
			t = glm::dot(v0l0, N) / denom;
			//check if the face is behind the ray
			if (t >= 0) {
				//get the intersection point
				glm::vec3 P = p_origin + t * p_dir;
				//check if the intersection point is in the triangle
				if (glm::dot(glm::cross(v1 - v0, P - v0), N) >= 0 && glm::dot(glm::cross(v2 - v1, P - v1), N) >= 0 && glm::dot(glm::cross(v0 - v2, P - v2), N) >= 0) {
					return true;
				}
			}
		}
		return false;

		
	}
};