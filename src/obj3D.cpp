#include "obj3D.h"

#include <vector>

#include "core/engine.h"
#include "utils/gl_utils.h"

Mesh* obj3D::CreateCube(const std::string &name, glm::vec3 center, float length, glm::vec3 color)
{
    glm::vec3 halfLength = glm::vec3(length / 2);

    std::vector<VertexFormat> vertices =
    {
        // fata front face
        VertexFormat(center + glm::vec3(-halfLength.x, -halfLength.y,  halfLength.z), color), // 0
        VertexFormat(center + glm::vec3( halfLength.x, -halfLength.y,  halfLength.z), color), // 1
        VertexFormat(center + glm::vec3( halfLength.x,  halfLength.y,  halfLength.z), color), // 2
        VertexFormat(center + glm::vec3(-halfLength.x,  halfLength.y,  halfLength.z), color), // 3
        // spate back face
        VertexFormat(center + glm::vec3(-halfLength.x, -halfLength.y, -halfLength.z), color), // 4
        VertexFormat(center + glm::vec3( halfLength.x, -halfLength.y, -halfLength.z), color), // 5
        VertexFormat(center + glm::vec3( halfLength.x,  halfLength.y, -halfLength.z), color), // 6
        VertexFormat(center + glm::vec3(-halfLength.x,  halfLength.y, -halfLength.z), color)  // 7
    };

    std::vector<unsigned int> indices =
    {
        // front face
        0, 1, 2,
        2, 3, 0,
        // right face
        1, 5, 6,
        6, 2, 1,
        // back face
        5, 4, 7,
        7, 6, 5,
        // left face
        4, 0, 3,
        3, 7, 4,
        // top face
        3, 2, 6,
        6, 7, 3,
        // bottom face
        4, 5, 1,
        1, 0, 4
    };

    Mesh* cube = new Mesh(name);
    cube->InitFromData(vertices, indices);
    return cube;
}

Mesh* obj3D::CreateCylinder(const std::string& name, glm::vec3 center,
                            float radius, float height, glm::vec3 color,
                            int sectors)
{
    std::vector<VertexFormat> vertices;
    std::vector<unsigned int> indices;

    float halfH = height / 2.0f;
    float sectorStep = 2.0f * glm::pi<float>() / sectors;

    // inelul lateral (cilindru de-a lungul axei Z)
    for (int i = 0; i <= sectors; i++) {
        float angle = i * sectorStep;
        float x = center.x + radius * cos(angle);
        float y = center.y + radius * sin(angle);

        // top ring
        vertices.push_back(VertexFormat(glm::vec3(x, y, center.z + halfH), color));
        // bottom ring
        vertices.push_back(VertexFormat(glm::vec3(x, y, center.z - halfH), color));
    }

    // indecsii pentru suprafata laterala
    for (int i = 0; i < sectors; i++) {
        unsigned int top1 = 2 * i;
        unsigned int bot1 = 2 * i + 1;
        unsigned int top2 = 2 * (i + 1);
        unsigned int bot2 = 2 * (i + 1) + 1;

        // primul triunghi
        indices.push_back(bot1);
        indices.push_back(top1);
        indices.push_back(top2);

        // al doilea triunghi
        indices.push_back(bot1);
        indices.push_back(top2);
        indices.push_back(bot2);
    }

    // centrele capetelor
    unsigned int centerTopIndex = (unsigned int)vertices.size();
    vertices.push_back(VertexFormat(glm::vec3(center.x, center.y, center.z + halfH), color));
    unsigned int centerBotIndex = (unsigned int)vertices.size();
    vertices.push_back(VertexFormat(glm::vec3(center.x, center.y, center.z - halfH), color));

    // capetele (discurile)
    for (int i = 0; i < sectors; i++) {
        unsigned int top1 = 2 * i;
        unsigned int top2 = 2 * (i + 1);

        unsigned int bot1 = 2 * i + 1;
        unsigned int bot2 = 2 * (i + 1) + 1;

        // top disc
        indices.push_back(centerTopIndex);
        indices.push_back(top2);
        indices.push_back(top1);

        // bottom disc
        indices.push_back(centerBotIndex);
        indices.push_back(bot1);
        indices.push_back(bot2);
    }

    Mesh* cylinder = new Mesh(name);
    cylinder->InitFromData(vertices, indices);
    return cylinder;
}

Mesh* obj3D::CreateSphere(const std::string &name, glm::vec3 center, float radius, glm::vec3 color, int sectors, int stacks)
{
    std::vector<VertexFormat> vertices;
    std::vector<unsigned int> indices;

    float sectorStep = 2.0f * glm::pi<float>() / sectors;
    float stackStep = glm::pi<float>() / stacks;

    for (int i = 0; i <= stacks; ++i) {
        float stackAngle = glm::pi<float>() / 2 - i * stackStep; // de la pi/2 la -pi/2
        float xy = radius * cosf(stackAngle);                   // raza pe planul XY
        float z = radius * sinf(stackAngle);                    // coordonata Z

        for (int j = 0; j <= sectors; ++j) {
            float sectorAngle = j * sectorStep;                 // de la 0 la 2pi

            float x = xy * cosf(sectorAngle);                  // coordonata X
            float y = xy * sinf(sectorAngle);                  // coordonata Y

            vertices.push_back(VertexFormat(glm::vec3(x + center.x, y + center.y, z + center.z), color));
        }
    }

    for (int i = 0; i < stacks; ++i) {
        int k1 = i * (sectors + 1);     // inceputul stivei curente
        int k2 = k1 + sectors + 1;      // inceputul stivei urmatoare

        for (int j = 0; j < sectors; ++j, ++k1, ++k2) {
            if (i != 0) {
                indices.push_back(k1);
                indices.push_back(k2);
                indices.push_back(k1 + 1);
            }

            if (i != (stacks - 1)) {
                indices.push_back(k1 + 1);
                indices.push_back(k2);
                indices.push_back(k2 + 1);
            }
        }
    }

    Mesh* sphere = new Mesh(name);
    sphere->InitFromData(vertices, indices);
    return sphere;
}

