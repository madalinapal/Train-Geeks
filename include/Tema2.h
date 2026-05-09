#pragma once

#include <vector>             
#include "components/simple_scene.h"
#include "components/text_renderer.h"
#include <string>
#include <cstdlib>

namespace m1
{
    // Tipurile de teren 
    enum class TileType {
        Plain,      
        Water,      
        Mountain    
    };

    enum class RailType {
    Surface,    
    Bridge,     
    Tunnel      
    };

    enum class TurnDir { None, Left, Right };

    struct Rail {
    glm::vec3 start, end;
    RailType type;
    Rail* next = nullptr;
    Rail* prev = nullptr;

    glm::vec3 dir = glm::vec3(1, 0, 0);
    float yaw = 0.0f;                   // unghi pe OY, in radiani
    TurnDir turnIn  = TurnDir::None;    // cum intri în segmentul asta (fata de prev)
    TurnDir turnOut = TurnDir::None;    // cum iesi din segmentul asta (fata de next)

    bool damaged = false;
    float damageTime = 0.0f;   // cat timp ramane avariat
    float seed = 0.0f; 

    Rail(const glm::vec3& s, const glm::vec3& e, RailType t)
        : start(s), end(e), type(t)
        {
            seed = (float)rand() / (float)RAND_MAX * 100.0f; 
        }

    };

    struct TrackSegment {
        glm::vec3 center;
        float rotY;
    };

    struct Train {
        Rail* rail;      // rail-ul curent
        float progress;  // 0 .. 1 pe segment
        float speed;     // unitati / secunda (sau doar factor)
        glm::vec3 position;
        float yaw = 0.0f;
        glm::vec3 forward = glm::vec3(1,0,0);

    };

    struct Station {
        glm::vec3 position;
        glm::vec3 color;
        float radius;
        std::string name;
    };
    
    struct Drezina {
    Rail* rail = nullptr;
    float progress = 0.0f;
    float speed = 0.6f;
    glm::vec3 position = glm::vec3(0);
    glm::vec3 forward = glm::vec3(1,0,0);
    float yaw = 0.0f;
    };

    class Tema2 : public gfxc::SimpleScene
    {
     public:
        struct ViewportArea
        {
            ViewportArea() : x(0), y(0), width(1), height(1) {}
            ViewportArea(int x, int y, int width, int height)
                : x(x), y(y), width(width), height(height) {}
            int x;
            int y;
            int width;
            int height;
        };

        Tema2();
        ~Tema2();

        void Init() override;

     private:
        void FrameStart() override;
        void Update(float deltaTimeSeconds) override;
        void FrameEnd() override;

        void OnInputUpdate(float deltaTime, int mods) override;
        void OnKeyPress(int key, int mods) override;
        void OnKeyRelease(int key, int mods) override;
        void OnMouseMove(int mouseX, int mouseY, int deltaX, int deltaY) override;
        void OnMouseBtnPress(int mouseX, int mouseY, int button, int mods) override;
        void OnMouseBtnRelease(int mouseX, int mouseY, int button, int mods) override;
        void OnMouseScroll(int mouseX, int mouseY, int offsetX, int offsetY) override;
        void OnWindowResize(int width, int height) override;

        void ComputeRailOrientation();
        void InitTrack();
        void RenderTrack();
        void RenderScene();

        // functii pentru teren ---
        void GenerateTerrain();
        void RenderTerrain();

        void RenderTrain(const glm::vec3& position, float yaw);
        void RenderDrezina(const glm::vec3& position, float yaw);
        
        void CreateStations();
        void RenderStations();

        void UpdateTPSCamera();
        void UpdateTopDownCamera();
        void UpdateTPSCameraFollow(const glm::vec3& targetPos, float targetYaw,
                                float dist, float height, float lookH);

        void BuildRailRoad();
        void RenderRails();
        void RenderRailSegment(Rail* r);
        void UpdateTrain(float deltaTime);

