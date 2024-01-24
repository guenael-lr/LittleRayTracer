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
		std::vector<std::vector<vertexDescriptor>> vertices;
		
		//vector of colliders
		std::vector<AABB*> colliders;

		ObjMesh(const char* p_path, glm::vec3 p_position, glm::quat p_quaternion) :Object(p_position, p_quaternion)
		{
			mesh = Mesh::loadFromOBJ(p_path);
			
			//get the min in each axis 
			glm::vec3 min = glm::vec3(FLT_MAX, FLT_MAX, FLT_MAX);
			glm::vec3 max = glm::vec3(-FLT_MAX, -FLT_MAX, -FLT_MAX);

			for (int i = 0; i < mesh->m_vertices.size(); i++) {
				min.x = glm::min(min.x, mesh->m_vertices[i].position.x);
				min.y = glm::min(min.y, mesh->m_vertices[i].position.y);
				min.z = glm::min(min.z, mesh->m_vertices[i].position.z);

				max.x = glm::max(max.x, mesh->m_vertices[i].position.x);
				max.y = glm::max(max.y, mesh->m_vertices[i].position.y);
				max.z = glm::max(max.z, mesh->m_vertices[i].position.z);
			}

			//set the min and max to posinworld
		
			min = getPosInWorld(min);
			max = getPosInWorld(max);
			
			int nb_w = 1;
			int nb_h = 1;

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
				std::vector<vertexDescriptor> vertices_collider;
				for (int j = 0; j < mesh->m_faces.size(); j++) {
					//get the 3 vertices of the face
					vertexDescriptor v1 = mesh->m_vertices[mesh->m_faces[j].v_position.x - 1];
					vertexDescriptor v2 = mesh->m_vertices[mesh->m_faces[j].v_position.y - 1];
					vertexDescriptor v3 = mesh->m_vertices[mesh->m_faces[j].v_position.z - 1];

					//set vertices in world
					v1.position = getPosInWorld(v1.position);
					v2.position = getPosInWorld(v2.position);
					v3.position = getPosInWorld(v3.position);

					//check if a vertice is in the collider
					if (colliders[i]->isInside(v1.position) || colliders[i]->isInside(v2.position) || colliders[i]->isInside(v3.position)) {
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
			
			//raycast to the colliders
			float min_t = FLT_MAX;
			
			int closest = -1;
			//find the closest collider
			for (int i = 0; i < colliders.size(); i++) {
				RaycastHit hit;
				if (colliders[i]->raycast(p_origin, p_dir, p_interval, hit)) {
					if (hit.t < min_t) {
						min_t = hit.t;
						closest = i;
					}
				} 
			}

			//if no collider is hit, return false
			if (closest < 0)
				return false;

			for (int i = 0; i < vertices[closest].size(); i++) {
				//get the 3 vertices of the face
				glm::vec3 v1 = vertices[closest][i].position;
				glm::vec3 v2 = vertices[closest][i + 1].position;
				glm::vec3 v3 = vertices[closest][i + 2].position;

				//get normal of face
				glm::vec3 normal = glm::normalize(glm::cross(v2 - v1, v3 - v1));

				//check if the ray and the face are parallel
				if (glm::dot(normal, p_dir) == 0) {
					continue;
				}

				//get the distance between the ray origin and the face
				float d = glm::dot(normal, v1);

				//compute t
				float t = (d - glm::dot(normal, p_origin)) / glm::dot(normal, p_dir);
				if (t < p_interval.x || t > p_interval.y) {
					continue;
				}

				//compute the intersection point
				glm::vec3 intersection = p_origin + t * p_dir;

				//check if the intersection point is inside the face


				glm::vec3 edge1 = v2 - v1;
				glm::vec3 edge2 = v3 - v2;

				glm::vec3 C = glm::cross(edge1, intersection - v1);
				glm::vec3 C2 = glm::cross(edge2, intersection - v2);
				glm::vec3 C3 = glm::cross(v1 - v3, intersection - v3);

				if (glm::dot(normal, C) >= 0 && glm::dot(normal, C2) >= 0 && glm::dot(normal, C3) >= 0) {
					p_hit.t = t;
					p_hit.hitPosition = intersection;
					p_hit.hitNormal = normal;
					p_hit.hitCollider = this;
					p_hit.hitMaterial = material;
					return true;
				}

			}


			return false;


		};
};