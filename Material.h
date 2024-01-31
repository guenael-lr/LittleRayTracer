#pragma once
#include "Utils.h"
#include "Texture.h"

class Material
{
public:
	glm::vec3 emissive;
	glm::vec3 color;
	float roughness;
	float metallic;
	Texture* texture;

	void setTexture(const char* p_path) {
		texture = new Texture();
		texture->loadFromFile(p_path);
	}

	glm::vec3  getColorFromTexture(glm::vec3 &point, glm::vec3 vertex[3], glm::vec3 uv[3]) {
		return texture->getColor(point, vertex, uv);
	};
};

