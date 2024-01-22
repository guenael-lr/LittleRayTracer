#include "Mesh.h"

Mesh::Mesh()
	: m_vbo_id(0)
	, m_ebo_id(0)
	, m_vao_id(0)
{
}

Mesh::~Mesh()
{
	if (m_vertices.size() > 0)
		m_vertices.clear();
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

Mesh* Mesh::loadFromOBJ(const char* p_path)
{
	std::ifstream file(p_path);
	if (!file.is_open())
		return NULL;

	std::vector<glm::vec3> positions;
	std::vector<glm::vec3> normals;
	std::vector<glm::vec2> texcoords;

	std::vector<glm::ivec3> vertexAttributes[3]; // one slot per vertex in the face 0/1/2 and one component per attribute (X=position, Y=texcoords, Z=normal)

	std::string line;
	while (std::getline(file, line))
	{
		if (line.empty())
			continue;
		std::vector<std::string> tokenAndValues = split(line, " \t");
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
			if (tokenAndValues.size() >= 4)
				v3.z = std::stof(tokenAndValues[3]);

			if (token.size() > 1)
			{
				switch (token[1])
				{
				case 't':	// TEXCOORD
					texcoords.push_back(v3.xy());
					break;
				case 'n':	// NORMAL
					normals.push_back(v3);
					break;
				}
			}
			else // POSITION
			{
				positions.push_back(v3);
			}
		}
		break;
		case 'f':	// FACE
		{
			if (tokenAndValues.size() < 4)
				continue;

			for (int i = 0; i < 3; i++)
			{
				std::vector<std::string> indicesStr = split(tokenAndValues[i + 1], "/");
				if (indicesStr.size() < 3)
					continue;
				glm::ivec3 ids = glm::ivec3(std::stoi(indicesStr[0]) - 1, std::stoi(indicesStr[1]) - 1, std::stoi(indicesStr[2]) - 1);
				vertexAttributes[i].push_back(ids);
			}

		}
		break;
		}
	}
	file.close();


	Mesh* m = new Mesh();
	if (!m)
		return NULL;

	// fill the arrays of the new mesh "m"

	// loop on all the vertices (3 vertices = 1 face)
	for (unsigned int i = 0; i < vertexAttributes[0].size(); i++)
	{
		for (int j = 0; j < 3; j++)
		{
			// fill the vertex descriptor
			vertexDescriptor vd;

			// position


			vd.position = positions[vertexAttributes[j][i].x];

			// texcoords
			vd.texcoords = texcoords[vertexAttributes[j][i].y] * glm::vec2(1, -1) + glm::vec2(0, 1);

			// normal
			vd.normal = normals[vertexAttributes[j][i].z];

			// FOKS lab

			// ...

			// add it to the vertex array/vector
			m->m_vertices.push_back(vd);
		}
		m->m_faces.push_back(glm::ivec3(i * 3 + 0, i * 3 + 1, i * 3 + 2));
	}

	//load the mesh in the GPU with the vao and the vbo 

	glGenBuffers(1, &m->m_vbo_id);
	glBindBuffer(GL_ARRAY_BUFFER, m->m_vbo_id);
	glBufferData(GL_ARRAY_BUFFER, m->m_vertices.size() * sizeof(vertexDescriptor), &m->m_vertices[0], GL_STATIC_DRAW);
	glBindBuffer(GL_ARRAY_BUFFER, 0);

	glGenBuffers(1, &m->m_ebo_id);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m->m_ebo_id);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, m->m_faces.size() * sizeof(faceDescriptor), &m->m_faces[0], GL_STATIC_DRAW);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

	glGenVertexArrays(1, &m->m_vao_id);
	glBindVertexArray(m->m_vao_id);

	glBindBuffer(GL_ARRAY_BUFFER, m->m_vbo_id);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m->m_ebo_id);

	glEnableClientState(GL_VERTEX_ARRAY);
	glVertexPointer(3, GL_FLOAT, sizeof(vertexDescriptor), (void*)0);
	glEnableClientState(GL_NORMAL_ARRAY);
	glNormalPointer(GL_FLOAT, sizeof(vertexDescriptor), (void*)12);
	glEnableClientState(GL_TEXTURE_COORD_ARRAY);
	glTexCoordPointer(2, GL_FLOAT, sizeof(vertexDescriptor), (void*)24);

	glBindVertexArray(0);



	return m;
}

void Mesh::draw()
{

	//static int g_progression = 0;
	//g_progression = (g_progression + 1) % (m_faces.size() + 1);

	glBindVertexArray(m_vao_id);
	//glDrawElements(GL_TRIANGLES, g_progression * 3, GL_UNSIGNED_INT, 0);
	glDrawElements(GL_TRIANGLES, m_faces.size() * 3, GL_UNSIGNED_INT, 0);
	glBindVertexArray(0);


	//glColor3f(0.0f, 0.0f, 0.0f);
	//glBegin(GL_TRIANGLES);
	//{
	//	for (unsigned int i = 0; i < m_faces.size(); i++)
	//	{
	//		faceDescriptor fd = m_faces[i];
	//		vertexDescriptor vd;
	//		for (int j = 0; j < 3; j++)
	//		{
	//			vd = m_vertices[fd[j]];

	//			glNormal3fv(&vd.normal.x);
	//			glTexCoord2fv(&vd.texcoords.x);
	//			glVertex3fv(&vd.position.x);
	//		}
	//	}
	//}
	//glEnd();

}
