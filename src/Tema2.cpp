#include "lab_m1/Tema2/Tema2.h"

#include <vector>
#include <string>
#include <iostream>
#include <cstdlib>
#include <ctime>

#include "lab_m1/Tema2/transform3D.h"
#include "lab_m1/Tema2/transf2D.h"
#include "lab_m1/Tema2/obj3D.h"
#include "lab_m1/Tema2/obj2D.h"

using namespace std;
using namespace m1;

Tema2::Tema2()
{
}


Tema2::~Tema2()
{
}

// calculeaza distanta minima in planul XZ dintre un punct p si segmentul [a,b]
static float DistPointToSegmentXZ(const glm::vec3& p,
                                  const glm::vec3& a,
                                  const glm::vec3& b)
{
    glm::vec2 P(p.x, p.z); // proiecteaza punctul pe XZ
    glm::vec2 A(a.x, a.z); // capat segment A pe XZ
    glm::vec2 B(b.x, b.z); // capat segment B pe XZ

    glm::vec2 AB = B - A;          // vectorul segmentului
    float ab2 = glm::dot(AB, AB);  // lungime^2
    if (ab2 < 1e-8f) return glm::length(P - A); // segment degenerat -> dist la A

    float t = glm::dot(P - A, AB) / ab2; // proiectie scalara pe AB
    t = glm::clamp(t, 0.0f, 1.0f);       // limiteaza in interiorul segmentului

    glm::vec2 Q = A + t * AB;            // punctul proiectat pe segment
    return glm::length(P - Q);           // distanta minima
}

int RandInt(int a, int b) {
    return a + rand() % (b - a + 1);
}

void Tema2::Init()
{
    polygonMode = GL_FILL;

    Shader* railShader = new Shader("RailDeform");
    railShader->AddShader(PATH_JOIN(window->props.selfDir, SOURCE_PATH::M1, "Tema2", "shaders", "VertexShader.glsl"), GL_VERTEX_SHADER);
    railShader->AddShader(PATH_JOIN(window->props.selfDir, SOURCE_PATH::M1, "Tema2", "shaders", "FragmentShader.glsl"), GL_FRAGMENT_SHADER);
    railShader->CreateAndLink();
    shaders["RailDeform"] = railShader;

    // TILE pentru campie (verde)
    Mesh* tilePlain = obj3D::CreateCube("tile_plain", glm::vec3(0, 0, 0), 1.0f, glm::vec3(10, 148, 65) / 255.0f);   // verde
    meshes["tile_plain"] = tilePlain;

    // TILE pentru apa (albastru)
    Mesh* tileWater = obj3D::CreateCube("tile_water", glm::vec3(0, 0, 0), 1.0f, glm::vec3(0.1f, 0.3f, 0.9f));   // albastru
    meshes["tile_water"] = tileWater;

    // TILE pentru munte (maroniu)
    Mesh* tileMountain = obj3D::CreateCube("tile_mountain", glm::vec3(0, 0, 0), 1.0f, glm::vec3(0.5f, 0.3f, 0.1f));   // maroniu
    meshes["tile_mountain"] = tileMountain;

    // sina normala
    Mesh* railPiece = obj3D::CreateCube("rail_piece", glm::vec3(0, 0, 0), 1.0f, glm::vec3(0.05f, 0.05f, 0.1f));  // aproape negru
    meshes["rail_piece"] = railPiece;

    glm::vec3 dark = glm::vec3(0.02f, 0.02f, 0.06f);  // negru
    glm::vec3 light = glm::vec3(1.0f, 1.0f, 1.0f);    // alb

    // sina pentru pod
    Mesh* bridgeMesh = obj3D::CreateBridge("rail_bridge", glm::vec3(0, 0, 0), 1.0f, dark, light,5);
    meshes["rail_bridge"] = bridgeMesh;

    // sina pentru tunel
    Mesh* tunnelMesh = obj3D::CreateTunnel("rail_tunnel", glm::vec3(0, 0, 0), 1.0f, dark, light, 0.6f);
    meshes["rail_tunnel"] = tunnelMesh;

    // tren
    Mesh* trainMesh = obj3D::CreateTrain("train", glm::vec3(0,0,0));
    meshes["train"] = trainMesh;

    // drezina - corp si bara animata
    meshes["drezina_body"] = obj3D::CreateDrezinaBody("drezina_body", glm::vec3(0));
    meshes["drezina_bar"]  = obj3D::CreateDrezinaBar ("drezina_bar",  glm::vec3(0));

    // sine avariate
    meshes["rail_piece_dmg1"] = obj3D::CreateCube("rail_piece_dmg1", glm::vec3(0,0,0), 1.0f,
                                             glm::vec3(0.35f, 0.05f, 0.05f)); // rosu inchis

    meshes["rail_piece_dmg2"] = obj3D::CreateCube("rail_piece_dmg2", glm::vec3(0,0,0), 1.0f,
                                             glm::vec3(0.8f, 0.6f, 0.1f));   // galben/portocaliu (flicker)

    // obiecte pentru mini-harta                                        
    meshes["minimap_train"] = obj3D::CreateCube("minimap_train", glm::vec3(0), 1.0f, glm::vec3(0, 1, 0));
    meshes["minimap_player"] = obj3D::CreateCube("minimap_player", glm::vec3(0), 1.0f, glm::vec3(1, 1, 0)); 

    AddMeshToList(obj2D::CreateRectangle("ui_bar_bg",   glm::vec3(0,0,0), 1, 1, glm::vec3(0.2f), true));
    AddMeshToList(obj2D::CreateRectangle("ui_bar_fill", glm::vec3(0,0,0), 1, 1, glm::vec3(0.1f, 0.9f, 0.2f), true));
    AddMeshToList(obj2D::CreateRectangle("ui_bar_outline", glm::vec3(0,0,0), 1, 1, glm::vec3(1), false));

    // Initializeaza drezina
    drezina.position = glm::vec3(0, 0, 0);
    drezina.yaw = 0.0f;

    float worldSize = 25.0f;   // cat de mare e harta (pe X si Z)
    tileSize = 1.0f;           // cat de mare e un tile

    // Dimensiuni tren 
    float wheelRadius  = 0.3f;
    const float wheelZOffset  = 0.6f;
    const float wheelScaleZ   = 0.8f;

    const float locoLength    = 6.0f;
    const float wagonLength   = 4.5f;
    const float couplingDist  = 5.5f;  

    const float cabinHeight   = 1.8f;
    const float wheelTopY     = 2.0f * wheelRadius;        
    const float floorHeight   = 0.1f;
    const float floorTopY     = wheelTopY + floorHeight;  
    const float cabinTopY     = floorTopY + cabinHeight;   

    // lungime totala 
    const float totalTrainLen = locoLength * 0.5f + couplingDist + wagonLength * 0.5f; // 10.75

    // latime totala 
    const float halfWidth = wheelZOffset + wheelRadius * wheelScaleZ; // 0.6 + 0.24 = 0.84
    const float totalWidth = 2.0f * halfWidth;                        // 1.68

    // inaltime totala 
    const float totalHeight = cabinTopY;                              // ~2.5

    // scale ca sa iasa 1 tile pe fiecare axa, mai putin latime 
    trainScaleX = tileSize / totalTrainLen;   
    trainScaleY = tileSize / totalHeight;     
    trainScaleZ = 0.8f *tileSize / totalWidth;     

    // dimensiuni drezina 
    const float baseL = 2.2f, baseH = 0.4f, baseW = 1.4f;

    const float wheelR = 0.28f;
    const float wheelT = 0.18f;

    const float postH = 1.0f;
    const float stemH = 0.55f;

    const float mainBarL = 0.7f * baseL;   
    const float mainBarR = 0.12f;

    // lungime pe X
    const float totalLenX = baseL;

    // latime pe Z: platforma + roti iesite in exterior
    const float zOff = baseW * 0.5f + wheelT * 0.5f;
    const float totalWidthZ = 2.0f * (zOff + wheelR);   // roata are raza pe Z

    // inaltime pe Y: roti (pana la ax) + platforma + stalp + tija (bara e la stemTop)
    const float topY = baseH + postH + stemH;  // aproximativ
    const float totalHeightY = topY;

    // scale ca sa incapa 1 tile
    drezinaScaleX = tileSize / totalLenX;
    drezinaScaleY = tileSize / totalHeightY;
    drezinaScaleZ = tileSize / totalWidthZ;

    rows = (int)(worldSize / tileSize);
    cols = (int)(worldSize / tileSize);

    srand((unsigned int)time(nullptr));

    // rezolutie mini-viewport
    glm::ivec2 resolution = window->GetResolution();
    resolution /= 2;
    miniViewportArea = ViewportArea(50, 50, resolution.x / 5.f, resolution.y / 5.f);

    GenerateTerrain();
    BuildRailRoad();
    CreateStations();

    for (Rail* rr = railHead; rr; rr = rr->next) {
        rr->seed = (float)rand() / (float)RAND_MAX * 100.0f; // pt flicker
        if (rr->next == railHead) break; 
    }

    drezinaRail = railHead;
    drezinaT = 0.0f;

    if (drezinaRail) {
        drezina.position = glm::mix(drezinaRail->start, drezinaRail->end, drezinaT);
        glm::vec3 dir = drezinaRail->end - drezinaRail->start;
        dir.y = 0;
        if (glm::length(dir) > 1e-6f) {
            dir = glm::normalize(dir);
            drezina.yaw = atan2(dir.z, dir.x);
        }
    }

    SpawnOneDamagedTile();
     
    // Initializeaza tren
    train.rail = railHead;
    train.progress = 0.0f;
    train.speed = 1.8f;  
    train.position = railHead ? railHead->start : glm::vec3(0);

    // matrice proiectie
    float fov = glm::radians(60.0f);
    float aspect = window->props.aspectRatio;
    projectionMatrix = glm::perspective(fov, aspect, 0.01f, 200.0f);

    // raza de reparare
    repairRadius = tileSize * 0.9f;

    // alege o destinatie initiala pentru tren
    PickNewTrainDestination();

    // initializeaza text renderer
    TextRenderer = new gfxc::TextRenderer(window->props.selfDir,
                                      window->props.resolution.x,
                                      window->props.resolution.y);
    TextRenderer->Load(window->props.selfDir + "/assets/fonts/Hack-Bold.ttf", fontSize);

    // setari de joc
    gameTime = 0.0f;
    gameOver = false;
    endGameReason.clear();

    repairDuration = 4.0f;   // 4 secunde pentru reparat
    repairProgress = 0.0f;
    repairTarget = nullptr;

    trainWaitTime = 0.0f;

    maxTrainWaitAllowed = 20.0f;   // 20 sec
    maxDamagedAllowed   = 10;      // 10 avarii simultan
    trainWaitTime       = 0.0f;
}