Mesh* obj3D::CreateBridge(const std::string& name, glm::vec3 center, float length, glm::vec3 darkColor, glm::vec3 lightColor,int bands)
{
    std::vector<VertexFormat> vertices;
    std::vector<unsigned int> indices;

    float halfX = length / 2.0f;
    float halfY = length / 2.0f;
    float halfZ = length / 2.0f;

    // impartim PODUL in "bands" benzi DE-A LUNGUL lui OZ (latimea podului)
    float bandWidth = (2.0f * halfZ) / bands;

    for (int b = 0; b < bands; b++)
    {
        float z0 = center.z - halfZ + b * bandWidth;
        float z1 = z0 + bandWidth;

        glm::vec3 col = (b % 2 == 0) ? darkColor : lightColor; // B W B W B

        int base = (int)vertices.size();

        glm::vec3 v0(center.x - halfX, center.y - halfY, z0);
        glm::vec3 v1(center.x + halfX, center.y - halfY, z0);
        glm::vec3 v2(center.x + halfX, center.y - halfY, z1);
        glm::vec3 v3(center.x - halfX, center.y - halfY, z1);

        glm::vec3 v4(center.x - halfX, center.y + halfY, z0);
        glm::vec3 v5(center.x + halfX, center.y + halfY, z0);
        glm::vec3 v6(center.x + halfX, center.y + halfY, z1);
        glm::vec3 v7(center.x - halfX, center.y + halfY, z1);

        vertices.emplace_back(v0, col);
        vertices.emplace_back(v1, col);
        vertices.emplace_back(v2, col);
        vertices.emplace_back(v3, col);
        vertices.emplace_back(v4, col);
        vertices.emplace_back(v5, col);
        vertices.emplace_back(v6, col);
        vertices.emplace_back(v7, col);

        // jos
        indices.push_back(base + 0); indices.push_back(base + 1); indices.push_back(base + 2);
        indices.push_back(base + 0); indices.push_back(base + 2); indices.push_back(base + 3);

        // sus  
        indices.push_back(base + 4); indices.push_back(base + 6); indices.push_back(base + 5);
        indices.push_back(base + 4); indices.push_back(base + 7); indices.push_back(base + 6);

        // fata
        indices.push_back(base + 0); indices.push_back(base + 4); indices.push_back(base + 5);
        indices.push_back(base + 0); indices.push_back(base + 5); indices.push_back(base + 1);

        // spate
        indices.push_back(base + 2); indices.push_back(base + 6); indices.push_back(base + 7);
        indices.push_back(base + 2); indices.push_back(base + 7); indices.push_back(base + 3);

        // stanga
        indices.push_back(base + 0); indices.push_back(base + 3); indices.push_back(base + 7);
        indices.push_back(base + 0); indices.push_back(base + 7); indices.push_back(base + 4);

        // dreapta
        indices.push_back(base + 1); indices.push_back(base + 5); indices.push_back(base + 6);
        indices.push_back(base + 1); indices.push_back(base + 6); indices.push_back(base + 2);
    }

    Mesh* bridge = new Mesh(name);
    bridge->InitFromData(vertices, indices);
    return bridge;
}

