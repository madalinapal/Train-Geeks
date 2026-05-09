#include "lab_m1/Tema2/obj2D.h"

#include <vector>

Mesh* obj2D::CreateRectangle(const std::string& name,
                            glm::vec3 leftBottomCorner,
                            float lengthX,
                            float lengthY,
                            glm::vec3 color,
                            bool fill)
{
    glm::vec3 corner = leftBottomCorner;

    std::vector<VertexFormat> vertices =
    {
        VertexFormat(corner,                          color),
        VertexFormat(corner + glm::vec3(lengthX, 0, 0),   color),
        VertexFormat(corner + glm::vec3(lengthX, lengthY, 0), color),
        VertexFormat(corner + glm::vec3(0, lengthY, 0),  color)
    };

    Mesh* rect = new Mesh(name);

    std::vector<unsigned int> indices;

    if (fill) {
        indices = { 0, 1, 2, 0, 2, 3 };
        rect->InitFromData(vertices, indices);
    } else {
        indices = { 0, 1, 2, 3 };
        rect->SetDrawMode(GL_LINE_LOOP);
        rect->InitFromData(vertices, indices);
    }

    return rect;
}
