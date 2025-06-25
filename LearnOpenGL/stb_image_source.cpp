#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

#if 0

#include "stb_image.h"
#include "camera.h"
#include "shader_s.h"
using namespace std;

#include <iostream>
#include <vector>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

// build and compile our shader zprogram
// ------------------------------------
Shader lightingShader("lighting.vert", "lighting.frag");
Shader lightCubeShader("light_cube.vert", "light_cube.frag");

unsigned char* data = stbi_load("container.jpg", &width, &height, &nrChannels, 0);

unsigned int diffuseMap = loadTexture("container2.png");
unsigned int specularMap = loadTexture("container2_specular.png");

// ImGui interface

#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"

// Setup Dear ImGui context
IMGUI_CHECKVERSION();
ImGui::CreateContext();
ImGuiIO& io = ImGui::GetIO();
io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
// Setup Platform/Render backends
ImGui_ImplGlfw_InitForOpenGL(window, true);
ImGui_ImplOpenGL3_Init();

// Start the Dear ImGui frame
ImGui_ImplOpenGL3_NewFrame();
ImGui_ImplGlfw_NewFrame();
ImGui::NewFrame();
ImGui::ShowDemoWindow();

// 6. ��Ӹ��ֿؼ�
ImGui::Text(u8"��Ⱦ��������:");
ImGui::SliderFloat("������", &mixValue, 0.0f, 1.0f);
ImGui::ColorEdit3("��Դ��ɫ", (float*)&lightColor);
ImGui::SliderFloat("��Ұ(FoV)", &fov, 15.0f, 120.0f);
ImGui::Checkbox("ʹ������", &useTexture);

ImGui::Separator();
ImGui::Text("ģ�Ϳ���:");
ImGui::SliderFloat3("λ��", &objectPosition.x, -5.0f, 5.0f);
ImGui::SliderFloat("��ת�ٶ�", &rotationSpeed, 0.0f, 5.0f);

ImGui::Separator();
ImGui::Text("��Ⱦ״̬:");
if (ImGui::Button("�߿�ģʽ"))
polygonMode = (polygonMode == GL_FILL) ? GL_LINE : GL_FILL;
ImGui::Checkbox("��ʾDemo����", &showDemoWindow);

ImGui::End();

// 7. ��ѡ����ʾImGui��ʾ����
if (showDemoWindow)
ImGui::ShowDemoWindow(&showDemoWindow);

ImGui::Render();
ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

ImGui_ImplOpenGL3_Shutdown();
ImGui_ImplGlfw_Shutdown();
ImGui::DestroyContext();


#endif