Mesh* obj3D::CreateTunnel(const std::string& name,
                          glm::vec3 center,
                          float length,
                          glm::vec3 outerColor,
                          glm::vec3 innerColor,
                          float innerScale)   
{
    std::vector<VertexFormat> vertices;
    std::vector<unsigned int> indices;

    float halfX = length / 2.0f;
    float halfY = length / 2.0f;
    float halfZ = length / 2.0f;

    // vrem 3 benzi pe OZ: B (margine sus) - W (in centru) - B (margine jos)
    int bands = 3;
    float bandWidth = (2.0f * halfZ) / bands;

    for (int b = 0; b < bands; b++)
    {
        float z0 = center.z - halfZ + b * bandWidth;
        float z1 = z0 + bandWidth;

        // banda din mijloc = alb, celelalte = negru
        glm::vec3 col = (b == 1) ? innerColor : outerColor;

        int base = (int)vertices.size();

        glm::vec3 v0(center.x - halfX, center.y - halfY, z0);
        glm::vec3 v1(center.x + halfX, center.y - halfY, z0);
        glm::vec3 v2(center.x + halfX, center.y - halfY, z1);
        glm::vec3 v3(center.x - halfX, center.y - halfY, z1);

        glm::vec3 v4(center.x - halfX, center.y + halfY, z0);
        glm::vec3 v5(center.x + halfX, center.y + halfY, z0);
        glm::vec3 v6(center.x + halfX, center.y + halfY, z1);
        glm::vec3 v7(center.x - halfX, center.y + halfY, z1);

        vertices.emplace_back(v0, col);
        vertices.emplace_back(v1, col);
        vertices.emplace_back(v2, col);
        vertices.emplace_back(v3, col);
        vertices.emplace_back(v4, col);
        vertices.emplace_back(v5, col);
        vertices.emplace_back(v6, col);
        vertices.emplace_back(v7, col);

        // jos
        indices.push_back(base + 0); indices.push_back(base + 1); indices.push_back(base + 2);
        indices.push_back(base + 0); indices.push_back(base + 2); indices.push_back(base + 3);

        // sus
        indices.push_back(base + 4); indices.push_back(base + 6); indices.push_back(base + 5);
        indices.push_back(base + 4); indices.push_back(base + 7); indices.push_back(base + 6);

        // fata
        indices.push_back(base + 0); indices.push_back(base + 4); indices.push_back(base + 5);
        indices.push_back(base + 0); indices.push_back(base + 5); indices.push_back(base + 1);

        // spate
        indices.push_back(base + 2); indices.push_back(base + 6); indices.push_back(base + 7);
        indices.push_back(base + 2); indices.push_back(base + 7); indices.push_back(base + 3);

        // stanga
        indices.push_back(base + 0); indices.push_back(base + 3); indices.push_back(base + 7);
        indices.push_back(base + 0); indices.push_back(base + 7); indices.push_back(base + 4);

        // dreapta
        indices.push_back(base + 1); indices.push_back(base + 5); indices.push_back(base + 6);
        indices.push_back(base + 1); indices.push_back(base + 6); indices.push_back(base + 2);
    }

    Mesh* tunnel = new Mesh(name);
    tunnel->InitFromData(vertices, indices);
    return tunnel;
}

// Helper: adauga un cub (prisma) cu dimensiuni diferite pe axe
static void AppendBox(std::vector<VertexFormat>& vertices,
                      std::vector<unsigned int>& indices,
                      const glm::vec3& center,
                      const glm::vec3& size,     // (sx, sy, sz) dimensiuni totale
                      const glm::vec3& color)
{
    glm::vec3 h = 0.5f * size;
    unsigned int base = (unsigned int)vertices.size();

    // 8 vertecsi
    vertices.emplace_back(center + glm::vec3(-h.x, -h.y,  h.z), color); // 0
    vertices.emplace_back(center + glm::vec3( h.x, -h.y,  h.z), color); // 1
    vertices.emplace_back(center + glm::vec3( h.x,  h.y,  h.z), color); // 2
    vertices.emplace_back(center + glm::vec3(-h.x,  h.y,  h.z), color); // 3
    vertices.emplace_back(center + glm::vec3(-h.x, -h.y, -h.z), color); // 4
    vertices.emplace_back(center + glm::vec3( h.x, -h.y, -h.z), color); // 5
    vertices.emplace_back(center + glm::vec3( h.x,  h.y, -h.z), color); // 6
    vertices.emplace_back(center + glm::vec3(-h.x,  h.y, -h.z), color); // 7

    // indici
    const unsigned int local[] = {
        0,1,2,  2,3,0,
        1,5,6,  6,2,1,
        5,4,7,  7,6,5,
        4,0,3,  3,7,4,
        3,2,6,  6,7,3,
        4,5,1,  1,0,4
    };

    for (unsigned int k : local) indices.push_back(base + k);
}

// Helper: adauga cilindru. Implicit e pe OZ.
// Daca axisX=true -> il roteste +90° pe OY ca axa sa devina OX.
static void AppendCylinder(std::vector<VertexFormat>& vertices,
                           std::vector<unsigned int>& indices,
                           const glm::vec3& center,
                           float radius,
                           float height,
                           const glm::vec3& color,
                           int sectors,
                           bool axisX)
{
    unsigned int base = (unsigned int)vertices.size();
    float halfH = height * 0.5f;
    float step = 2.0f * glm::pi<float>() / (float)sectors;

    auto rotY90 = [&](const glm::vec3& p) -> glm::vec3 {
        // rotatie +90° in jurul OY, in jurul lui center
        glm::vec3 q = p - center;
        // x' = z, z' = -x
        q = glm::vec3(q.z, q.y, -q.x);
        return center + q;
    };

    auto pushVertex = [&](glm::vec3 p) {
        if (axisX) p = rotY90(p);
        vertices.emplace_back(p, color);
    };

    // inel lateral 
    for (int i = 0; i <= sectors; i++) {
        float a = i * step;
        float x = center.x + radius * cos(a);
        float y = center.y + radius * sin(a);

        pushVertex(glm::vec3(x, y, center.z + halfH)); // top
        pushVertex(glm::vec3(x, y, center.z - halfH)); // bottom
    }

    // laterale
    for (int i = 0; i < sectors; i++) {
        unsigned int top1 = base + 2 * i;
        unsigned int bot1 = base + 2 * i + 1;
        unsigned int top2 = base + 2 * (i + 1);
        unsigned int bot2 = base + 2 * (i + 1) + 1;

        indices.push_back(bot1); indices.push_back(top1); indices.push_back(top2);
        indices.push_back(bot1); indices.push_back(top2); indices.push_back(bot2);
    }

    // centre capete
    unsigned int centerTop = (unsigned int)vertices.size();
    pushVertex(glm::vec3(center.x, center.y, center.z + halfH));

    unsigned int centerBot = (unsigned int)vertices.size();
    pushVertex(glm::vec3(center.x, center.y, center.z - halfH));

    // discuri
    for (int i = 0; i < sectors; i++) {
        unsigned int top1 = base + 2 * i;
        unsigned int top2 = base + 2 * (i + 1);
        unsigned int bot1 = base + 2 * i + 1;
        unsigned int bot2 = base + 2 * (i + 1) + 1;

        indices.push_back(centerTop); indices.push_back(top2); indices.push_back(top1);
        indices.push_back(centerBot); indices.push_back(bot1); indices.push_back(bot2);
    }
}

