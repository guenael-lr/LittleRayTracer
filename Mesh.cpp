#include "Mesh.h"

Mesh::Mesh()
{
}

Mesh::~Mesh()
{
	if (m_vertices.size() > 0)
		m_vertices.clear();
	if (m_faces.size() > 0)
		m_faces.clear();
}

//load obj file without opengl
Mesh* Mesh::loadFromOBJ(const char* p_path)
{
	Mesh* mesh = new Mesh();
	std::ifstream file(p_path);
	if (!file.is_open())
	{
		std::cout << "Error loading file " << p_path << std::endl;
		return nullptr;
	}
	std::string line;
	//read each line of the file
	while (std::getline(file, line))
	{
		//split the line into words
		std::istringstream iss(line);
		if (line == "")
			continue;
		std::vector<std::string> tokens{ std::istream_iterator<std::string>{iss}, std::istream_iterator<std::string>{} };
		//if the line is a vertex
		if (tokens[0] == "v")
		{
			//create a vertex with the 3 first values of the line
			vertexDescriptor vertex;
			vertex.position.x = std::stof(tokens[1]);
			vertex.position.y = std::stof(tokens[2]);
			vertex.position.z = std::stof(tokens[3]);
			//add the vertex to the mesh
			mesh->m_vertices.push_back(vertex);
		}
		//if line is a vt
		else if (tokens[0] == "vt")
		{
			//create a vertex with the 2 first values of the line
			vertexDescriptor vertex;
			vertex.texcoords.x = std::stof(tokens[1]);
			vertex.texcoords.y = std::stof(tokens[2]);
			//add the vertex to the mesh
			mesh->m_vertices.push_back(vertex);
		}

		//if line is a vn
		else if (tokens[0] == "vn")
		{
			//create a vertex with the 3 first values of the line
			vertexDescriptor vertex;
			vertex.normal.x = std::stof(tokens[1]);
			vertex.normal.y = std::stof(tokens[2]);
			vertex.normal.z = std::stof(tokens[3]);
			//add the vertex to the mesh
			mesh->m_vertices.push_back(vertex);
		}
		else if (tokens[0] == "f")
		{ 
			//create face from format f 1/1/1 2/2/2 3/3/3
			faceDescriptor face;
			face.v_position.x = std::stof(tokens[1].substr(0, tokens[1].find("/")));
			face.v_texcoords.x = std::stof(tokens[1].substr(tokens[1].find_last_of("/") + 1, tokens[1].size()));
			face.v_normal.x = std::stof(tokens[1].substr(tokens[1].find_last_of("/") + 1, tokens[1].size()));

			face.v_position.y = std::stof(tokens[2].substr(0, tokens[2].find("/")));
			face.v_texcoords.y = std::stof(tokens[2].substr(tokens[2].find_last_of("/") + 1, tokens[2].size()));
			face.v_normal.y = std::stof(tokens[2].substr(tokens[2].find_last_of("/") + 1, tokens[2].size()));

			face.v_position.z = std::stof(tokens[3].substr(0, tokens[3].find("/")));
			face.v_texcoords.z = std::stof(tokens[3].substr(tokens[3].find_last_of("/") + 1, tokens[3].size()));
			face.v_normal.z = std::stof(tokens[3].substr(tokens[3].find_last_of("/") + 1, tokens[3].size()));

			mesh->m_faces.push_back(face);

		}
	}


	return mesh;
}
