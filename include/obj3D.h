#pragma once

#include <string>

#include "core/gpu/mesh.h"
#include "utils/glm_utils.h"

namespace obj3D
{

    Mesh* CreateCube(const std::string &name, glm::vec3 center, float length, glm::vec3 color);
    Mesh* CreateSphere(const std::string &name, glm::vec3 center, float radius, glm::vec3 color, int sectors = 36, int stacks = 18);
    Mesh* CreateCylinder(const std::string& name, glm::vec3 center,
                         float radius, float height, glm::vec3 color,
                         int sectors = 36);
    Mesh* CreateBridge(const std::string& name,
                       glm::vec3 center,
                       float length,          // dimensiunea „cubului” de bază
                       glm::vec3 darkColor,   // negru (șina)
                       glm::vec3 lightColor,  // alb (spațiile)
                       int layers = 5);
    Mesh* CreateTunnel(const std::string& name,
                       glm::vec3 center,
                       float length,
                       glm::vec3 outerColor,   // negru
                       glm::vec3 innerColor,   // alb
                       float innerScale = 0.5f);
    Mesh* CreateTrain(const std::string& name, glm::vec3 center);
    Mesh* CreateDrezina(const std::string& name, glm::vec3 center);

    Mesh* CreateDrezinaBody(const std::string& name, glm::vec3 center);
    Mesh* CreateDrezinaBar (const std::string& name, glm::vec3 center);
    Mesh* CreatePyramid(const std::string& name, glm::vec3 center,
                        float baseLength, float height, glm::vec3 color);
}