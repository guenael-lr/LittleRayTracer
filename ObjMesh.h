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
			
			int nb_w = 1;
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
				faces_in_colliders.push_back(std::vector<faceDescriptor>());
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
						faces_in_colliders[i].push_back(mesh->m_faces[j]);
					}
				}
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
			
			for(int i = 0; i < colliders.size(); i++) {
				//check if the raycast intersect the collider*
				RaycastHit AABBHit;
				if (colliders[i]->raycast(p_origin, p_dir, p_interval, AABBHit)) {
					float min_t = INFINITY;
					RaycastHit hit;
					for (int j = 0; j < faces_in_colliders[i].size(); ++j) {
						//get the 3 vertices of the face
						glm::vec3 v1 = mesh->v_Positions[faces_in_colliders[i][j].v_position[0]];
						glm::vec3 v2 = mesh->v_Positions[faces_in_colliders[i][j].v_position[1]];
						glm::vec3 v3 = mesh->v_Positions[faces_in_colliders[i][j].v_position[2]];
						v1 = getPosInWorld(v1);
						v2 = getPosInWorld(v2);
						v3 = getPosInWorld(v3);

						glm::vec3 N = glm::normalize(glm::cross(v2 - v1, v3 - v1));
// 
						//check if the ray intersect the face
						if (rayTriangleIntersect(p_origin, p_dir, v1, v2, v3, N, hit.t)) {
							//set the hit informations
							hit.hitPosition = p_origin + hit.t * glm::normalize(p_dir);
							hit.hitNormal = glm::normalize(hit.hitPosition - m_position);
							hit.hitCollider = this;
							hit.hitMaterial = material;
							hit.hitUV[0] = mesh->v_TexCoords[faces_in_colliders[i][j].v_texcoord[0]];
							hit.hitUV[1] = mesh->v_TexCoords[faces_in_colliders[i][j].v_texcoord[1]];
							hit.hitUV[2] = mesh->v_TexCoords[faces_in_colliders[i][j].v_texcoord[2]];
							hit.hitVertices[0] = v1;
							hit.hitVertices[1] = v2;
							hit.hitVertices[2] = v3;
							if (min_t > hit.t) {
								p_hit = hit;
								min_t = hit.t;

							}

						}
					}
					if (min_t < INFINITY) {
						return true;
					}
					return false;

				}
			}
			return false;
		};

	bool rayTriangleIntersect(glm::vec3 &p_origin, glm::vec3 &p_dir,glm::vec3 &v0, glm::vec3 &v1, glm::vec3 &v2, glm::vec3 &N, float &t) 
	{
		const float EPSILON = 0.0000001;
		glm::vec3 edge1, edge2, h, s, q;
		float a, f, u, v;
		edge1 = v1 - v0;
		edge2 = v2 - v0;
		h = glm::cross(p_dir, edge2);
		a = glm::dot(edge1,h);
		if (a > -EPSILON && a < EPSILON)
			return false;    // Le rayon est parallèle au triangle.

		f = 1.0 / a;
		s = p_origin - v0;
		u = f * glm::dot(s,h);
		if (u < 0.0 || u > 1.0)
			return false;
		q = glm::cross(s, edge1);
		v = f * glm::dot(p_dir,q);
		if (v < 0.0 || u + v > 1.0)
			return false;

		// On calcule t pour savoir ou le point d'intersection se situe sur la ligne.
		t = f * glm::dot(edge2, q);
		if (t > EPSILON && glm::dot(p_dir, N) < EPSILON) // Intersection avec le rayon
		{
			return true;
		}
		else // On a bien une intersection de droite, mais pas de rayon.
			return false;

		
	}
};