// CREATE TRAIN (locomotiva + vagon) intr-un singur mesh
// Originea (0,0,0) = centrul locomotivei, orientat pe +X
Mesh* obj3D::CreateTrain(const std::string& name, glm::vec3 center)
{
    std::vector<VertexFormat> vertices;
    std::vector<unsigned int> indices;

    // culori 
    glm::vec3 colWheel(1, 0, 0);
    glm::vec3 colFloor(1, 1, 0);
    glm::vec3 colBody(0.3f, 0.9f, 0.3f);
    glm::vec3 colBlue(0, 0, 1);
    glm::vec3 colMag(1, 0, 1);

    const float wheelRadius  = 0.3f;
    const float wheelCenterY = wheelRadius;
    const float wheelZOffset = 0.4f;
    const float wheelThick   = 0.2f * 0.8f; 

    const float floorHeight  = 0.1f;

    // loco
    const float locoLength   = 6.0f;
    const float locoWidth    = 1.2f;
    const float leftEdge     = -locoLength * 0.5f;
    const float rightEdge    =  locoLength * 0.5f;

    // wagon
    const float wagonLength  = 4.5f;
    const float wagonWidth   = 1.2f;
    const float bodyHeight   = 1.2f;
    const float couplingDist = 5.5f;              // centrul vagonului e in spate
    const float wagonCenterX = -couplingDist;

    // cabine/engine
    const float cabinHeight  = 1.8f;
    const float cabinLength  = 1.6f;

    // roti locomotiva
    float wheelStepX = locoLength / 5.0f; // 6 roti => 5 intervale
    for (int i = 0; i < 6; i++) {
        float x = leftEdge + i * wheelStepX;

        AppendCylinder(vertices, indices,
                       center + glm::vec3(x, wheelCenterY,  wheelZOffset),
                       wheelRadius, wheelThick, colWheel, 18, false);

        AppendCylinder(vertices, indices,
                       center + glm::vec3(x, wheelCenterY, -wheelZOffset),
                       wheelRadius, wheelThick, colWheel, 18, false);
    }

    // platforma locomotiva
    float floorCenterY = 2.0f * wheelRadius + floorHeight * 0.5f;
    AppendBox(vertices, indices,
              center + glm::vec3(0.0f, floorCenterY, 0.0f),
              glm::vec3(locoLength + 0.5f, floorHeight, locoWidth),
              colFloor);

    float floorTopY = floorCenterY + floorHeight * 0.5f;

    // cabina verde (mai lunga)
    float cabinCenterX = leftEdge + cabinLength;       
    float cabinCenterY = floorTopY + cabinHeight * 0.5f;

    AppendBox(vertices, indices,
              center + glm::vec3(cabinCenterX, cabinCenterY, 0.0f),
              glm::vec3(2.0f * cabinLength, cabinHeight, locoWidth),
              colBody);

    // cilindri motor (pe OX)
    float cabinRightX = cabinCenterX + cabinLength;
    float engineStartXFull = cabinRightX;
    float engineEndXFull   = rightEdge;
    float fullLengthX      = engineEndXFull - engineStartXFull;

    float engineRadius  = cabinHeight / 4.0f;
    float engineCenterY = floorTopY + cabinHeight / 4.0f;

    float blueFactor  = 0.85f;
    float blueLengthX = fullLengthX * blueFactor;

    float blueStartX  = engineStartXFull;
    float blueEndX    = blueStartX + blueLengthX;
    float blueCenterX = 0.5f * (blueStartX + blueEndX);

    // cilindru albastru: axa pe OX => axisX=true
    AppendCylinder(vertices, indices,
                   center + glm::vec3(blueCenterX, engineCenterY, 0.0f),
                   engineRadius, blueLengthX, colBlue, 22, true);

    float magStartX  = blueEndX;
    float magEndX    = engineEndXFull;
    float magLengthX = magEndX - magStartX;
    float magCenterX = 0.5f * (magStartX + magEndX);

    if (magLengthX > 1e-4f) {
        AppendCylinder(vertices, indices,
                       center + glm::vec3(magCenterX, engineCenterY, 0.0f),
                       engineRadius * 0.2f, magLengthX, colMag, 18, true);
    }

    // roti vagon
    float wagonFirstWheelX = -1.6f;
    float wagonWheelStepX  = 3.2f; // -1.6 si +1.6

    for (int i = 0; i < 2; i++) {
        float x = wagonCenterX + wagonFirstWheelX + i * wagonWheelStepX;

        AppendCylinder(vertices, indices,
                       center + glm::vec3(x, wheelCenterY,  wheelZOffset),
                       wheelRadius, wheelThick, colWheel, 18, false);

        AppendCylinder(vertices, indices,
                       center + glm::vec3(x, wheelCenterY, -wheelZOffset),
                       wheelRadius, wheelThick, colWheel, 18, false);
    }

    // platforma + corp vagon
    AppendBox(vertices, indices,
              center + glm::vec3(wagonCenterX, floorCenterY, 0.0f),
              glm::vec3(wagonLength, floorHeight, wagonWidth),
              colFloor);

    float bodyCenterY = floorCenterY + floorHeight * 0.5f + bodyHeight * 0.5f;
    AppendBox(vertices, indices,
              center + glm::vec3(wagonCenterX, bodyCenterY, 0.0f),
              glm::vec3(wagonLength, bodyHeight, wagonWidth),
              colBody);

    Mesh* train = new Mesh(name);
    train->InitFromData(vertices, indices);
    return train;
}

