#pragma once
#include "../shader_s.h"
#include "../camera.h"

#include "iostream"
using namespace std;

// 在头文件中（例如globals.h）声明：
extern float G_vertices[];
extern glm::vec3 G_cubePositions[];
extern glm::vec3 G_pointLightPositions[];

class LightBase
{
public:
    LightBase(Shader& InModelShader, Shader& InLightShader,Camera& InCamera):
    model_shader_(InModelShader),
    light_shader_(InLightShader),
    camera_(InCamera)
    {
        if (!bEnableLighting)
            return;
        SetUp();
    }

    void EnableLighting(bool InEnableLighting)
    {
        bEnableLighting = InEnableLighting;
    }

protected:
    void SetUp();

protected:
    unsigned int VBO, cubeVAO;
    unsigned int lightCubeVAO;

    Shader& model_shader_;
    Shader& light_shader_;
    Camera& camera_;

    bool bEnableLighting = true;
};

class DirectionalLight : public LightBase
{
public:
    DirectionalLight(Shader& InModelShader, Shader& InLightShader,Camera& InCamera)
    : LightBase(InModelShader,InLightShader, InCamera)
    {
        if (!bEnableLighting)
            return;
        
        model_shader_.use();
        
        model_shader_.setVec3("viewPos", camera_.Position);
        model_shader_.setFloat("material.shininess", 32.0f);

        // directional light
        model_shader_.setVec3("dirLight.direction", -0.2f, -1.0f, -0.3f);
        model_shader_.setVec3("dirLight.ambient", 0.05f, 0.05f, 0.05f);
        model_shader_.setVec3("dirLight.diffuse", 0.4f, 0.4f, 0.4f);
        model_shader_.setVec3("dirLight.specular", 0.5f, 0.5f, 0.5f);
    }

    void Draw(const glm::mat4& projection)
    {
        if (!bEnableLighting)
            return;
        
        glBindVertexArray(lightCubeVAO);
        // also draw the lamp object(s)
        light_shader_.use();

        glm::mat4 view = camera_.GetViewMatrix();
        light_shader_.setMat4("projection", projection);
        light_shader_.setMat4("view", view);

        glm::mat4 model = glm::mat4(1.0f);
        model = glm::translate(model, glm::vec3(2.f, 2.f, 2.f)); // translate it down so it's at the center of the scene
        model = glm::scale(model, glm::vec3(0.2f)); // Make it a smaller cube
        light_shader_.setMat4("model", model);

        glDrawArrays(GL_TRIANGLES, 0, 36);
        glBindVertexArray(0);
    }
};

class PointLight : public LightBase
{
public:
    PointLight(Shader& InModelShader, Shader& InLightShader,Camera& InCamera ,int InPointNum = 4)
    : LightBase(InModelShader,InLightShader, InCamera)
    {
        if (!bEnableLighting)
            return;
        
        model_shader_.use();

        for (int i = 0; i < InPointNum; i++)
        {
            // pointLights[i]
            const string& pointLightName = "pointLights[" + to_string(i) + "]";

            model_shader_.setVec3(pointLightName + ".position", G_pointLightPositions[0]);
            model_shader_.setVec3(pointLightName + ".ambient", 0.05f, 0.05f, 0.05f);
            model_shader_.setVec3(pointLightName + ".diffuse", 0.8f, 0.8f, 0.8f);
            model_shader_.setVec3(pointLightName + ".specular", 1.0f, 1.0f, 1.0f);
            model_shader_.setFloat(pointLightName + ".constant", 1.0f);
            model_shader_.setFloat(pointLightName + ".linear", 0.09f);
            model_shader_.setFloat(pointLightName + ".quadratic", 0.032f);
        }
    }

    void Draw(const glm::mat4& projection)
    {
        if (!bEnableLighting)
            return;
        
        glBindVertexArray(lightCubeVAO);
        light_shader_.use();

        glm::mat4 view = camera_.GetViewMatrix();
        light_shader_.setMat4("projection", projection);
        light_shader_.setMat4("view", view);
        
        // we now draw as many light bulbs as we have point lights.
        for (unsigned int i = 0; i < 4; i++)
        {
            glm::mat4 model = glm::mat4(1.0f);
            model = glm::translate(model, G_pointLightPositions[i]);
            model = glm::scale(model, glm::vec3(0.2f)); // Make it a smaller cube
            light_shader_.setMat4("model", model);
            glDrawArrays(GL_TRIANGLES, 0, 36);
        }
    }
};

class SpotLight : public LightBase
{
public:
    SpotLight(Shader& InModelShader, Shader& InLightShader,Camera& InCamera)
   : LightBase(InModelShader,InLightShader, InCamera)
    {
        if (!bEnableLighting)
            return;
        
        model_shader_.use();

        // pointLights[i]
        const string& pointLightName = "spotLight";
        model_shader_.setVec3(pointLightName + ".position", camera_.Position);
        model_shader_.setVec3(pointLightName + ".direction", camera_.Front);
        model_shader_.setVec3(pointLightName + ".ambient", 0.05f, 0.05f, 0.05f);
        model_shader_.setVec3(pointLightName + ".diffuse", 0.8f, 0.8f, 0.8f);
        model_shader_.setVec3(pointLightName + ".specular", 1.0f, 1.0f, 1.0f);
        model_shader_.setFloat(pointLightName + ".cutOff", glm::cos(glm::radians(12.5f)));
        model_shader_.setFloat(pointLightName + ".outerCutOff", glm::cos(glm::radians(17.5f)));
    }

    void Draw(const glm::mat4& projection)
    {
        if (!bEnableLighting)
            return;

        model_shader_.use();
        const string& pointLightName = "spotLight";
        model_shader_.setVec3(pointLightName + ".position", camera_.Position);
        model_shader_.setVec3(pointLightName + ".direction", camera_.Front);
    }
};