void Tema2::UpdateTopDownCameraAt(const glm::vec3& target, float height)
{
    auto cam = GetSceneCamera();
    if (!cam) return;

    glm::vec3 camPos = target + glm::vec3(0.0f, height, 0.0f);

    glm::vec3 dir = glm::normalize(target - camPos); // (0,-1,0)
    glm::vec3 up  = glm::vec3(0, 0, -1);            // IMPORTANT pt privit in jos

    glm::quat q = glm::quatLookAt(dir, up);
    cam->SetPositionAndRotation(camPos, q);
    cam->Update();
}

void Tema2::RenderRepairBar2D()
{
    if (!repairTarget) return;

    glm::ivec2 res = window->GetResolution();
    res /= 2;

    glDisable(GL_DEPTH_TEST);

    float x = 25.0f;
    float y = (float)res.y - 90.0f;
    float W = 280.0f;
    float H = 18.0f;

    float p = glm::clamp(repairProgress, 0.0f, 1.0f);

    // BG
    {
        glm::mat3 m(1);
        m *= transform2D::Translate(x, y);
        m *= transform2D::Scale(W, H);
        RenderMesh2D(meshes["ui_bar_bg"], shaders["VertexColor"], m);
    }

    // FILL
    {
        glm::mat3 m(1);
        m *= transform2D::Translate(x, y);
        m *= transform2D::Scale(W * p, H);
        RenderMesh2D(meshes["ui_bar_fill"], shaders["VertexColor"], m);
    }

    // OUTLINE
    {
        glm::mat3 m(1);
        m *= transform2D::Translate(x, y);
        m *= transform2D::Scale(W, H);
        RenderMesh2D(meshes["ui_bar_outline"], shaders["VertexColor"], m);
    }

    glEnable(GL_DEPTH_TEST);
}

void Tema2::GetMinimapBounds(float& minX, float& maxX, float& minZ, float& maxZ) const
{
    minX =  1e9f; maxX = -1e9f;
    minZ =  1e9f; maxZ = -1e9f;

    // rails
    if (railHead) {
        const Rail* r = railHead;
        do {
            minX = std::min(minX, std::min(r->start.x, r->end.x));
            maxX = std::max(maxX, std::max(r->start.x, r->end.x));
            minZ = std::min(minZ, std::min(r->start.z, r->end.z));
            maxZ = std::max(maxZ, std::max(r->start.z, r->end.z));
            r = r->next;
        } while (r && r != railHead);
    }

    // statii
    for (const auto& s : stations) {
        minX = std::min(minX, s.position.x);
        maxX = std::max(maxX, s.position.x);
        minZ = std::min(minZ, s.position.z);
        maxZ = std::max(maxZ, s.position.z);
    }

    // fallback daca ceva e gol
    if (minX > maxX) { minX = -5; maxX = 5; minZ = -5; maxZ = 5; }
}

void Tema2::RenderMinimap(bool mainWasTPS, const glm::mat4& mainProj)
{
    GLint oldVp[4];
    glGetIntegerv(GL_VIEWPORT, oldVp);

    glViewport(miniViewportArea.x, miniViewportArea.y,
               miniViewportArea.width, miniViewportArea.height);

    glEnable(GL_SCISSOR_TEST);
    glScissor(miniViewportArea.x, miniViewportArea.y,
              miniViewportArea.width, miniViewportArea.height);

    //  ramane scena din spate (transparent efect)
    glClear(GL_DEPTH_BUFFER_BIT);  
    glDisable(GL_SCISSOR_TEST);

    float minX, maxX, minZ, maxZ;
    GetMinimapBounds(minX, maxX, minZ, maxZ); // limitele pt minimap

    float cx = 0.5f * (minX + maxX); // centru X
    float cz = 0.5f * (minZ + maxZ); // centru Z

    float pad = tileSize; // padding
    float halfX = 0.5f * (maxX - minX) + pad; // jumatate latime X
    float halfZ = 0.5f * (maxZ - minZ) + pad; // jumatate latime Z

    float half = std::max(halfX, halfZ); // jumatate latime/inaltime finala

    projectionMatrix = glm::ortho(-half, half, -half, half, 0.01f, 150.0f);
    UpdateTopDownCameraAt(glm::vec3(cx, 0.0f, cz), 40.0f);

    // randare minimap 
    RenderTerrain();
    RenderRails();

    // statiile: randate si ele (mai mici)
    for (size_t i = 0; i < stations.size(); i++) {
        std::string meshName = "station_" + std::to_string(i);

        glm::mat4 m(1.0f);

        // lift mic ca sa nu se ingroape in sol
        float y = 0.10f;

        // scale mic pentru minimap
        float s = 0.35f;
        if (i == 0) {
            m *= transform3D::Translate(stations[i].position.x, y, stations[i].position.z);
            m *= transform3D::Scale(s, s, s);
        } else if (i == 1) {
            m *= transform3D::Translate(stations[i].position.x, y, stations[i].position.z);
            m *= transform3D::Scale(s, s, s);
        } else {
            m *= transform3D::Translate(stations[i].position.x, y, stations[i].position.z);
            m *= transform3D::Scale(s, s, s);
        }

        RenderMesh(meshes[meshName], shaders["VertexColor"], m);
    }

    // marker jucator (drezina)
    if (meshes.count("minimap_player")) {
        glm::mat4 m(1.0f);
        m *= transform3D::Translate(drezina.position.x, 0.12f, drezina.position.z);
        m *= transform3D::Scale(tileSize * 0.35f, 0.15f, tileSize * 0.35f);
        RenderMesh(meshes["minimap_player"], shaders["VertexColor"], m);
    }

    // marker tren (verde)
    if (meshes.count("minimap_train")) {
        glm::mat4 m(1.0f);
        m *= transform3D::Translate(train.position.x, 0.12f, train.position.z);
        m *= transform3D::Scale(tileSize * 0.35f, 0.15f, tileSize * 0.35f);
        RenderMesh(meshes["minimap_train"], shaders["VertexColor"], m);
    }

    // restore viewport
    glViewport(oldVp[0], oldVp[1], oldVp[2], oldVp[3]);

    // restore proiectie + camera principala
    projectionMatrix = mainProj;
    if (mainWasTPS) UpdateTPSCamera();
    else            UpdateTopDownCamera();
}