Mesh* obj3D::CreateDrezina(const std::string& name, glm::vec3 center)
{
    std::vector<VertexFormat> vertices;
    std::vector<unsigned int> indices;

    auto AddBox = [&](const glm::mat4& T, const glm::vec3& size, const glm::vec3& color)
    {
        const glm::vec3 h = size * 0.5f;
        unsigned int base = (unsigned int)vertices.size();

        glm::vec3 p[8] = {
            {-h.x, -h.y,  h.z}, { h.x, -h.y,  h.z}, { h.x,  h.y,  h.z}, {-h.x,  h.y,  h.z},
            {-h.x, -h.y, -h.z}, { h.x, -h.y, -h.z}, { h.x,  h.y, -h.z}, {-h.x,  h.y, -h.z}
        };

        for (int i = 0; i < 8; i++) {
            glm::vec4 w = T * glm::vec4(p[i], 1.0f);
            vertices.emplace_back(glm::vec3(w), color);
        }

        unsigned int idx[] = {
            0, 1, 2,  2, 3, 0,
            1, 5, 6,  6, 2, 1,
            5, 4, 7,  7, 6, 5,
            4, 0, 3,  3, 7, 4,
            3, 2, 6,  6, 7, 3,
            4, 5, 1,  1, 0, 4
        };

        for (unsigned int k : idx) indices.push_back(base + k);
    };

    // cilindru cu axa pe OZ (local), apoi îl transformam cu matricea T
    auto AddCylinderZ = [&](const glm::mat4& T, float radius, float height, const glm::vec3& color, int sectors = 36)
    {
        unsigned int base = (unsigned int)vertices.size();

        float halfH = height * 0.5f;
        float step = 2.0f * glm::pi<float>() / sectors;

        for (int i = 0; i <= sectors; i++) {
            float a = i * step;
            float x = radius * cos(a);
            float y = radius * sin(a);

            glm::vec3 top(x, y, +halfH);
            glm::vec3 bot(x, y, -halfH);

            glm::vec3 wTop = glm::vec3(T * glm::vec4(top, 1.0f));
            glm::vec3 wBot = glm::vec3(T * glm::vec4(bot, 1.0f));

            vertices.emplace_back(wTop, color);
            vertices.emplace_back(wBot, color);
        }

        for (int i = 0; i < sectors; i++) {
            unsigned int top1 = base + 2 * i;
            unsigned int bot1 = base + 2 * i + 1;
            unsigned int top2 = base + 2 * (i + 1);
            unsigned int bot2 = base + 2 * (i + 1) + 1;

            indices.push_back(bot1); indices.push_back(top1); indices.push_back(top2);
            indices.push_back(bot1); indices.push_back(top2); indices.push_back(bot2);
        }

        unsigned int centerTop = (unsigned int)vertices.size();
        vertices.emplace_back(glm::vec3(T * glm::vec4(0, 0, +halfH, 1.0f)), color);

        unsigned int centerBot = (unsigned int)vertices.size();
        vertices.emplace_back(glm::vec3(T * glm::vec4(0, 0, -halfH, 1.0f)), color);

        for (int i = 0; i < sectors; i++) {
            unsigned int top1 = base + 2 * i;
            unsigned int top2 = base + 2 * (i + 1);

            unsigned int bot1 = base + 2 * i + 1;
            unsigned int bot2 = base + 2 * (i + 1) + 1;

            indices.push_back(centerTop); indices.push_back(top2); indices.push_back(top1);
            indices.push_back(centerBot); indices.push_back(bot1); indices.push_back(bot2);
        }
    };

    // ---- culori
    glm::vec3 colBase  = glm::vec3(1.0f, 0.6f, 0.2f);  // portocaliu
    glm::vec3 colDark  = glm::vec3(0.2f, 0.2f, 0.2f);  // gri inchis
    glm::vec3 colGreen = glm::vec3(0.2f, 0.9f, 0.2f);  // verde

    // ---- dimensiuni (local)
    float baseL = 2.2f, baseH = 0.4f, baseW = 1.4f;

    float wheelR = 0.28f;
    float wheelT = 0.18f;

    float postL = 0.3f, postH = 0.7f, postW = 0.2f;

    float stemR = 0.12f, stemH = 0.55f;

    // bara mare (orizontala) + manere perpendiculare la capete
    float mainBarR = 0.12f;
    float mainBarL =  0.7f * baseL;     // lungime bara orizontala (pe OX)
    float endBarR  = 0.12f;
    float endBarL  = 1.2f;     // fiecare bara perpendiculara (pe OZ)

    // capete verzi pe barele de la capete
    float capR = 0.14f;
    float capL = 0.55f;        // lungime capat verde (pe OZ)
    float endCoreL = glm::max(0.0f, endBarL - 2.0f * capL);

    // 1) platforma (box)
    {
        glm::vec3 c = center + glm::vec3(0.0f, baseH * 1.2f, 0.0f);
        glm::mat4 T = glm::translate(glm::mat4(1.0f), c);
        AddBox(T, glm::vec3(baseL, baseH, baseW), colBase);
    }

    // 2) roti (4 cilindri pe OZ)
    {
        float x1 = -baseL * 0.5f + wheelR;
        float x2 =  baseL * 0.5f - wheelR;

        float zOff = baseW * 0.5f + wheelT * 0.5f;
        float y    = wheelR;

        auto wheelAt = [&](float x, float z)
        {
            glm::vec3 c = center + glm::vec3(x, y, z);
            glm::mat4 T = glm::translate(glm::mat4(1.0f), c);
            AddCylinderZ(T, wheelR, wheelT, colDark, 36);
        };

        wheelAt(x1, +zOff);
        wheelAt(x1, -zOff);
        wheelAt(x2, +zOff);
        wheelAt(x2, -zOff);
    }

    // 3) stalp (box)
    glm::vec3 postTop;
    {
        glm::vec3 c = center + glm::vec3(0.0f, baseH + postH * 0.5f, 0.0f);
        glm::mat4 T = glm::translate(glm::mat4(1.0f), c);
        AddBox(T, glm::vec3(postL, postH, postW), colBase);

        postTop = c + glm::vec3(0, postH * 0.5f, 0);
    }

    // 4) bara verticala (cilindru pe OY)
    // (AddCylinderZ -> rotim Z->Y cu -90° pe OX)
    glm::vec3 stemTop;
    {
        glm::vec3 c = postTop + glm::vec3(0.0f, stemH * 0.5f, 0.0f);

        glm::mat4 T = glm::translate(glm::mat4(1.0f), c)
                    * glm::rotate(glm::mat4(1.0f), -glm::half_pi<float>(), glm::vec3(1,0,0));

        AddCylinderZ(T, stemR, stemH, colDark, 36);
        stemTop = c + glm::vec3(0, stemH * 0.5f, 0);
    }

    // 5) bara orizontala (cilindru pe OX)
    // (AddCylinderZ -> rotim Z->X cu +90° pe OY)
    {
        glm::vec3 c = stemTop;

        glm::mat4 Tmain = glm::translate(glm::mat4(1.0f), c)
                        * glm::rotate(glm::mat4(1.0f), +glm::half_pi<float>(), glm::vec3(0,1,0));

        AddCylinderZ(Tmain, mainBarR, mainBarL, colDark, 36);

        // 6) la capete: doua bare pe OZ (perpendiculare pe bara mare),
        // cu terminatii verzi
        float xEnd = mainBarL * 0.5f;

        auto AddEndHandle = [&](float xSign)
        {
            glm::vec3 endCenter = c + glm::vec3(xSign * xEnd, 0.0f, 0.0f);

            // (a) miez (gri) pe OZ
            if (endCoreL > 1e-4f) {
                glm::mat4 Tcore = glm::translate(glm::mat4(1.0f), endCenter);
                AddCylinderZ(Tcore, endBarR, endCoreL, colDark, 36);
            }

            // (b) capete verzi (doua bucati) pe OZ
            float zShift = (endCoreL * 0.5f) + (capL * 0.5f);

            glm::mat4 Tcap1 = glm::translate(glm::mat4(1.0f), endCenter + glm::vec3(0,0, +zShift));
            glm::mat4 Tcap2 = glm::translate(glm::mat4(1.0f), endCenter + glm::vec3(0,0, -zShift));

            AddCylinderZ(Tcap1, capR, capL, colGreen, 36);
            AddCylinderZ(Tcap2, capR, capL, colGreen, 36);
        };

        AddEndHandle(+1.0f); // capat dreapta pe OX
        AddEndHandle(-1.0f); // capat stanga  pe OX
    }

    Mesh* mesh = new Mesh(name);
    mesh->InitFromData(vertices, indices);
    return mesh;
}


