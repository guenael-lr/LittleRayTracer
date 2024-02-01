#include "PostProcessEffect.h"

// https://learnopengl.com/Advanced-Lighting/Bloom#:~:text=This%20light%20bleeding%2C%20or%20glow,scene%20a%20glow-like%20effect.
// ressource for glowing

void GlowEffect::applyPostProcess(glm::vec3* p_scenePixels, glm::vec3* p_postProcessedScenePixels, const int& p_resolutionX, const int& p_resolutionY, const int& p_numFrame) 
{
	// Cree un tableau temporaire pour stocker l'image post process
	auto tempImage = new glm::vec3[p_resolutionX * p_resolutionY];                
	memcpy(tempImage, p_scenePixels, p_resolutionX * p_resolutionY * sizeof(glm::vec3));
	
    // Apply luminosity threshold
    for (int i = 0; i < p_resolutionX * p_resolutionY; ++i)
    {
	    auto tempPixel = p_scenePixels[i] / static_cast<float>(p_numFrame);
    	 tempImage[i] = dot(tempPixel, glm::vec3(0.2126f, 0.7152f, 0.0722f)) > m_luminosityThreshold ? tempPixel : glm::vec3(0.f);
    }

    // Apply blur passes
    for (unsigned int pass = 0; pass < m_nbBlurPass; ++pass)
        applyBilinearFiltering(tempImage, p_resolutionX, p_resolutionY);
	
    // Final compositing
    for (int i = 0; i < p_resolutionX * p_resolutionY; ++i)
    {
        tempImage[i] += p_scenePixels[i] / static_cast<float>(p_numFrame);
        p_postProcessedScenePixels[i] = glm::vec3(1.0f) - exp(-tempImage[i] * m_exposure);
    }
	delete[] tempImage;
}

void GlowEffect::applyBilinearFiltering(glm::vec3* p_pixels, const int& p_resolutionX, const int& p_resolutionY)
{
    constexpr float weights[5] = {0.227027f, 0.1945946f, 0.1216216f, 0.054054f, 0.016216f};

    // Temporary buffer for the horizontal pass
    auto horizontalBuffer = new glm::vec3[p_resolutionX * p_resolutionY];

    // Horizontal pass
    for (int j = 0; j < p_resolutionY; ++j)
    {
        for (int i = 0; i < p_resolutionX; ++i)
        {
            const int currIndex = j * p_resolutionX + i;
            glm::vec3 blurredPixel = weights[0] * p_pixels[currIndex];

            for (int k = 1; k < 5; ++k)
            {
                blurredPixel += weights[k] * p_pixels[glm::clamp(currIndex + k, 0, p_resolutionX * p_resolutionY - 1)];
                blurredPixel += weights[k] * p_pixels[glm::clamp(currIndex - k, 0, p_resolutionX * p_resolutionY - 1)];
            }

            horizontalBuffer[currIndex] = blurredPixel;
        }
    }

    // Vertical pass directly onto the p_pixel buffer
    for (int i = 0; i < p_resolutionX; ++i)
    {
        for (int j = 0; j < p_resolutionY; ++j)
        {
            const int currIndex = j * p_resolutionX + i;
            glm::vec3 blurredPixel = weights[0] * horizontalBuffer[currIndex];

            for (int k = 1; k < 5; ++k)
            {
                blurredPixel += weights[k] * horizontalBuffer[glm::clamp(currIndex + k * p_resolutionX, 0, p_resolutionX * p_resolutionY - 1)];
                blurredPixel += weights[k] * horizontalBuffer[glm::clamp(currIndex - k * p_resolutionX, 0, p_resolutionX * p_resolutionY - 1)];
            }

            p_pixels[currIndex] = blurredPixel;
        }
    }

    // Clean up temporary buffer
    delete[] horizontalBuffer;
}

void GammaCorrectionEffect::applyPostProcess(glm::vec3* p_scenePixels, glm::vec3* p_postProcessedScenePixels, const int& p_resolutionX, const int& p_resolutionY, const
                                             int& p_numFrame)
{
	// Cree un tableau temporaire pour stocker l'image post process
	const auto tempImage = new glm::vec3[p_resolutionX * p_resolutionY];
	
	memcpy(tempImage, p_scenePixels, p_resolutionX * p_resolutionY * sizeof(glm::vec3));
	// apply gamma correction on accumulated pixels with r g b
	for (int i = 0; i < p_resolutionX * p_resolutionY; i++)
	{
		tempImage[i].g = powf(tempImage[i].g / static_cast<float>(p_numFrame), 1.f / m_gamma);
		tempImage[i].b = powf(tempImage[i].b / static_cast<float>(p_numFrame), 1.f / m_gamma);
		tempImage[i].r = powf(tempImage[i].r / static_cast<float>(p_numFrame), 1.f / m_gamma);
	}

	memcpy(p_postProcessedScenePixels, tempImage, p_resolutionX * p_resolutionY * sizeof(glm::vec3));
	// Libere la memoire du tableau temporaire
	delete[] tempImage;
}

void ToneMappingEffect::applyPostProcess(glm::vec3* p_scenePixels, glm::vec3* p_postProcessedScenePixels, const int& p_resolutionX, const int& p_resolutionY, const
                                         int& p_numFrame)
{
	// Cree un tableau temporaire pour stocker l'image post process
	const auto tempImage = new glm::vec3[p_resolutionX * p_resolutionY];
	memcpy(tempImage, p_scenePixels, p_resolutionX * p_resolutionY * sizeof(glm::vec3));

	// Calculate average luminance
	float averageLuminance = 0.0f;
	for (int i = 0; i < p_resolutionX * p_resolutionY; ++i)
	{
		const float luminance = (0.2126f * tempImage[i].r + 0.7152f * tempImage[i].g + 0.0722f * tempImage[i].b) / static_cast<float>(p_numFrame);
		averageLuminance += logf(0.00001f + luminance);
	}
	averageLuminance = expf(averageLuminance / (p_resolutionX * p_resolutionY));

	// Apply Reinhard tone mapping
	for (int i = 0; i < p_resolutionX * p_resolutionY; ++i)
	{
		p_postProcessedScenePixels[i] = tempImage[i] / static_cast<float>(p_numFrame) * (m_key / (1.0f + averageLuminance));
	}

	// Libere la memoire du tableau temporaire
	delete[] tempImage;
}