int Tema2::CountDamagedRails() const
{
    if (!railHead) return 0;

    int cnt = 0;
    const Rail* curr = railHead;
    do {
        if (curr->damaged) cnt++;
        curr = curr->next;
    } while (curr && curr != railHead);

    return cnt;
}

Rail* Tema2::FindClosestDamagedRail(float& outDist) const
{
    outDist = 1e9f;
    if (!railHead) return nullptr;

    Rail* best = nullptr;
    Rail* curr = railHead;

    do {
        if (curr->damaged) {
            float d = DistPointToSegmentXZ(drezina.position, curr->start, curr->end);
            if (d < outDist) {
                outDist = d;
                best = curr;
            }
        }
        curr = curr->next;
    } while (curr && curr != railHead);

    return best;
}

std::string Tema2::FormatTimeMMSS(float sec) const
{
    int s = (int)std::floor(sec);
    int mm = s / 60;
    int ss = s % 60;

    char buf[32];
    snprintf(buf, sizeof(buf), "%02d:%02d", mm, ss);
    return std::string(buf);
}

void Tema2::UpdateRepairHold(float dt)
{
    if (gameOver) return;

    bool F = window->KeyHold(GLFW_KEY_F);
    if (!F) {
        repairTarget = nullptr;
        repairProgress = 0.0f;
        return;
    }

    float dist;
    Rail* cand = FindClosestDamagedRail(dist);

    // nu e nimic avariat sau nu sunt in raza
    if (!cand || dist > repairRadius) {
        repairTarget = nullptr;
        repairProgress = 0.0f;
        return;
    }

    // pastrez tinta 
    if (cand != repairTarget) {
        repairTarget = cand;
        repairProgress = 0.0f;
    }

    // cresc progresul
    if (repairDuration <= 0.0f) repairDuration = 4.0f;
    repairProgress += dt / repairDuration;
    repairProgress = glm::clamp(repairProgress, 0.0f, 1.0f);

    // complet -> repar instant
    if (repairProgress >= 1.0f && repairTarget) {
        // repar tinta
        repairTarget->damaged = false;
        repairTarget->damageTime = 0.0f;

        // daca folosesc tile=2 jumatati, repar si vecini "lipiti"
        const float EPS = 1e-4f;

        if (repairTarget->next && repairTarget->next->damaged &&
            glm::length(repairTarget->end - repairTarget->next->start) < EPS) {
            repairTarget->next->damaged = false;
            repairTarget->next->damageTime = 0.0f;
        }

        if (repairTarget->prev && repairTarget->prev->damaged &&
            glm::length(repairTarget->start - repairTarget->prev->end) < EPS) {
            repairTarget->prev->damaged = false;
            repairTarget->prev->damageTime = 0.0f;
        }

        repairTarget = nullptr;
        repairProgress = 0.0f;
    }
}

void Tema2::RenderUI()
{
    glm::vec3 white(1.0f, 1.0f, 1.0f);
    glm::vec3 red  (1.0f, 0.0f, 0.0f);

    if (!TextRenderer) return;

    glm::ivec2 res = window->GetResolution();
    float x = 20.0f;
    float y = (float)res.y - 40.0f;

    int damaged = CountDamagedRails();

    std::string line1 = "Timp joc: " + FormatTimeMMSS(gameTime);
    std::string line2 = "Segmente avariate: " + std::to_string(damaged) +
                        " / " + std::to_string(maxDamagedAllowed);

    char buf[128];
    snprintf(buf, sizeof(buf), "Asteptare tren: %.1fs / %.0fs",
             trainWaitTime, maxTrainWaitAllowed);
    std::string line3(buf);

    TextRenderer->RenderText(line1, res.x * 0.5f - 225.0f, res.y * 0.5f - 1400.0f, 1.0f, red);
    TextRenderer->RenderText(line2,res.x * 0.5f - 350.0f, res.y * 0.5f - 1300.0f, 1.0f, red);
    TextRenderer->RenderText(line3, res.x * 0.5f - 350.0f, res.y * 0.5f - 1200.0f, 1.0f, red);

    // bara de reparatie (doar daca tii F pe un segment in raza) 
    if (repairTarget) {
        int blocks = 20;
        int filled = (int)std::round(repairProgress * blocks);
        filled = glm::clamp(filled, 0, blocks);

        std::string bar = "[";
        for (int i = 0; i < blocks; i++) bar += (i < filled ? "#" : "-");
        bar += "]";

        int pct = (int)std::round(repairProgress * 100.0f);
        std::string line4 = "Reparatie " + bar + " " + std::to_string(pct) + "%";

        TextRenderer->RenderText(line4, res.x*0.5f-2500.0f, res.y*0.5f - 1250.0f, 1.0f, glm::vec3(1));
        TextRenderer->RenderText("Tine apasat F in raza avariei", res.x*0.5f-2500.0f, res.y*0.5f - 1200.0f, 0.9f, glm::vec3(1));
    }

    glm::vec3 c(1);
    std::string destLine = "Destinatie: -";
    if (trainDestStation >= 0 && trainDestStation < (int)stations.size()) {
        destLine = "Destinatie: " + stations[trainDestStation].name;
        c = stations[trainDestStation].color;
    }

    TextRenderer->RenderText(destLine,res.x * 0.5f - 2500.0f, res.y * 0.5f - 1300.0f,
                                 1.5f, c);


    // END GAME 
    if (gameOver) {
        std::string title = "END GAME";
        TextRenderer->RenderText(title, res.x * 0.5f - 1600.0f, res.y * 0.5f - 700.0f,
                                 1.5f, glm::vec3(1, 0.2f, 0.2f));

        TextRenderer->RenderText(endGameReason, res.x * 0.5f - 1900.0f, res.y * 0.5f - 700.0f,
                                 1.0f, glm::vec3(1));

        TextRenderer->RenderText("Restart: Ruleaza din nou executabilul",
                                 res.x * 0.5f - 1900.0f, res.y * 0.5f - 600.0f,
                                 0.9f, glm::vec3(1));
    }
}

void Tema2::CheckEndGame()
{
    if (gameOver) return;

    int damaged = CountDamagedRails();
    if (damaged >= maxDamagedAllowed) {
        gameOver = true;
        endGameReason = "Prea multe segmente avariate simultan (" +
                        std::to_string(damaged) + ").";
        return;
    }

    if (trainWaitTime >= maxTrainWaitAllowed) {
        gameOver = true;
        endGameReason = "Un tren a asteptat prea mult in fata unei avarii (" +
                        std::to_string((int)trainWaitTime) + "s).";
        return;
    }
}