Mesh* obj3D::CreateDrezinaBody(const std::string& name, glm::vec3 center)
{
    std::vector<VertexFormat> vertices;
    std::vector<unsigned int> indices;

    auto AddBox = [&](const glm::mat4& T, const glm::vec3& size, const glm::vec3& color)
    {
        const glm::vec3 h = size * 0.5f;
        unsigned int base = (unsigned int)vertices.size();

        glm::vec3 p[8] = {
            {-h.x, -h.y,  h.z}, { h.x, -h.y,  h.z}, { h.x,  h.y,  h.z}, {-h.x,  h.y,  h.z},
            {-h.x, -h.y, -h.z}, { h.x, -h.y, -h.z}, { h.x,  h.y, -h.z}, {-h.x,  h.y, -h.z}
        };

        for (int i = 0; i < 8; i++) {
            glm::vec4 w = T * glm::vec4(p[i], 1.0f);
            vertices.emplace_back(glm::vec3(w), color);
        }

        unsigned int idx[] = {
            0, 1, 2,  2, 3, 0,
            1, 5, 6,  6, 2, 1,
            5, 4, 7,  7, 6, 5,
            4, 0, 3,  3, 7, 4,
            3, 2, 6,  6, 7, 3,
            4, 5, 1,  1, 0, 4
        };

        for (unsigned int k : idx) indices.push_back(base + k);
    };

    auto AddCylinderZ = [&](const glm::mat4& T, float radius, float height, const glm::vec3& color, int sectors = 36)
    {
        unsigned int base = (unsigned int)vertices.size();

        float halfH = height * 0.5f;
        float step = 2.0f * glm::pi<float>() / sectors;

        for (int i = 0; i <= sectors; i++) {
            float a = i * step;
            float x = radius * cos(a);
            float y = radius * sin(a);

            glm::vec3 top(x, y, +halfH);
            glm::vec3 bot(x, y, -halfH);

            glm::vec3 wTop = glm::vec3(T * glm::vec4(top, 1.0f));
            glm::vec3 wBot = glm::vec3(T * glm::vec4(bot, 1.0f));

            vertices.emplace_back(wTop, color);
            vertices.emplace_back(wBot, color);
        }

        for (int i = 0; i < sectors; i++) {
            unsigned int top1 = base + 2 * i;
            unsigned int bot1 = base + 2 * i + 1;
            unsigned int top2 = base + 2 * (i + 1);
            unsigned int bot2 = base + 2 * (i + 1) + 1;

            indices.push_back(bot1); indices.push_back(top1); indices.push_back(top2);
            indices.push_back(bot1); indices.push_back(top2); indices.push_back(bot2);
        }

        unsigned int centerTop = (unsigned int)vertices.size();
        vertices.emplace_back(glm::vec3(T * glm::vec4(0, 0, +halfH, 1.0f)), color);

        unsigned int centerBot = (unsigned int)vertices.size();
        vertices.emplace_back(glm::vec3(T * glm::vec4(0, 0, -halfH, 1.0f)), color);

        for (int i = 0; i < sectors; i++) {
            unsigned int top1 = base + 2 * i;
            unsigned int top2 = base + 2 * (i + 1);

            unsigned int bot1 = base + 2 * i + 1;
            unsigned int bot2 = base + 2 * (i + 1) + 1;

            indices.push_back(centerTop); indices.push_back(top2); indices.push_back(top1);
            indices.push_back(centerBot); indices.push_back(bot1); indices.push_back(bot2);
        }
    };

    glm::vec3 colBase  = glm::vec3(1.0f, 0.6f, 0.2f);
    glm::vec3 colDark  = glm::vec3(0.2f, 0.2f, 0.2f);

    float baseL = 2.2f, baseH = 0.4f, baseW = 1.4f;
    float wheelR = 0.28f;
    float wheelT = 0.18f;
    float postL = 0.3f, postH = 0.7f, postW = 0.2f;
    float stemR = 0.12f, stemH = 0.55f;

    // 1) platforma (box)
    {
        glm::vec3 c = center + glm::vec3(0.0f, baseH * 1.2f, 0.0f);
        glm::mat4 T = glm::translate(glm::mat4(1.0f), c);
        AddBox(T, glm::vec3(baseL, baseH, baseW), colBase);
    }

    // 2) roti (4 cilindri pe OZ)
    {
        float x1 = -baseL * 0.5f + wheelR;
        float x2 =  baseL * 0.5f - wheelR;

        float zOff = baseW * 0.5f + wheelT * 0.5f;
        float y    = wheelR;

        auto wheelAt = [&](float x, float z)
        {
            glm::vec3 c = center + glm::vec3(x, y, z);
            glm::mat4 T = glm::translate(glm::mat4(1.0f), c);
            AddCylinderZ(T, wheelR, wheelT, colDark, 36);
        };

        wheelAt(x1, +zOff);
        wheelAt(x1, -zOff);
        wheelAt(x2, +zOff);
        wheelAt(x2, -zOff);
    }

    // 3) stalp (box)
    glm::vec3 postTop;
    {
        glm::vec3 c = center + glm::vec3(0.0f, baseH + postH * 0.5f, 0.0f);
        glm::mat4 T = glm::translate(glm::mat4(1.0f), c);
        AddBox(T, glm::vec3(postL, postH, postW), colBase);

        postTop = c + glm::vec3(0, postH * 0.5f, 0);
    }

    // 4) bara verticala (cilindru pe OY)
    {
        glm::vec3 c = postTop + glm::vec3(0.0f, stemH * 0.5f, 0.0f);

        glm::mat4 T = glm::translate(glm::mat4(1.0f), c)
                    * glm::rotate(glm::mat4(1.0f), -glm::half_pi<float>(), glm::vec3(1,0,0));

        AddCylinderZ(T, stemR, stemH, colDark, 36);
    }

    Mesh* mesh = new Mesh(name);
    mesh->InitFromData(vertices, indices);
    return mesh;
}


