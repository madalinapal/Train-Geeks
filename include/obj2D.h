#pragma once

#include <string>

#include "core/gpu/mesh.h"
#include "utils/glm_utils.h"


namespace obj2D
{

    Mesh* CreateSquare(const std::string &name, glm::vec3 leftBottomCorner, float length, glm::vec3 color, bool fill = false);
    Mesh* CreateSemiEllipse(const std::string &name, glm::vec3 center, float radiusX, float radiusY, glm::vec3 color, int segments = 100);
    Mesh* CreateSemiCircle(const std::string &name, glm::vec3 center, float radius, glm::vec3 color, int segments = 100);
    Mesh* CreateRectangle(const std::string &name, glm::vec3 leftBottomCorner, float lengthX, float lengthY, glm::vec3 color, bool fill = false);
    Mesh* CreateTriangle(const std::string &name, glm::vec3 corner, float base, float height, glm::vec3 color, bool fill = false);
}