void Tema2::CreateStations()
{
    stations.clear();                 // goleste lista de statii existente
    if (!railHead) return;            // daca nu exista sine, iesi

    float stationRadius = tileSize * 0.7f; // raza statiei, raportata la marimea tile-ului
    float offsetDist    = tileSize;        // cat de departe de sina plasam statia (pe lateral)

    std::vector<Rail*> railSegments;  // colecteaza segmentele pentru indexare usoara
    Rail* curr = railHead;
    do {
        railSegments.push_back(curr);
        curr = curr->next;
    } while (curr && curr != railHead);

    int count = (int)railSegments.size();  // numar total de segmente
    if (count < 10) return;                // nu pun statii pe trasee prea scurte

    // aleg 3 pozitii pe traseu (aprox la 1/8, 1/2, 7/8 din lungime)
    int indices[3] = { count / 8, count / 2, (7 * count) / 8 };

    // culori diferite pentru fiecare statie
    glm::vec3 colors[3] = {
        glm::vec3(1.0f, 0.1f, 0.1f),          // rosu
        glm::vec3(0.1f, 0.4f, 1.0f),          // albastru
        glm::vec3(10, 84, 65) / 255.0f        // verde
    };

    for (int i = 0; i < 3; i++) {
        Rail* r = railSegments[indices[i]];                 // segmentul pe care ancorez statia
        glm::vec3 railCenter = (r->start + r->end) * 0.5f;  // centru segment

        // directie perpendiculara pe sina (in plan XZ) ca sa o pun in lateral
        glm::vec3 outsideDir = glm::vec3(r->dir.z, 0, -r->dir.x);

        // pozitia finala a statiei: deplasata lateral fata de sina
        glm::vec3 finalPos = railCenter + outsideDir * offsetDist;
        finalPos.y = 0.0f;  // statia sta pe sol

        // configurez statia
        Station s;
        s.position = finalPos;
        s.color = colors[i];
        s.radius = stationRadius;
        if (i == 0) {
            s.name = "Statia rosie";
        } else if (i == 1) {
            s.name = "Statia albastra";
        } else {
            s.name = "Statia verde";
        }
        stations.push_back(s);

        // nume mesh unic pentru statia curenta
        std::string meshName = "station_" + std::to_string(i);

        // creez mesh-ul in functie de index: sfera, paralelipiped, piramida
        if (i == 0) {
            // 0 = SFERA
            meshes[meshName] = obj3D::CreateSphere(meshName, glm::vec3(0), s.radius, s.color, 40, 20);
        }
        else if (i == 1) {
            // 1 = PARALELIPIPED 
            meshes[meshName] = obj3D::CreateCube(meshName, glm::vec3(0), 1.0f, s.color);
        }
        else {
            // 2 = PIRAMIDA 
            meshes[meshName] = obj3D::CreatePyramid(meshName, glm::vec3(0), 1.0f, 1.0f, s.color);
        }
    }
}

void Tema2::RenderStations()
{
    for (size_t i = 0; i < stations.size(); i++) {
        std::string meshName = "station_" + std::to_string(i);

        // PULSE doar pentru destinatie
        float pulse = 1.0f;
        if (destPulseActive && (int)i == trainDestStation) {
            float s = 0.5f + 0.5f * sin(tAnim * destPulseSpeed); // 0..1
            pulse = 1.0f + destPulseAmp * s;
        }

        glm::mat4 model = glm::mat4(1.0f);

        if (i == 0) {
            // SFERA 
            model *= transform3D::Translate(stations[i].position.x,
                                            stations[i].radius,
                                            stations[i].position.z);
            model *= transform3D::Scale(pulse, pulse, pulse);   // <-- puls
        }
        else if (i == 1) {
            // PARALELIPIPED 
            float w = tileSize * 1.6f * pulse;
            float h = tileSize * 1.2f * pulse;
            float d = tileSize * 1.0f * pulse;

            model *= transform3D::Translate(stations[i].position.x,
                                            h * 0.5f,
                                            stations[i].position.z);
            model *= transform3D::Scale(w, h, d);
        }
        else {
            // PIRAMIDA 
            float base = tileSize * 1.6f * pulse;
            float h    = tileSize * 1.8f * pulse;

            model *= transform3D::Translate(stations[i].position.x- tileSize * 0.5f,
                                            0.0f,
                                            stations[i].position.z );
            model *= transform3D::Scale(base, h, base);
        }

        RenderMesh(meshes[meshName], shaders["VertexColor"], model);
    }
}

void Tema2::InitTrack()
{
    track.clear();

    glm::vec3 pos(0.0f, 0.0f, 0.0f);
    glm::vec3 dir(1.0f, 0.0f, 0.0f);
    float rotY = 0.0f;

    auto addStraight = [&](int count) {
        for (int i = 0; i < count; i++) {
            track.push_back({ pos, rotY });
            pos += dir * tileSize;   
        }
    };

    auto turnLeft = [&]() {
        rotY += glm::half_pi<float>();           // +90
        dir  = glm::vec3(-dir.z, 0.0f, dir.x);   // rotim directia cu +90
    };

    auto turnRight = [&]() {
        rotY -= glm::half_pi<float>();           // -90
        dir  = glm::vec3(dir.z, 0.0f, -dir.x);
    };

    addStraight(6);   // 1) inainte pe +X
    turnLeft();       // curba 1 (spre +Z)
    addStraight(4);   // 2) pe +Z
    turnRight();      // curba 2 (spre +X)
    addStraight(5);   // 3) pe +X
    turnRight();      // curba 3 (spre -Z)
    addStraight(4);   // 4) pe -Z
}

static TurnDir TurnFromDirs(const glm::vec3& a, const glm::vec3& b)
{
    // a si b sunt directii normalizate in XZ
    float d = glm::clamp(glm::dot(a, b), -1.0f, 1.0f);

    // daca e aproape paralel => nu e curba
    if (d > 0.999f) return TurnDir::None;

    // semnul cross-ului pe Y iti spune stanga/dreapta
    float crossY = a.x * b.z - a.z * b.x;  // (a × b).y pentru vectori in plan XZ

    if (crossY >  1e-6f) return TurnDir::Left;
    if (crossY < -1e-6f) return TurnDir::Right;
    return TurnDir::None;
}

void Tema2::ComputeRailOrientation()
{
    if (!railHead) return;

    Rail* r = railHead;
    Rail* first = railHead;

    int guard = 0;
    const int maxSegments = 5000;

    // 1) dir + yaw
    do {
        glm::vec3 d = r->end - r->start;
        d.y = 0.0f;

        float len = glm::length(d);
        if (len > 1e-6f) {
            r->dir = d / len;
            r->yaw = atan2(r->dir.z, r->dir.x);
        } else {
            r->dir = glm::vec3(1, 0, 0);
            r->yaw = 0.0f;
        }

        r = r->next;
        guard++;
    } while (r && r != first && guard < maxSegments);

    // 2) turnIn / turnOut
    r = railHead;
    guard = 0;
    do {
        Rail* p = r->prev ? r->prev : nullptr;
        Rail* n = r->next ? r->next : nullptr;

        if (p) r->turnIn  = TurnFromDirs(p->dir, r->dir);
        else   r->turnIn  = TurnDir::None;

        if (n) r->turnOut = TurnFromDirs(r->dir, n->dir);
        else   r->turnOut = TurnDir::None;

        r = r->next;
        guard++;
    } while (r && r != first && guard < maxSegments);
}

void Tema2::RenderTrack()
{
    float railY = 0.00f;  

    // aceleasi valori ca in RenderTerrain
    float halfW = cols * tileSize / 2.0f;
    float halfH = rows * tileSize / 2.0f;

    for (const auto& seg : track) {
        float jFloat = (seg.center.x + halfW - tileSize / 2.0f) / tileSize;
        float iFloat = (seg.center.z + halfH - tileSize / 2.0f) / tileSize;

        int j = (int)glm::round(jFloat);   // col
        int i = (int)glm::round(iFloat);   // row

        i = glm::clamp(i, 0, rows - 1);
        j = glm::clamp(j, 0, cols - 1);

        // 2) recalculez CENTRUL tile-ului, ca in RenderTerrain
        float xPos = j * tileSize - halfW + tileSize / 2.0f;
        float zPos = i * tileSize - halfH + tileSize / 2.0f;

        // 3) aleg mesh-ul in functie de tipul terenului
        const char* meshName = "rail_piece";
        switch (terrain[i][j]) {
        case TileType::Plain:
            meshName = "rail_piece";  
            break;
        case TileType::Water:
            meshName = "rail_bridge";  
            break;
        case TileType::Mountain:
            meshName = "rail_tunnel"; 
            break;
        }

        // 4) desenez sina exact peste tile
        glm::mat4 model = glm::mat4(1.0f);
        model *= transform3D::Translate(xPos, railY, zPos);
        model *= transform3D::RotateOY(seg.rotY);
        model *= transform3D::Scale(tileSize, 0.1f, tileSize);

        RenderMesh(meshes[meshName], shaders["VertexColor"], model);
    }
}