Mesh* obj3D::CreateDrezinaBar(const std::string& name, glm::vec3 center)
{
    std::vector<VertexFormat> vertices;
    std::vector<unsigned int> indices;

    auto AddCylinderZ = [&](const glm::mat4& T, float radius, float height, const glm::vec3& color, int sectors = 36)
    {
        unsigned int base = (unsigned int)vertices.size();

        float halfH = height * 0.5f;
        float step = 2.0f * glm::pi<float>() / sectors;

        for (int i = 0; i <= sectors; i++) {
            float a = i * step;
            float x = radius * cos(a);
            float y = radius * sin(a);

            glm::vec3 top(x, y, +halfH);
            glm::vec3 bot(x, y, -halfH);

            glm::vec3 wTop = glm::vec3(T * glm::vec4(top, 1.0f));
            glm::vec3 wBot = glm::vec3(T * glm::vec4(bot, 1.0f));

            vertices.emplace_back(wTop, color);
            vertices.emplace_back(wBot, color);
        }

        for (int i = 0; i < sectors; i++) {
            unsigned int top1 = base + 2 * i;
            unsigned int bot1 = base + 2 * i + 1;
            unsigned int top2 = base + 2 * (i + 1);
            unsigned int bot2 = base + 2 * (i + 1) + 1;

            indices.push_back(bot1); indices.push_back(top1); indices.push_back(top2);
            indices.push_back(bot1); indices.push_back(top2); indices.push_back(bot2);
        }

        unsigned int centerTop = (unsigned int)vertices.size();
        vertices.emplace_back(glm::vec3(T * glm::vec4(0, 0, +halfH, 1.0f)), color);

        unsigned int centerBot = (unsigned int)vertices.size();
        vertices.emplace_back(glm::vec3(T * glm::vec4(0, 0, -halfH, 1.0f)), color);

        for (int i = 0; i < sectors; i++) {
            unsigned int top1 = base + 2 * i;
            unsigned int top2 = base + 2 * (i + 1);

            unsigned int bot1 = base + 2 * i + 1;
            unsigned int bot2 = base + 2 * (i + 1) + 1;

            indices.push_back(centerTop); indices.push_back(top2); indices.push_back(top1);
            indices.push_back(centerBot); indices.push_back(bot1); indices.push_back(bot2);
        }
    };

    glm::vec3 colDark  = glm::vec3(0.2f, 0.2f, 0.2f);
    glm::vec3 colGreen = glm::vec3(0.2f, 0.9f, 0.2f);

    // aceleasi dimensiuni ca in Body 
    float baseL = 2.2f, baseH = 0.4f;
    float postH = 0.7f;
    float stemH = 0.55f;

    float mainBarR = 0.12f;
    float mainBarL = 0.7f * baseL;   
    float endBarR  = 0.12f;
    float endBarL  = 1.2f;

    float capR = 0.14f;
    float capL = 0.55f;
    float endCoreL = glm::max(0.0f, endBarL - 2.0f * capL);

    // punctul unde sta bara sus 
    glm::vec3 stemTop = center + glm::vec3(0.0f, baseH + postH + stemH, 0.0f);

    // 5) bara orizontala (cilindru pe OX)  (Z->X cu +90° pe OY)
    {
        glm::vec3 c = stemTop;

        glm::mat4 Tmain = glm::translate(glm::mat4(1.0f), c)
                        * glm::rotate(glm::mat4(1.0f), +glm::half_pi<float>(), glm::vec3(0,1,0));

        AddCylinderZ(Tmain, mainBarR, mainBarL, colDark, 36);

        // 6) la capete: doua bare pe OZ + terminatii verzi
        float xEnd = mainBarL * 0.5f;

        auto AddEndHandle = [&](float xSign)
        {
            glm::vec3 endCenter = c + glm::vec3(xSign * xEnd, 0.0f, 0.0f);

            // miez gri
            if (endCoreL > 1e-4f) {
                glm::mat4 Tcore = glm::translate(glm::mat4(1.0f), endCenter);
                AddCylinderZ(Tcore, endBarR, endCoreL, colDark, 36);
            }

            // capete verzi
            float zShift = (endCoreL * 0.5f) + (capL * 0.5f);

            glm::mat4 Tcap1 = glm::translate(glm::mat4(1.0f), endCenter + glm::vec3(0,0, +zShift));
            glm::mat4 Tcap2 = glm::translate(glm::mat4(1.0f), endCenter + glm::vec3(0,0, -zShift));

            AddCylinderZ(Tcap1, capR, capL, colGreen, 36);
            AddCylinderZ(Tcap2, capR, capL, colGreen, 36);
        };

        AddEndHandle(+1.0f);
        AddEndHandle(-1.0f);
    }

    Mesh* mesh = new Mesh(name);
    mesh->InitFromData(vertices, indices);
    return mesh;
}

