#pragma once
#include "Utils.h"

class PostProcessEffect
{
public:
    PostProcessEffect() = default;
    virtual ~PostProcessEffect() = default;
    virtual void applyPostProcess(glm::vec3* p_scenePixels, int p_resolutionX, int p_resolutionY) = 0;
};

class GlowEffect final : public PostProcessEffect
{
public:
    explicit GlowEffect(const float p_sigmaSpace = 1.0f, const float p_sigmaColor = 0.1f) :
        m_sigmaSpace(p_sigmaSpace), m_sigmaColor(p_sigmaColor) { }
    void applyPostProcess(glm::vec3* p_scenePixels, int p_resolutionX, int p_resolutionY) override;

private:
    void bilateralFilter(glm::vec3* p_input, glm::vec3* p_output, int p_width, int p_height);
    void spatialFilter(glm::vec3* p_input, glm::vec3* p_output, int p_width, int p_height, int p_radius);
    void colorFilter(glm::vec3* p_input, glm::vec3* p_output, int p_width, int p_height, int p_radius);

    float m_sigmaSpace; //Correspond to the glow pixel stretching
    float m_sigmaColor; //Correspond to the color smoothing on glow
};