void Tema2::RenderTrain(const glm::vec3& position, float yaw)
{
    glm::mat4 model = glm::mat4(1);
    model *= transform3D::Translate(position.x, position.y, position.z);
    model *= transform3D::RotateOY(-yaw);
    model *= transform3D::Scale(trainScaleX, trainScaleY, trainScaleZ);

    RenderMesh(meshes["train"], shaders["VertexColor"], model);
}

void Tema2::GenerateTerrain()
{
    terrain.assign(rows, std::vector<TileType>(cols, TileType::Plain));

    // harta originala pentru care am ales coordonatele era 50x50 
    const float REF = 50.0f;
    float sR = rows / REF;                 // scale pe randuri
    float sC = cols / REF;                 // scale pe coloane
    float s  = 0.5f * (sR + sC);           // scale mediu (pt raze)

    auto SR = [&](float r) { return (int)glm::round(r * sR); };
    auto SC = [&](float c) { return (int)glm::round(c * sC); };
    auto SS = [&](float x) { return x * s; };  // pentru raze / grosimi

    // Helper functions

    auto placeCircularMountain = [&](int centerRow, int centerCol, float radius) {
        for (int i = 0; i < rows; i++) {
            for (int j = 0; j < cols; j++) {
                float dx = (float)j - centerCol;
                float dy = (float)i - centerRow;
                float distance = sqrt(dx * dx + dy * dy);

                if (distance <= radius && terrain[i][j] != TileType::Water) {
                    terrain[i][j] = TileType::Mountain;
                }
            }
        }
    };

    auto placeHorizontalRiver = [&](int startRow, int startCol, int endCol, int thickness = 2) {
        thickness = glm::max(1, thickness);
        for (int j = startCol; j < endCol && j < cols; j++) {
       
            int waveOffset = (int)(sin(j * 0.3f) * 2); //creeaza wave uri

            for (int t = 0; t < thickness; t++) {
                int row = startRow + waveOffset + t - thickness / 2;
                if (row >= 0 && row < rows) {
                    terrain[row][j] = TileType::Water;
                }
            }
        }
    };

    auto placeVerticalRiver = [&](int startRow, int endRow, int startCol, int thickness = 2) {
        thickness = glm::max(1, thickness);
        for (int i = startRow; i < endRow && i < rows; i++) {
            int waveOffset = (int)(cos(i * 0.3f) * 2); //creeaza wave uri

            for (int t = 0; t < thickness; t++) {
                int col = startCol + waveOffset + t - thickness / 2;
                if (col >= 0 && col < cols) {
                    terrain[i][col] = TileType::Water;
                }
            }
        }
    };

    auto placeDiagonalRiver = [&](int startRow, int startCol, int endRow, int endCol, int thickness = 2) {
        thickness = glm::max(1, thickness);
        int steps = std::max(abs(endRow - startRow), abs(endCol - startCol));
        if (steps <= 0) return;

        for (int step = 0; step <= steps; step++) {
            float t = (float)step / steps;
            int row = (int)(startRow + t * (endRow - startRow));
            int col = (int)(startCol + t * (endCol - startCol));

            int waveOffset = (int)(sin(step * 0.2f) * 1.5f);

            for (int th = 0; th < thickness; th++) {
                int r = row + th - thickness / 2 + waveOffset;
                int c = col + th - thickness / 2;

                if (r >= 0 && r < rows && c >= 0 && c < cols) {
                    terrain[r][c] = TileType::Water;
                }
            }
        }
    };

    //  CREARE ELEMENTE HARTA (dar scalate) 

    // RAU 1: rau orizontal in partea de sus (curge stanga -> dreapta)
    placeHorizontalRiver(SR(12), 0, cols, glm::max(1, (int)glm::round(3 * sR)));

    // RAU 2: rau vertical pe partea stanga (curge sus -> jos)
    placeVerticalRiver(0, rows, SC(8), glm::max(1, (int)glm::round(3 * sC)));

    // GRUP MUNTI 1: formatiune circulara mare in stanga-sus
    placeCircularMountain(SR(8),  SC(18), SS(5.5f));

    // GRUP MUNTI 2: formatiune circulara medie in dreapta-jos
    placeCircularMountain(SR(35), SC(38), SS(4.0f));

    // GRUP MUNTI 3: formatiune circulara mica in centru
    placeCircularMountain(SR(25), SC(25), SS(3.5f));

    // GRUP MUNTI 4: alta formatiune in stanga-jos
    placeCircularMountain(SR(40), SC(12), SS(3.0f));

    placeCircularMountain(SR(18), SC(42), SS(2.5f));
    placeCircularMountain(SR(42), SC(30), SS(2.0f));
}

void Tema2::RenderTerrain()
{
    float y = 0.0f;                         
    float halfW = cols * tileSize / 2.0f;
    float halfH = rows * tileSize / 2.0f;

    for (int i = 0; i < rows; i++) {
        for (int j = 0; j < cols; j++) {
            glm::mat4 model = glm::mat4(1);

            // pozitionam tile-ul astfel incat harta sa fie centrată în jurul originii
            float xPos = j * tileSize - halfW + tileSize / 2.0f;
            float zPos = i * tileSize - halfH + tileSize / 2.0f;

            model *= transform3D::Translate(xPos, y, zPos);
            model *= transform3D::Scale(tileSize, 0.05f, tileSize);   // foarte subtire pe Y

            switch (terrain[i][j]) {
            case TileType::Plain:
                RenderMesh(meshes["tile_plain"], shaders["VertexColor"], model);
                break;
            case TileType::Water:
                RenderMesh(meshes["tile_water"], shaders["VertexColor"], model);
                break;
            case TileType::Mountain:
                RenderMesh(meshes["tile_mountain"], shaders["VertexColor"], model);
                break;
            }
        }
    }
}