        void UpdateRailDamage(float dt);
        bool AnyDamagedRails() const;
        void SpawnOneDamagedTile();
        void TryRepairNearbyRail();
        void PickNewTrainDestination();
        void RenderRepairBar2D();

        // helpers
        int CountDamagedRails() const;
        Rail* FindClosestDamagedRail(float& outDist) const;
        std::string FormatTimeMMSS(float sec) const;
        void RenderUI();
        void UpdateRepairHold(float dt);
        void CheckEndGame();

        void GetMinimapBounds(float& minX, float& maxX, float& minZ, float& maxZ) const;
        void UpdateTopDownCameraAt(const glm::vec3& target, float height);
        void RenderMinimap(bool mainWasTPS, const glm::mat4& mainProj);


     protected:
        glm::mat4 modelMatrix;
        glm::mat4 projectionMatrix;
        glm::mat4 viewMatrix;
        float translateX, translateY, translateZ;
        float scaleX, scaleY, scaleZ;
        float angularStepOX, angularStepOY, angularStepOZ;
        GLenum polygonMode;
        ViewportArea miniViewportArea;

        // datele pentru harta / teren 
        std::vector<std::vector<TileType>> terrain;
        int rows, cols;
        float tileSize;

            // sine & tren ---
        Rail* railHead;   // inceputul listei de sine   
        Train train;
        
        float trainScaleX = 1.0f;
        float trainScaleY = 1.0f;
        float trainScaleZ = 1.0f;

        float drezinaScaleX = 1.0f;
        float drezinaScaleY = 1.0f;
        float drezinaScaleZ = 1.0f;

        float tAnim = 0.0f; // timp animatie general

        std::vector<TrackSegment> track;  // segmente pentru desenat sine
        std::vector<Station> stations;      // statiile de pe traseu
        Drezina drezina;

        float halfW; // jumate latime teren
        float halfH; // jumate inaltime teren

        // Camera TPS
        bool tpsEnabled = true;
        float tpsDist   = 6.0f;   // cat de in spate
        float tpsHeight = 4.0f;   // cat de sus
        float lookHeight = 1.0f;  // unde “se uita” pe drezina (putin deasupra solului)

        // damage spawn control
        float damageSpawnTimer = 0.0f;
        float damageSpawnInterval   = 20.0f;   // 1 avarie la 20 sec
        float damageDuration   = 8.0f;    // cat sta avariat 
        bool  allowOnlyOneDamagedTile = true;

        Rail* drezinaRail = nullptr;   // segmentul curent pe care se afla drezina
        float drezinaT = 0.0f;         // progres pe segment (0..1)
        float drezinaSpeed = 2.0f;     // unitati / secunda
        int trainDestStation = -1;     // 0..stations.size()-1
        bool destPulseActive = false;

        float destPulseAmp   = 0.25f;  // 25% marire (ajusteaza)
        float destPulseSpeed = 6.0f;   // viteza puls (ajusteaza)

        bool  trainStopped = false;
        float trainStopTimer = 0.0f;
        float trainStopDuration = 2.0f; // cat sta in statie

        // ---- UI / game state ----
        gfxc::TextRenderer* TextRenderer = nullptr;
        unsigned int fontSize = 22;

        float gameTime = 0.0f;                 // timp total joc (sec)
        bool gameOver = false;
        std::string endGameReason;

        // repair system
        float repairRadius = 0.9f;             
        float repairDuration = 4.0f;           // secunde necesare ca să se repare (bara)
        Rail* repairTarget = nullptr;
        float repairProgress = 0.0f;           // 0..1

        // limite pt endgame
        int maxDamagedAllowed = 10;
        float maxTrainWaitAllowed = 20.0f;

        float trainWaitTime = 0.0f;            // cat sta blocat in fata unei avarii

        enum class FollowTarget { Train, Drezina };
        FollowTarget followTarget = FollowTarget::Train;

        // parametri camera  pt tren
        float trainCamDist   = 7.0f;
        float trainCamHeight = 4.0f;
        float trainLookH     = 1.2f;


    };
}