Mesh* obj3D::CreatePyramid(const std::string& name, glm::vec3 center, float baseLength, float height, glm::vec3 color)
{
    std::vector<VertexFormat> vertices;
    std::vector<unsigned int> indices;

    float halfL = baseLength * 0.5f;

    // varf
    vertices.emplace_back(glm::vec3(center.x, center.y + height, center.z), color);

    // baza (4 colturi)
    vertices.emplace_back(glm::vec3(center.x - halfL, center.y, center.z - halfL), color); // 1
    vertices.emplace_back(glm::vec3(center.x + halfL, center.y, center.z - halfL), color); // 2
    vertices.emplace_back(glm::vec3(center.x + halfL, center.y, center.z + halfL), color); // 3
    vertices.emplace_back(glm::vec3(center.x - halfL, center.y, center.z + halfL), color); // 4

    // fete laterale
    indices.push_back(0); indices.push_back(1); indices.push_back(2); // fata 1-2
    indices.push_back(0); indices.push_back(2); indices.push_back(3); // fata 2-3
    indices.push_back(0); indices.push_back(3); indices.push_back(4); // fata 3-4
    indices.push_back(0); indices.push_back(4); indices.push_back(1); // fata 4-1
    
    // baza
    indices.push_back(1); indices.push_back(4); indices.push_back(3);
    indices.push_back(1); indices.push_back(3); indices.push_back(2);

    Mesh* pyramid = new Mesh(name);
    pyramid->InitFromData(vertices, indices);
    return pyramid;
}