void Tema2::BuildRailRoad()
{
    // Curata lista existenta
    Rail* current = railHead;
    while (current != nullptr) {
        Rail* next = current->next;
        delete current;
        current = next;
    }
    railHead = nullptr;
    Rail* tail = nullptr;

    float halfW = cols * tileSize / 2.0f;
    float halfH = rows * tileSize / 2.0f;

    auto TileCenter = [&](int i, int j) -> glm::vec3 {
        float x = j * tileSize - halfW + tileSize * 0.5f;
        float z = i * tileSize - halfH + tileSize * 0.5f;
        return glm::vec3(x, 0.0f, z);
    };

    auto WorldToTile = [&](const glm::vec3& p, int& i, int& j) {
    
    float jf = (p.x + halfW) / tileSize;
    float if_ = (p.z + halfH) / tileSize;

    j = (int)floor(jf);
    i = (int)floor(if_);

    i = glm::clamp(i, 0, rows - 1);
    j = glm::clamp(j, 0, cols - 1);
    };


    glm::vec3 dir(1.0f, 0.0f, 0.0f);

    auto RailTypeAt = [&](const glm::vec3& p) -> RailType {
        int i, j;
        WorldToTile(p, i, j);

        if (terrain[i][j] == TileType::Water)    return RailType::Bridge;
        if (terrain[i][j] == TileType::Mountain) return RailType::Tunnel;
        return RailType::Surface;
    };


    auto addOne = [&](const glm::vec3& start, const glm::vec3& end, RailType railType) {
        Rail* newRail = new Rail(start, end, railType);
        if (!railHead) { railHead = newRail; tail = newRail; }
        else { tail->next = newRail; newRail->prev = tail; tail = newRail; }
    };

    auto addSegment = [&](glm::vec3& pos, int count) {
        for (int k = 0; k < count; k++) {
            glm::vec3 start = pos;
            glm::vec3 end   = pos + dir * tileSize;
            glm::vec3 mid   = (start + end) * 0.5f;

            // puncte sigur în interiorul tile-urilor
            glm::vec3 sampleA = start + dir * (tileSize * 0.25f);
            glm::vec3 sampleB = end   - dir * (tileSize * 0.25f);

            RailType tA = RailTypeAt(sampleA);
            RailType tB = RailTypeAt(sampleB);

            addOne(start, mid, tA);
            addOne(mid,   end, tB);

            pos = end;
        }
    };

    auto turnRight = [&]() {
        dir = glm::vec3(-dir.z, 0.0f, dir.x);
    };

    auto turnLeft = [&]() {
        dir = glm::vec3(dir.z, 0.0f, -dir.x);
    };

    //  Alege un dreptunghi in interior, PE TILE-URI 
    int margin = 3;                 // tile-uri margine
    int minI = margin;
    int minJ = margin;
    int maxI = rows - 1 - margin;
    int maxJ = cols - 1 - margin;

    int stepsX = (maxJ - minJ);     // cati pasi de la minJ la maxJ
    int stepsZ = (maxI - minI);

    glm::vec3 pos = TileCenter(minI, minJ);
    dir = glm::vec3(1, 0, 0);

    // --- deviere prin interior pe latura dreaptă ---
    int z1 = stepsZ / 3;                 // cobori putin pe dreapta
    int z2 = stepsZ / 3;                 // inca putin dupa ce intri in interior
    int z3 = stepsZ - z1 - z2;           // restul pana jos
    int xIn = stepsX / 2;                // cat intri in interior
    addSegment(pos, stepsX);             // sus: st -> dr
    turnRight();                         // spre jos (+Z)
    addSegment(pos, z1);                 // coboara putin pe margine

    turnRight();                         // acum mergi spre stanga (-X)
    addSegment(pos, xIn);                // intra in interior

    // "turnLeft" = 3 x turnRight (ca să cobori iar)
    turnRight(); turnRight(); turnRight();
    addSegment(pos, z2);                 // coboara in interior
    // iar "turnLeft" ca sa mergi spre dreapta si sa revii pe margine
    turnRight(); turnRight(); turnRight();
    addSegment(pos, xIn);                // revii la marginea dreapta

    turnRight();                         // spre jos (+Z)
    addSegment(pos, z3);                 // pana jos

    turnRight();                         // spre stanga (-X)
    addSegment(pos, stepsX);             // jos: dr -> st

    turnRight();                         // spre sus (-Z)
    addSegment(pos, stepsZ);             // urca pe stanga pana sus (inchizi bucla)
    // inchide bucla
    if (tail && railHead) tail->next = railHead, railHead->prev = tail;
    ComputeRailOrientation();
    
    {
        float p = 0.08f; // 8% avariate (ajusteaza)
        Rail* r = railHead;
        Rail* first = railHead;
        int guard = 0;

    }
}

void Tema2::RenderRails()
{
    if (railHead == nullptr) return;
    
    Rail* current = railHead;
    Rail* first = railHead;
    int maxSegments = 1000;  // Previne bucle infinite
    int count = 0;
    
    do {
        RenderRailSegment(current);
        current = current->next;
        count++;
    } while (current != nullptr && current != first && count < maxSegments);
}

void Tema2::RenderRailSegment(Rail* r)
{
    if (!r) return;

    glm::vec3 a = r->start;        // punctul de inceput al segmentului
    glm::vec3 b = r->end;          // punctul de sfarsit al segmentului

    glm::vec3 d = b - a;           // directia segmentului
    d.y = 0.0f;                    // proiecteaza pe planul XZ
    float len = glm::length(d);    // lungimea segmentului
    if (len < 1e-6f) return;       // ignora segmentele prea scurte

    float angleY = atan2(d.z, d.x); // unghi de rotatie pe OY

    // calculeaza inaltimea sinei deasupra terenului
    float terrainTop = 0.05f * 0.5f;
    float railHeight = 0.02f;
    float railY = terrainTop + railHeight * 0.5f + 0.001f;

    // suprapunere pentru a evita gapuri intre segmente (mai mult daca e avariat)
    float overlap = (r->damaged ? 0.02f : 0.06f) * len;

    glm::vec3 center = (a + b) * 0.5f; // centrul segmentului

    // construieste matricea de transformare
    glm::mat4 model(1.0f);
    model *= transform3D::Translate(center.x, railY, center.z);
    model *= transform3D::RotateOY(angleY);
    model *= transform3D::Scale(len + overlap, railHeight, tileSize);

    // alege mesh-ul in functie de tipul sinei
    const char* meshName = "rail_piece";
    if (r->type == RailType::Bridge) meshName = "rail_bridge";
    else if (r->type == RailType::Tunnel) meshName = "rail_tunnel";

    // activeaza deformare doar pe sine de suprafata (nu pe poduri/tuneluri)
    int damaged = (r->damaged && r->type == RailType::Surface) ? 1 : 0;

    // incearca sa obtii shader-ul de deformare
    Shader* sh = nullptr;
    if (shaders.count("RailDeform")) sh = shaders["RailDeform"];

    // fallback: daca nu exista shader-ul, foloseste shader-ul default
    if (!sh) {
        RenderMesh(meshes[meshName], shaders["VertexColor"], model);
        return;
    }

    sh->Use();

    // trimite uniform-urile catre shader
    glUniform1i(glGetUniformLocation(sh->program, "u_damaged"), damaged);
    glUniform1f(glGetUniformLocation(sh->program, "u_seed"), r->seed);
    glUniform1f(glGetUniformLocation(sh->program, "u_pulseAmp"), 0.35f);

    // timp pentru animatii (flicker / zgomot animat)
    glUniform1f(glGetUniformLocation(sh->program, "u_time"), tAnim);

    // parametri de deformare pentru sina avariata
    glUniform1f(glGetUniformLocation(sh->program, "u_dipAmp"),  0.06f * tileSize);  // adancime inclinare
    glUniform1f(glGetUniformLocation(sh->program, "u_sideAmp"), 0.05f * tileSize);  // deplasare laterala
    glUniform1f(glGetUniformLocation(sh->program, "u_tiltAmp"), glm::radians(9.0f)); // unghiul de inclinare

    RenderMesh(meshes[meshName], sh, model);
}

void Tema2::PickNewTrainDestination()
{
    if (stations.empty()) {
        trainDestStation = -1;
        destPulseActive = false;
        return;
    }

    int newDest = rand() % (int)stations.size();

    if (stations.size() > 1) {
        int guard = 0;
        while (newDest == trainDestStation && guard++ < 20) {
            newDest = rand() % (int)stations.size();
        }
    }

    trainDestStation = newDest;
    destPulseActive = true;
}

void Tema2::TryRepairNearbyRail()
{
    if (!railHead) return;

    Rail* best = nullptr;
    float bestDist = 1e9f;

    // cauta segmentul avariat cel mai apropiat de drezina
    Rail* curr = railHead;
    do {
        if (curr->damaged) {
            float d = DistPointToSegmentXZ(drezina.position, curr->start, curr->end);
            if (d < bestDist) {
                bestDist = d;
                best = curr;
            }
        }
        curr = curr->next;
    } while (curr && curr != railHead);

    if (!best) return;                    // nu exista nimic avariat
    if (bestDist > repairRadius) return;  // prea departe pentru a putea repara

    // Repara instant segmentul gasit
    best->damaged = false;
    best->damageTime = 0.0f;

    // Daca folosesti "tile = 2 jumatati", repara si vecinul care share-uieste capatul
    const float EPS = 1e-4f;

    // vecin inainte (urmatorul segment pe traseu)
    if (best->next && best->next->damaged) {
        if (glm::length(best->end - best->next->start) < EPS) {
            best->next->damaged = false;
            best->next->damageTime = 0.0f;
        }
    }
    // vecin inapoi (segmentul anterior pe traseu)
    if (best->prev && best->prev->damaged) {
        if (glm::length(best->start - best->prev->end) < EPS) {
            best->prev->damaged = false;
            best->prev->damageTime = 0.0f;
        }
    }
}

