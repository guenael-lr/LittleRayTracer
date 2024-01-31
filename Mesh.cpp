#include "Mesh.h"

Mesh::Mesh()
{
}

Mesh::~Mesh()
{
	if (m_faces.size() > 0)
		m_faces.clear();
}

std::vector<std::string> split(std::string p_text, const char* p_delims)
{
	//https://stackoverflow.com/questions/236129/how-do-i-iterate-over-the-words-of-a-string
	std::vector<std::string> tokens;
	std::size_t start = p_text.find_first_not_of(p_delims), end = 0;

	while ((end = p_text.find_first_of(p_delims, start)) != std::string::npos)
	{
		tokens.push_back(p_text.substr(start, end - start));
		start = p_text.find_first_not_of(p_delims, end);
	}
	if (start != std::string::npos)
		tokens.push_back(p_text.substr(start));

	return tokens;
}

//load obj file without opengl
Mesh* Mesh::loadFromOBJ(const char* p_path)
{
	std::ifstream file(p_path);
	if (!file.is_open())
		return NULL;

	Mesh* m = new Mesh();
	if (!m)
		return NULL;

	std::string line;
	while (std::getline(file, line))
	{
		if (line.empty())
			continue;
		std::vector<std::string> tokenAndValues = split(line, " ");
		if (tokenAndValues.size() == 0)
			continue;


		std::string token = tokenAndValues[0];

		switch (token[0])
		{
		case'v':
		{
			if (token.size() > 2 || tokenAndValues.size() < 3)
				continue;

			glm::vec3 v3 = glm::vec3(std::stof(tokenAndValues[1]), std::stof(tokenAndValues[2]), 0.0f);
				if (tokenAndValues.size() > 3)
					v3.z = std::stof(tokenAndValues[3]);

			if (token.size() > 1)
			{
				switch (token[1])
				{
				case 't':	// TEXCOORD
					m->m_VecTexCoords.push_back(v3); 
					break;
				case 'n':	// NORMAL
					m->m_VecNormals.push_back(v3); 
					break;
				}
			}
			else // POSITION
			{
				m->m_VecPositions.push_back(v3);
			}
		}
		break;
		case 'f':	// FACE
		{
			if (tokenAndValues.size() < 4)
				continue;

			faceDescriptor f;

			for (int i = 0; i < 3; i++)
			{
				std::vector<std::string> indicesStr = split(tokenAndValues[i + 1], "/");
				f.m_normal[i] = std::stoi(indicesStr[2]) - 1;
				f.m_texcoord[i] = std::stoi(indicesStr[1]) - 1;
				f.m_position[i] = std::stoi(indicesStr[0]) - 1;
			}

			m->m_faces.push_back(f);
				
		}
		break;
		}
	}
	file.close();

	return m;
}