bool Tema2::AnyDamagedRails() const
{
    if (!railHead) return false;

    const Rail* curr = railHead;
    do {
        if (curr->damaged) return true;
        curr = curr->next;
    } while (curr && curr != railHead);

    return false;
}

void Tema2::UpdateRailDamage(float dt)
{
    if (!railHead) return;

    Rail* r = railHead;
    do {
        if (r->damaged && r->damageTime > 0.0f) {
            r->damageTime -= dt;

            if (r->damageTime <= 0.0f) {
                r->damageTime = 0.0f;
                r->damaged = false;   // reparat
            }
        }
        r = r->next;
    } while (r && r != railHead);
}

void Tema2::UpdateTrain(float dt)
{
    if (!train.rail) return;

    // 0) Stationare in statii (nu e asteptare din cauza avariei)
    if (trainStopped) {
        trainWaitTime = 0.0f;

        trainStopTimer -= dt;
        if (trainStopTimer <= 0.0f) {
            trainStopped = false;
            trainStopTimer = 0.0f;
            PickNewTrainDestination();
        }
        return;
    }

    const float stopDist = 0.20f * tileSize;      // cat inaintea unei avarii trebuie sa se opreasca
    float distToTravel   = dt * train.speed;      // viteza = unitati world / sec

    bool blockedByDamage = false;

    // Daca e deja pe un segment avariat -> blocheaza miscarea
    if (train.rail->damaged) {
        blockedByDamage = true;
        trainWaitTime += dt;
        distToTravel = 0.0f;
    }

    int guard = 0;
    const int maxSteps = 2000;

    while (distToTravel > 1e-6f && guard++ < maxSteps && !blockedByDamage) {

        Rail* curr = train.rail;
        Rail* next = curr->next ? curr->next : railHead;
        if (!next) break;

        glm::vec3 v = curr->end - curr->start;
        v.y = 0.0f;
        float segLen = glm::length(v);
        if (segLen < 1e-6f) break;

        // distanta ramasa pana la capatul segmentului curent
        float distLeft = (1.0f - train.progress) * segLen;

        // Daca segmentul urmator e avariat -> calculeaza punctul de oprire pe segment curent
        if (next->damaged) {
            float stopAt = segLen - stopDist;              // distanta de la inceput
            stopAt = glm::clamp(stopAt, 0.0f, segLen);     // verifica limitele
            float targetProg = stopAt / segLen;

            // cat mai ai pana la punctul de oprire
            float targetDelta = (targetProg - train.progress) * segLen;

            // daca inca nu ai ajuns, misca-te pana acolo
            if (targetDelta > 1e-6f) {
                float move = glm::min(distToTravel, targetDelta);
                train.progress += move / segLen;
                distToTravel  -= move;
            }

            // ai ajuns la punct de oprire -> blocaj
            blockedByDamage = true;
            trainWaitTime += dt;
            break;
        }

        // N-ai avarie in fata -> miscarea normala
        if (distToTravel < distLeft) {
            train.progress += distToTravel / segLen;
            distToTravel = 0.0f;
        } else {
            // treci pe segmentul urmator
            distToTravel -= distLeft;
            train.rail = next;
            train.progress = 0.0f;

            // daca ai intrat pe un segment avariat
            if (train.rail->damaged) {
                blockedByDamage = true;
                trainWaitTime += dt;
                break;
            }
        }
    }

    // daca NU e blocat de avarie -> resetare timp asteptare
    if (!blockedByDamage) {
        trainWaitTime = 0.0f;
    }

    // actualizeaza pozitia si yaw din rail + progress
    glm::vec3 start = train.rail->start;
    glm::vec3 end   = train.rail->end;

    train.position = glm::mix(start, end, train.progress);

    glm::vec3 d = end - start;
    d.y = 0.0f;
    if (glm::length(d) > 1e-6f) {
        train.forward = glm::normalize(d);
        float yaw = atan2(train.forward.z, train.forward.x);

        glm::vec3 meshForward = glm::normalize(glm::vec3(cos(yaw), 0.0f, sin(yaw)));
        if (glm::dot(meshForward, train.forward) < 0.0f) yaw += glm::pi<float>();

        train.yaw = yaw;
    }

    // Detectie sosire in statia de destinatie
    if (!blockedByDamage && trainDestStation >= 0 && trainDestStation < (int)stations.size()) {
        glm::vec3 sp = stations[trainDestStation].position;
        float dx = train.position.x - sp.x;
        float dz = train.position.z - sp.z;
        float dist = sqrt(dx * dx + dz * dz);

        const float arriveDist = tileSize * 1.15f;
        if (dist < arriveDist) {
            trainStopped = true;
            trainStopTimer = trainStopDuration;
            destPulseActive = false;
        }
    }
}

void Tema2::RenderDrezina(const glm::vec3& position, float yaw)
{
    const float modelYawOffset = 0.0f;

    // baza: pozitie + orientare + scalare (in tile)
    glm::mat4 base = glm::mat4(1.0f);
    base *= transform3D::Translate(position.x, position.y, position.z);
    base *= transform3D::RotateOY(yaw + modelYawOffset);
    base *= transform3D::Scale(drezinaScaleX, drezinaScaleY, drezinaScaleZ);

    // 1) body static
    RenderMesh(meshes["drezina_body"], shaders["VertexColor"], base);

    // 2) bara animata
    // pivot la stemTop (fix ca in constructia mesh-ului)
    const float baseH = 0.4f;
    const float postH = 0.7f;
    const float stemH = 0.55f;
    const float pivotY = baseH + postH + stemH;   

    float angle = glm::radians(10.0f) * sin(tAnim * 2.5f); // amplitudine + viteza

    glm::mat4 barM = base;
    barM *= transform3D::Translate(0.0f, pivotY, 0.0f);
    barM *= transform3D::RotateOZ(angle);     // “sus-jos” din bara orizontala gri (un capat sus, celalalt jos)
    barM *= transform3D::Translate(0.0f, -pivotY, 0.0f);

    RenderMesh(meshes["drezina_bar"], shaders["VertexColor"], barM);
}

void Tema2::FrameStart()
{
    // Clears the color buffer (using the previously set color) and depth buffer
    glClearColor(0.53f, 0.81f, 0.92f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
}

void Tema2::RenderScene()
{
    RenderTerrain();
    RenderRails();
    RenderStations(); 
    RenderDrezina(drezina.position, drezina.yaw);
    RenderTrain(train.position, train.yaw);
}

void Tema2::UpdateTopDownCamera()
{
    auto cam = GetSceneCamera();
    if (!cam) return;

    // (A) top-down pe toata harta
    glm::vec3 target(0.0f, 0.0f, 0.0f);

    float camH = 30.0f; // inaltime mare pe OY
    glm::vec3 camPos = target + glm::vec3(0.0f, camH, 0.0f);

    // directia e in jos; up NU poate fi (0,1,0) cand privesti fix in jos
    glm::vec3 dir = glm::normalize(target - camPos);   // (0,-1,0)
    glm::vec3 up  = glm::vec3(0, 0, -1);              // important

    glm::quat q = glm::quatLookAt(dir, up);
    cam->SetPositionAndRotation(camPos, q);
    cam->Update();
}

void Tema2::UpdateTPSCameraFollow(const glm::vec3& targetPos, float targetYaw,
                                  float dist, float height, float lookH)
{
    auto cam = GetSceneCamera();
    if (!cam) return;

    glm::vec3 forward = glm::normalize(glm::vec3(cos(targetYaw), 0.0f, sin(targetYaw)));
    glm::vec3 up(0, 1, 0);

    glm::vec3 camPos = targetPos - forward * dist + up * height;
    glm::vec3 lookAt = targetPos + up * lookH;

    glm::vec3 dir = glm::normalize(lookAt - camPos);
    glm::quat q = glm::quatLookAt(dir, up);

    cam->SetPositionAndRotation(camPos, q);
    cam->Update();
}

void Tema2::UpdateTPSCamera()
{
    auto cam = GetSceneCamera();
    if (!cam) return;

    // forward din yaw (în plan XZ)
    glm::vec3 forward = glm::normalize(glm::vec3(cos(drezina.yaw), 0.0f, sin(drezina.yaw)));
    glm::vec3 up(0, 1, 0);

    // pozitia camerei: in spate + sus
    glm::vec3 camPos = drezina.position - forward * tpsDist + up * tpsHeight;

    // tinta: drezina (usor mai sus)
    glm::vec3 target = drezina.position + up * lookHeight;

    glm::vec3 dir = glm::normalize(target - camPos);

    // orientare (look-at) prin quaternion
    glm::quat q = glm::quatLookAt(dir, up);

    cam->SetPositionAndRotation(camPos, q);
    cam->Update();
}

void Tema2::SpawnOneDamagedTile()
{
    if (!railHead) return;

    // colecteaza toate segmentele sinelor intr-un vector
    std::vector<Rail*> rails;
    Rail* curr = railHead;
    do {
        rails.push_back(curr);
        curr = curr->next;
    } while (curr && curr != railHead);

    if ((int)rails.size() < 2) return;

    // numarul de perechi de segmente (fiecare avarie = 2 segmente)
    int maxPairs = (int)rails.size() / 2;
    const float EPS = 1e-4f;

    // incearca sa gasesti o pereche valida pentru a dauna
    for (int tries = 0; tries < 100; tries++) {
        int pairIndex = rand() % maxPairs;
        int k = pairIndex * 2;

        Rail* r0 = rails[k];
        Rail* r1 = rails[(k + 1) % rails.size()];

        // verifica daca segmentele nu sunt deja avariate
        if (r0->damaged || r1->damaged) continue;

        if (r0->type != RailType::Surface || r1->type != RailType::Surface) continue;
        // verifica daca segmentele sunt conectate (capatul unuia = inceput altuia)
        if (glm::length(r0->end - r1->start) > EPS) continue;

        // siguranta: nu spawna avaria prea aproape de tren
        const float safeDist = tileSize * 1.2f;
        float d0 = DistPointToSegmentXZ(train.position, r0->start, r0->end);
        float d1 = DistPointToSegmentXZ(train.position, r1->start, r1->end);
        if (d0 < safeDist || d1 < safeDist) continue;

        // nu spawna pe segmentul curent al trenului
        if (train.rail == r0 || train.rail == r1) continue;
        // nu spawna pe urmatorul segment dupa tren
        if (train.rail && (train.rail->next == r0 || train.rail->next == r1)) continue;

        // OK -> aplica avaria pe ambele segmente
        r0->damaged = true; r0->damageTime = -1.0f;
        r1->damaged = true; r1->damageTime = -1.0f;

        // genereaza seede diferite pentru flicker-ul vizual
        float s = (float)rand() / (float)RAND_MAX * 100.0f;
        r0->seed = s;
        r1->seed = s + 13.37f;
        return;
    }
}

void Tema2::Update(float deltaTimeSeconds)
{
    glLineWidth(3);
    glPointSize(5);
    glPolygonMode(GL_FRONT_AND_BACK, polygonMode);

    glm::ivec2 resolution = window->GetResolution();
    resolution /= 2;                 
    glViewport(0, 0, resolution.x, resolution.y);

    tAnim += deltaTimeSeconds;

    // daca e game over: curata si afiseaza doar UI (overlay)
    if (gameOver) {
        glClearColor(0, 0, 0, 1);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        RenderUI();      
        return;
    }

    // joc normal 
    gameTime += deltaTimeSeconds;

    // spawn avarii
    damageSpawnTimer += deltaTimeSeconds;
    if (damageSpawnTimer >= damageSpawnInterval) {
        damageSpawnTimer = 0.0f;
        SpawnOneDamagedTile();
    }

    // repair hold / tren etc.
    UpdateRepairHold(deltaTimeSeconds);
    UpdateTrain(deltaTimeSeconds);

    // verific endgame dupa update
    CheckEndGame();

    // daca a devenit game over chiar acum, iesi (si va randa overlay next frame)
    if (gameOver) {
        cout << "Game Over triggered in Update()\n";
    }

    // camera + randare normala 
    projectionMatrix = glm::perspective(glm::radians(60.0f),
        window->props.aspectRatio, 0.01f, 200.0f);

    bool wsHeld = window->KeyHold(GLFW_KEY_W) || window->KeyHold(GLFW_KEY_S);

    // cand repar (tin F si am target valid) -> camera pe drezina
    bool repairingNow = window->KeyHold(GLFW_KEY_F) && (repairTarget != nullptr);

    // prioritate: F (repair) > W/S > tren
    if (repairingNow || wsHeld) {
        UpdateTPSCameraFollow(drezina.position, drezina.yaw, tpsDist, tpsHeight, lookHeight);
    } else {
        UpdateTPSCameraFollow(train.position, train.yaw, trainCamDist, trainCamHeight, trainLookH);
    }

    bool mainWasTPS = true;
    glm::mat4 mainProj = projectionMatrix;

    RenderScene();
    RenderRepairBar2D();                 
    RenderMinimap(mainWasTPS, mainProj); 
    RenderUI();
}

void Tema2::FrameEnd()
{
}

void Tema2::OnInputUpdate(float deltaTime, int mods)
{
    if (!drezinaRail) return;

    bool W = window->KeyHold(GLFW_KEY_W);
    bool S = window->KeyHold(GLFW_KEY_S);

    // doar W/S conteaza
    float input = 0.0f;
    if (W) input += 1.0f;
    if (S) input -= 1.0f;

    if (fabs(input) < 1e-6f) {
        // stau pe loc, dar raman fix pe sina
        drezina.position = glm::mix(drezinaRail->start, drezinaRail->end, drezinaT);
        drezina.position.y = 0.0f;
        return;
    }

    // lungimea segmentului curent
    glm::vec3 seg = drezinaRail->end - drezinaRail->start;
    seg.y = 0.0f;
    float segLen = glm::length(seg);
    if (segLen < 1e-6f) return;

    // cat avansez ca fractie din segment
    float dtT = (drezinaSpeed * deltaTime) / segLen;
    drezinaT += input * dtT;

    // treceri intre segmente (bucla)
    while (drezinaT > 1.0f) {
        drezinaT -= 1.0f;
        drezinaRail = drezinaRail->next ? drezinaRail->next : railHead;
    }
    while (drezinaT < 0.0f) {
        drezinaT += 1.0f;
        drezinaRail = drezinaRail->prev ? drezinaRail->prev : railHead;
    }

    // pozitia pe sina
    drezina.position = glm::mix(drezinaRail->start, drezinaRail->end, drezinaT);
    drezina.position.y = 0.0f;

    // yaw: orientata in directia de mers (W inainte, S invers)
    glm::vec3 dir = drezinaRail->end - drezinaRail->start;
    dir.y = 0.0f;
    if (glm::length(dir) > 1e-6f) {
        dir = glm::normalize(dir);
        float yawForward = atan2(dir.z, dir.x);
        drezina.yaw = (input > 0.0f) ? yawForward : (yawForward + glm::pi<float>());
    }
}

void Tema2::OnKeyPress(int key, int mods)
{


}

void Tema2::OnKeyRelease(int key, int mods)
{
    // Add key release event
}

void Tema2::OnMouseMove(int mouseX, int mouseY, int deltaX, int deltaY)
{
    // Add mouse move event
}

void Tema2::OnMouseBtnPress(int mouseX, int mouseY, int button, int mods)
{
    // Add mouse button press event
}

void Tema2::OnMouseBtnRelease(int mouseX, int mouseY, int button, int mods)
{
    // Add mouse button release event
}

void Tema2::OnMouseScroll(int mouseX, int mouseY, int offsetX, int offsetY)
{
}

void Tema2::OnWindowResize(int width, int height)
{
}
