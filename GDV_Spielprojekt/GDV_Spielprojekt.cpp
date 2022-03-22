
#include "yoshix_fix_function.h"

#include <math.h>
#include <windows.h>
#include <iostream>
#include <ctime>

using namespace gfx;

namespace
{
    double    g_Frequency;
    long long g_StartTick;
} // namespace

namespace
{
    void GetFrequency()
    {
        long long Frequency;

        ::QueryPerformanceFrequency(reinterpret_cast<LARGE_INTEGER*>(&Frequency));

        g_Frequency = static_cast<double>(Frequency);
    }

    // -----------------------------------------------------------------------------

    void StartTime()
    {
        ::QueryPerformanceCounter(reinterpret_cast<LARGE_INTEGER*>(&g_StartTick));
    }

    // -----------------------------------------------------------------------------

    double GetTimeInSeconds()
    {
        long long CurrentRealTimeTick;

        ::QueryPerformanceCounter(reinterpret_cast<LARGE_INTEGER*>(&CurrentRealTimeTick));

        return static_cast<double>(CurrentRealTimeTick - g_StartTick) / g_Frequency;
    }
} // namespace

namespace
{
    class CApplication : public IApplication
    {
    public:

        CApplication();
        virtual ~CApplication();

    private:

        float   m_FieldOfViewY;     // Vertical view angle of the camera.
        // -> Positions
        float lifeBarPosition = 0;
  
        //Meshes / Objects
        BHandle m_pCubeMesh;        // A pointer to a YoshiX mesh, which represents a single cube.
        BHandle m_pPyramidMesh;     // the frontarea of the rocket
        BHandle m_pEnemyMesh;       //used for the randomly spawned enemy 
        BHandle m_pRocketFrontMesh; //rocketFront
        BHandle m_pRocketBodyMesh;  //rocketBody
        BHandle m_pRocketWingsMesh;  //rocketBody

        BHandle m_pDroneBodyMeshForeground; //used for main body of three attack drones (unused)
        BHandle m_pDroneTailMeshForeground; //used for tail of three attack drones -> foreground because the backgrounds are darker

        BHandle m_pDroneBodyMeshBackground; //used for main body of three attack drones
        BHandle m_pDroneTailMeshBackground;

        BHandle m_pTriangleMesh;    // represents single Triangle
        BHandle m_pGroundCubeMesh;  // represents the ground mesh
        BHandle m_pGroaundBrownMesh;//represents the brown rock mesh cube
        BHandle m_pBackgroundMesh;  //Quader representing the background
        BHandle m_pHeartLifeBarMesh; // used for current level
        BHandle m_pFifthLevelMesh; // used to indicate the fifth level for readability
        

        //Textures
        BHandle m_pQuadTexture;         // BackgroundTexture
        BHandle m_pWallTexture;         // Attempt to texture the rocket
        BHandle m_pGroundTexture;       //Ground as the name implies
        BHandle m_pMountainTexture;       //For the upcoming mountains


        

    private:

        virtual bool InternOnStartup();
        virtual bool InternOnShutdown();
        virtual bool InternOnCreateMeshes();
        virtual bool InternOnReleaseMeshes();
        virtual bool InternOnResize(int _Width, int _Height);
        virtual bool InternOnUpdate();
        virtual bool InternOnFrame();
        virtual bool InternOnKeyEvent(unsigned int _Key, bool _IsKeyDown, bool _IsAltDown);
        virtual bool buildGround();
        virtual bool showThrusters();
        virtual bool shootProjectile();
        virtual bool spawnGroundObject();
        virtual bool spawnEnemy();
        virtual bool checkCollision();
        virtual bool moveBackground();
        virtual bool spawnEnemy_attackDrones();
        virtual bool drawLifeContainter(float _X, float _Y);
        virtual bool drawCurrentLevel(float _X, float _Y);
        virtual bool levelController();
        virtual bool InternOnCreateTextures();
        virtual bool InternOnReleaseTextures();
        virtual bool particleEffects();

    };
} // namespace

namespace
{
    CApplication::CApplication()
        : m_FieldOfViewY(60.0f)     // Set the vertical view angle of the camera to 60 degrees.
        , m_pCubeMesh(nullptr)
        , m_pTriangleMesh(nullptr)
        , m_pGroundCubeMesh(nullptr)
        , m_pGroaundBrownMesh(nullptr)
        , m_pQuadTexture(nullptr)
        , m_pWallTexture(nullptr)
        , m_pBackgroundMesh(nullptr)
        , m_pGroundTexture(nullptr)
        , m_pMountainTexture(nullptr)
        , m_pRocketBodyMesh(nullptr)
        , m_pRocketFrontMesh(nullptr)
        , m_pPyramidMesh(nullptr)
        , m_pRocketWingsMesh(nullptr)
        , m_pHeartLifeBarMesh(nullptr)
        , m_pFifthLevelMesh(nullptr)
        , m_pDroneBodyMeshForeground(nullptr)
        , m_pDroneTailMeshForeground(nullptr)
        , m_pDroneBodyMeshBackground(nullptr)
        , m_pDroneTailMeshBackground(nullptr)
        , m_pEnemyMesh(nullptr)
    {
    }

    // -----------------------------------------------------------------------------

    CApplication::~CApplication()
    {
    }

    // -----------------------------------------------------------------------------

    bool CApplication::InternOnStartup()
    {
        // -----------------------------------------------------------------------------
        // Define the background color of the window. Colors are always 4D tuples,
        // whereas the components of the tuple represent the red, green, blue, and alpha 
        // channel. The alpha channel indicates the transparency of the color. A value
        // of 1 means the color is completely opaque. A value of 0 means the color is
        // completely transparent. The channels red, green, and blue also are values
        // 0..1 with 0 indicating the minimum intensity and 1 the maximum intensity.
        // -----------------------------------------------------------------------------
        float ClearColor[4] = { 0.2f, 0.19f, 0.15f, 1.0f, };

        SetClearColor(ClearColor);

        return true;
    }

    bool CApplication::InternOnCreateTextures()
    {
        // -----------------------------------------------------------------------------
        // Load an image from the given path and create a YoshiX texture representing
        // the image.
        // -----------------------------------------------------------------------------
        CreateTexture("..\\data\\images\\background_star.dds", &m_pQuadTexture);
        CreateTexture("..\\data\\images\\wall.dds", &m_pWallTexture);
        CreateTexture("..\\data\\images\\seamless_dirt.dds", &m_pGroundTexture);
        CreateTexture("..\\data\\images\\mountainTexture.dds", &m_pMountainTexture);
        

        return true;
    }

    bool CApplication::InternOnReleaseTextures()
    {
        // -----------------------------------------------------------------------------
        // Important to release the texture again when the application is shut down.
        // -----------------------------------------------------------------------------
        ReleaseTexture(m_pQuadTexture);
        ReleaseTexture(m_pWallTexture); 
        ReleaseTexture(m_pGroundTexture);
        ReleaseTexture(m_pMountainTexture);

        return true;
    }

    // -----------------------------------------------------------------------------

    bool CApplication::InternOnShutdown()
    {
        return true;
    }

    // -----------------------------------------------------------------------------

    bool CApplication::InternOnCreateMeshes()
    {
        // -----------------------------------------------------------------------------
        // Define the vertices of the mesh and their attributes.
        // -----------------------------------------------------------------------------

        //Scheitelpunkte(x,y,z)
        static float s_TriangleVertices[][3] =
        {
            { -1.0f, -2.0f, 0.0f, },
            {  1.0f, -2.0f, 0.0f, },
            {  0.0f,  1.5f, 0.0f, },
        };

        static float s_TriangleColors[][4] =
        {
            { 1.0f, 0.0f, 0.0f, 1.0f, },        // Color of vertex 0.
            { 1.0f, 0.0f, 0.0f, 1.0f, },        // Color of vertex 1.
            { 0.3f, 0.0f, 0.0f, 1.0f, },        // Color of vertex 2.
        };

        static float s_TriangleTexCoords[][2] =
        {
            { 0.0f, 1.0f, },                    // Texture coordinate of vertex 0.
            { 1.0f, 1.0f, },                    // Texture coordinate of vertex 1.
            { 0.5f, 0.0f, },                    // Texture coordinate of vertex 2.
        };



        //jede Zeile ist ein Dreieck
        static int s_TriangleIndices[][3] =
        {
            { 0, 1, 2, },
        };

        SMeshInfo MeshInfo;

     
        
        //static Colors
        static const float groundGreen[4] = { 0.36f, 0.8f, 0.17f, 1.0f, };
        static const float groundDarkerGreen[4] = { 0.24f, 0.49f, 0.13f, 1.0f, };
        //

        static float s_QuadTexCoords[][2] =
        {
            { 0.0f, 1.0f, },                    // Texture coordinate of vertex 0.
            { 1.0f, 1.0f, },                    // Texture coordinate of vertex 1.
            { 1.0f, 0.0f, },                    // Texture coordinate of vertex 2.
            { 0.0f, 0.0f, },                    // Texture coordinate of vertex 3.
        };

    

        static const float s_HalfEdgeLength = 1.0f;

        static float s_CubeVertices[][3] =
        {
            { -s_HalfEdgeLength, -s_HalfEdgeLength, -s_HalfEdgeLength, }, //1
            {  s_HalfEdgeLength, -s_HalfEdgeLength, -s_HalfEdgeLength, }, //2
            {  s_HalfEdgeLength,  s_HalfEdgeLength, -s_HalfEdgeLength, }, //3
            { -s_HalfEdgeLength,  s_HalfEdgeLength, -s_HalfEdgeLength, }, //4

            {  s_HalfEdgeLength, -s_HalfEdgeLength, -s_HalfEdgeLength, }, //5
            {  s_HalfEdgeLength, -s_HalfEdgeLength,  s_HalfEdgeLength, }, //6
            {  s_HalfEdgeLength,  s_HalfEdgeLength,  s_HalfEdgeLength, }, //7
            {  s_HalfEdgeLength,  s_HalfEdgeLength, -s_HalfEdgeLength, }, //8
                                                                          
            {  s_HalfEdgeLength, -s_HalfEdgeLength,  s_HalfEdgeLength, }, //9
            { -s_HalfEdgeLength, -s_HalfEdgeLength,  s_HalfEdgeLength, }, //10
            { -s_HalfEdgeLength,  s_HalfEdgeLength,  s_HalfEdgeLength, }, //11
            {  s_HalfEdgeLength,  s_HalfEdgeLength,  s_HalfEdgeLength, }, //12

            { -s_HalfEdgeLength, -s_HalfEdgeLength,  s_HalfEdgeLength, }, //13      1 (in x-File)
            { -s_HalfEdgeLength, -s_HalfEdgeLength, -s_HalfEdgeLength, }, //14      2
            { -s_HalfEdgeLength,  s_HalfEdgeLength, -s_HalfEdgeLength, }, //15      3
            { -s_HalfEdgeLength,  s_HalfEdgeLength,  s_HalfEdgeLength, }, //16      4

            { -s_HalfEdgeLength,  s_HalfEdgeLength, -s_HalfEdgeLength, }, //17
            {  s_HalfEdgeLength,  s_HalfEdgeLength, -s_HalfEdgeLength, }, //18
            {  s_HalfEdgeLength,  s_HalfEdgeLength,  s_HalfEdgeLength, }, //19
            { -s_HalfEdgeLength,  s_HalfEdgeLength,  s_HalfEdgeLength, }, //20

            { -s_HalfEdgeLength, -s_HalfEdgeLength,  s_HalfEdgeLength, }, //21
            {  s_HalfEdgeLength, -s_HalfEdgeLength,  s_HalfEdgeLength, }, //22
            {  s_HalfEdgeLength, -s_HalfEdgeLength, -s_HalfEdgeLength, }, //23
            { -s_HalfEdgeLength, -s_HalfEdgeLength, -s_HalfEdgeLength, }, //24
        };

        static float s_CubeColors[][4] =
        {
            //front
            { 1.0f, 0.0f, 0.0f, 1.0f, },
            { 1.0f, 0.0f, 0.0f, 1.0f, },
            { 1.0f, 0.0f, 0.0f, 1.0f, },
            { 1.0f, 0.0f, 0.0f, 1.0f, },
            //rightside
            { 0.0f, 1.0f, 0.0f, 1.0f, },
            { 0.0f, 1.0f, 0.0f, 1.0f, },
            { 0.0f, 1.0f, 0.0f, 1.0f, },
            { 0.0f, 1.0f, 0.0f, 1.0f, },
            //upper side
            { 0.0f, 0.0f, 1.0f, 1.0f, },
            { 0.0f, 0.0f, 1.0f, 1.0f, },
            { 0.0f, 0.0f, 1.0f, 1.0f, },
            { 0.0f, 0.0f, 1.0f, 1.0f, },
            //
            { 1.0f, 1.0f, 0.0f, 1.0f, },
            { 1.0f, 1.0f, 0.0f, 1.0f, },
            { 1.0f, 1.0f, 0.0f, 1.0f, },
            { 1.0f, 1.0f, 0.0f, 1.0f, },
            //left side
            { 0.0f, 1.0f, 1.0f, 1.0f, },
            { 0.0f, 1.0f, 1.0f, 1.0f, },
            { 0.0f, 1.0f, 1.0f, 1.0f, },
            { 0.0f, 1.0f, 1.0f, 1.0f, },

            { 1.0f, 1.0f, 1.0f, 1.0f, },
            { 1.0f, 1.0f, 1.0f, 1.0f, },
            { 1.0f, 1.0f, 1.0f, 1.0f, },
            { 1.0f, 1.0f, 1.0f, 1.0f, },
        };
        // Pyramid
        static const float s_HalfEdgeLength_main = 1.0f;

        static float s_PyramidVertices[][3] =
        {
            {  -2.7340431213378906,-2.7352387905120850,-2.9623701572418213,},
            {        -0.0160623434931040,2.8046987056732178,0.2329618334770203,},
            {        -0.0745132714509964,2.8046987056732178,0.1713254153728485,},
            {        -2.7340431213378906,-2.7352387905120850,2.5775671005249023,},
            {        -2.7340431213378906,-2.7352387905120850,2.5775671005249023,},
            {        -0.0745132714509964,2.8046987056732178,0.1713254153728485,},
            {        -0.0776989310979843,2.8046987056732178,0.1810673177242279,},
            {        2.8058938980102539,-2.7352387905120850,2.5775671005249023,},
            {        2.8058938980102539,-2.7352387905120850,2.5775671005249023,},
            {        -0.0776989310979843,2.8046987056732178,0.1810673177242279,},
            {        -0.0387316457927227,2.8046987056732178,0.1842524707317352,},
            {        2.8058938980102539,-2.7352387905120850,-2.9623701572418213,},
            {        2.8058938980102539,-2.7352387905120850,-2.9623701572418213,},
            {        -0.0387316457927227,2.8046987056732178,0.1842524707317352,},
            {        -0.0160623434931040,2.8046987056732178,0.2329618334770203,},
            {        -2.7340431213378906,-2.7352387905120850,-2.9623701572418213,},
            {        -2.7340431213378906,-2.7352387905120850,2.5775671005249023,},
            {        2.8058938980102539,-2.7352387905120850,2.5775671005249023,},
            {        2.8058938980102539,-2.7352387905120850,-2.9623701572418213,},
            {        -2.7340431213378906,-2.7352387905120850,-2.9623701572418213,},
            {        -0.0776989310979843,2.8046987056732178,0.1810673177242279,},
            {        -0.0745132714509964,2.8046987056732178,0.1713254153728485,},
            {        -0.0160623434931040,2.8046987056732178,0.2329618334770203,},
            {        -0.0387316457927227,2.8046987056732178,0.1842524707317352,},
        };

        static float s_PyramidColors[][4] =
        {
            //front
            { 1.0f, 1.0f, 1.0f, 1.0f, },
            { 1.0f, 1.0f, 1.0f, 1.0f, },
            { 1.0f, 1.0f, 1.0f, 1.0f, },
            { 1.0f, 1.0f, 1.0f, 1.0f, },
            //rightside
         { 1.0f, 1.0f, 1.0f, 1.0f, },
            { 1.0f, 1.0f, 1.0f, 1.0f, },
            { 1.0f, 1.0f, 1.0f, 1.0f, },
            { 1.0f, 1.0f, 1.0f, 1.0f, },
            //upper side
               { 1.0f, 1.0f, 1.0f, 1.0f, },
            { 1.0f, 1.0f, 1.0f, 1.0f, },
            { 1.0f, 1.0f, 1.0f, 1.0f, },
            { 1.0f, 1.0f, 1.0f, 1.0f, },
            //
              { 1.0f, 1.0f, 1.0f, 1.0f, },
            { 1.0f, 1.0f, 1.0f, 1.0f, },
            { 1.0f, 1.0f, 1.0f, 1.0f, },
            { 1.0f, 1.0f, 1.0f, 1.0f, },
            //left side
             { 1.0f, 1.0f, 1.0f, 1.0f, },
            { 1.0f, 1.0f, 1.0f, 1.0f, },
            { 1.0f, 1.0f, 1.0f, 1.0f, },
            { 1.0f, 1.0f, 1.0f, 1.0f, },

            { 1.0f, 1.0f, 1.0f, 1.0f, },
            { 1.0f, 1.0f, 1.0f, 1.0f, },
            { 1.0f, 1.0f, 1.0f, 1.0f, },
            { 1.0f, 1.0f, 1.0f, 1.0f, },
        };
        //Pyramid End
        static float s_WingColors[][4] =
        {
            { 0.0f, 0.55f, 0.7f, 1.0f, },        // Color of vertex 0.
            { 0.0f, 0.55f, 0.7f, 1.0f, },        // Color of vertex 1.
            { 0.0f, 0.55f, 0.7f, 1.0f, },        // Color of vertex 2.
        };
        static float s_GroundColors[][4] =
        {
            //front
            { 0.4f, 0.3f, 0.25f, 1.0f, },
            { 0.4f, 0.3f, 0.25f, 1.0f, }, 
            { groundGreen[0],groundGreen[1],groundGreen[2],groundGreen[3]},
            { groundGreen[0],groundGreen[1],groundGreen[2],groundGreen[3]},

            { 0.4f, 0.3f, 0.25f, 1.0f, },
            { 0.4f, 0.3f, 0.25f, 1.0f, },
            { groundGreen[0],groundGreen[1],groundGreen[2],groundGreen[3]},
            { groundGreen[0],groundGreen[1],groundGreen[2],groundGreen[3]},

            { 0.4f, 0.3f, 0.25f, 1.0f, },
            { 0.4f, 0.3f, 0.25f, 1.0f, },
            { groundGreen[0],groundGreen[1],groundGreen[2],groundGreen[3]},
            { groundGreen[0],groundGreen[1],groundGreen[2],groundGreen[3]},

            { 0.4f, 0.3f, 0.25f, 1.0f, },
            { 0.4f, 0.3f, 0.25f, 1.0f, },
            { groundGreen[0],groundGreen[1],groundGreen[2],groundGreen[3]},
            { groundGreen[0],groundGreen[1],groundGreen[2],groundGreen[3]},
            //upper side
            { groundGreen[0],groundGreen[1],groundGreen[2],groundGreen[3]},
            { groundGreen[0],groundGreen[1],groundGreen[2],groundGreen[3]},
            { groundDarkerGreen[0],groundDarkerGreen[1],groundDarkerGreen[2],groundDarkerGreen[3]},
            { groundDarkerGreen[0],groundDarkerGreen[1],groundDarkerGreen[2],groundDarkerGreen[3]},

            { 0.4f, 0.3f, 0.25f, 1.0f, },
            { 0.4f, 0.3f, 0.25f, 1.0f, },
            { groundGreen[0],groundGreen[1],groundGreen[2],groundGreen[3]},
            { groundGreen[0],groundGreen[1],groundGreen[2],groundGreen[3]},
        };
        //BROWN FOR GROUND == { 0.4f, 0.3f, 0.25f, 1.0f, },
        //GRAS FOR GROUND ==  { 0.5f, 1.0f, 0.0f, 1.0f, },
        static float s_rockBrownOnlyColors[][4] =
        {
            //front
            { 0.2f, 0.16f, 0.15f, 1.0f, },
            { 0.2f, 0.16f, 0.15f, 1.0f, },
            { 0.4f, 0.3f, 0.25f, 1.0f, },
            { 0.4f, 0.3f, 0.25f, 1.0f, },

            { 0.4f, 0.3f, 0.25f, 1.0f, },
            { 0.4f, 0.3f, 0.25f, 1.0f, },
            { 0.4f, 0.3f, 0.25f, 1.0f, },
            { 0.4f, 0.3f, 0.25f, 1.0f, },

            { 0.4f, 0.3f, 0.25f, 1.0f, },
            { 0.4f, 0.3f, 0.25f, 1.0f, },
            { 0.4f, 0.3f, 0.25f, 1.0f, },
            { 0.4f, 0.3f, 0.25f, 1.0f, },

            { 0.4f, 0.3f, 0.25f, 1.0f, },
            { 0.4f, 0.3f, 0.25f, 1.0f, },
            { 0.4f, 0.3f, 0.25f, 1.0f, },
            { 0.4f, 0.3f, 0.25f, 1.0f, },
            //upper side
            { 0.4f, 0.3f, 0.25f, 1.0f, },
            { 0.4f, 0.3f, 0.25f, 1.0f, },
            { 0.4f, 0.3f, 0.25f, 1.0f, },
            { 0.4f, 0.3f, 0.25f, 1.0f, },

            { 0.4f, 0.3f, 0.25f, 1.0f, },
            { 0.4f, 0.3f, 0.25f, 1.0f, },
            { 0.4f, 0.3f, 0.25f, 1.0f, },
            { 0.4f, 0.3f, 0.25f, 1.0f, },
        };

        static float s_GroundCubeVertices[][3] =
        {
            { -s_HalfEdgeLength, -s_HalfEdgeLength, -s_HalfEdgeLength, },
            {  s_HalfEdgeLength, -s_HalfEdgeLength, -s_HalfEdgeLength, },
            {  s_HalfEdgeLength,  s_HalfEdgeLength, -s_HalfEdgeLength, },
            { -s_HalfEdgeLength,  s_HalfEdgeLength, -s_HalfEdgeLength, },

            {  s_HalfEdgeLength, -s_HalfEdgeLength, -s_HalfEdgeLength, },
            {  s_HalfEdgeLength, -s_HalfEdgeLength,  s_HalfEdgeLength, },
            {  s_HalfEdgeLength,  s_HalfEdgeLength,  s_HalfEdgeLength, },
            {  s_HalfEdgeLength,  s_HalfEdgeLength, -s_HalfEdgeLength, },

            {  s_HalfEdgeLength, -s_HalfEdgeLength,  s_HalfEdgeLength, },
            { -s_HalfEdgeLength, -s_HalfEdgeLength,  s_HalfEdgeLength, },
            { -s_HalfEdgeLength,  s_HalfEdgeLength,  s_HalfEdgeLength, },
            {  s_HalfEdgeLength,  s_HalfEdgeLength,  s_HalfEdgeLength, },

            { -s_HalfEdgeLength, -s_HalfEdgeLength,  s_HalfEdgeLength, },
            { -s_HalfEdgeLength, -s_HalfEdgeLength, -s_HalfEdgeLength, },
            { -s_HalfEdgeLength,  s_HalfEdgeLength, -s_HalfEdgeLength, },
            { -s_HalfEdgeLength,  s_HalfEdgeLength,  s_HalfEdgeLength, },

            { -s_HalfEdgeLength,  s_HalfEdgeLength, -s_HalfEdgeLength, },
            {  s_HalfEdgeLength,  s_HalfEdgeLength, -s_HalfEdgeLength, },
            {  s_HalfEdgeLength,  s_HalfEdgeLength,  s_HalfEdgeLength, },
            { -s_HalfEdgeLength,  s_HalfEdgeLength,  s_HalfEdgeLength, },

            { -s_HalfEdgeLength, -s_HalfEdgeLength,  s_HalfEdgeLength, },
            {  s_HalfEdgeLength, -s_HalfEdgeLength,  s_HalfEdgeLength, },
            {  s_HalfEdgeLength, -s_HalfEdgeLength, -s_HalfEdgeLength, },
            { -s_HalfEdgeLength, -s_HalfEdgeLength, -s_HalfEdgeLength, },
        };
        // -----------------------------------------------------------------------------
        // Define the topology of the mesh via indices. An index addresses a vertex from
        // the array above. Three indices represent one triangle. When defining the 
        // triangles of a mesh imagine that you are standing in front of the triangle 
        // and looking to the center of the triangle. If the mesh represents a closed
        // body such as a cube, your view position has to be outside of the body. Now
        // define the indices of the addressed vertices of the triangle in counter-
        // clockwise order.
        // -----------------------------------------------------------------------------
        static int s_CubeIndices[][3] =
        {
            {  0,  1,  2, },
            {  0,  2,  3, },

            {  4,  5,  6, },
            {  4,  6,  7, },

            {  8,  9, 10, },
            {  8, 10, 11, },

            { 12, 13, 14, },
            { 12, 14, 15, },

            { 16, 17, 18, },
            { 16, 18, 19, },

            { 20, 21, 22, },
            { 20, 22, 23, },
        };
        //Background-Mesh-Quadrat
        static float s_QuadBackgroundVertices[][3] =
        {
            { -35.0f, -35.0f, 0.0f, },
            {  35.0f, -35.0f, 0.0f, },
            {  35.0f,  35.0f, 0.0f, },
            { -35.0f,  35.0f, 0.0f, },
        };

        static float s_QuadBackgroundTexCoords[][2] =
        {
            { 0.0f, 1.0f, },                    // Texture coordinate of vertex 0.
            { 1.0f, 1.0f, },                    // Texture coordinate of vertex 1.
            { 1.0f, 0.0f, },                    // Texture coordinate of vertex 2.
            { 0.0f, 0.0f, },                    // Texture coordinate of vertex 3.
        };

        static int s_QuadBackgroundIndices[][3] =
        {
            { 0, 1, 2, },
            { 0, 2, 3, },
        };

        MeshInfo.m_pVertices = &s_QuadBackgroundVertices[0][0];
        MeshInfo.m_pNormals = nullptr;                      // No normals.
        MeshInfo.m_pColors = nullptr;                      // No colors.
        MeshInfo.m_pTexCoords = &s_QuadTexCoords[0][0];
        MeshInfo.m_NumberOfVertices = 4;
        MeshInfo.m_NumberOfIndices = 6;
        MeshInfo.m_pIndices = &s_QuadBackgroundIndices[0][0];
        MeshInfo.m_pTexture = m_pQuadTexture;

        CreateMesh(MeshInfo, &m_pBackgroundMesh);
        //End Background-Mesh
        // Start Heart Mesh
        static float s_HeartVertices[][3] =
        {
        {  1.0000000000000000,1.0000000000000000,-1.0000000000000000,},
{        -1.0000000000000000,1.0000000000000000,-1.0000000000000000,},
{        -0.5156519412994385,-0.9999999403953552,-0.5629054903984070,},
{        0.6298478245735168,-0.9999999403953552,-0.5589676499366760,},
{        0.6298478245735168,-1.0000001192092896,0.1691266298294067,},
{        0.6298478245735168,-0.9999999403953552,-0.5589676499366760,},
{        -0.5156519412994385,-0.9999999403953552,-0.5629054903984070,},
{        -0.5077763795852661,-1.0000001192092896,0.1770021915435791,},
{        -0.5077763795852661,-1.0000001192092896,0.1770021915435791,},
{        -0.5156519412994385,-0.9999999403953552,-0.5629054903984070,},
{        -1.0000000000000000,1.0000000000000000,-1.0000000000000000,},
{        -1.0000000000000000,1.0000000000000000,1.0000000000000000,},
{        -1.0000000000000000,1.0000000000000000,1.0000000000000000,},
{        1.0000000000000000,1.0000000000000000,1.0000000000000000,},
{        0.6298478245735168,-1.0000001192092896,0.1691266298294067,},
{        -0.5077763795852661,-1.0000001192092896,0.1770021915435791,},
{        1.0000000000000000,1.0000000000000000,1.0000000000000000,},
{        1.0000000000000000,1.0000000000000000,-1.0000000000000000,},
{        0.6298478245735168,-0.9999999403953552,-0.5589676499366760,},
{        0.6298478245735168,-1.0000001192092896,0.1691266298294067,},
{        -1.0000000000000000,1.0000000000000000,1.0000000000000000,},
{        -1.0000000000000000,1.0000000000000000,-1.0000000000000000,},
{        1.0000000000000000,1.0000000000000000,-1.0000000000000000,},
 {       1.0000000000000000,1.0000000000000000,1.0000000000000000,},
        };

        static int s_HeartIndices[][3] =
        {
            {  0,  1,  2, },
            {  0,  2,  3, },

            {  4,  5,  6, },
            {  4,  6,  7, },

            {  8,  9, 10, },
            {  8, 10, 11, },

            { 12, 13, 14, },
            { 12, 14, 15, },

            { 16, 17, 18, },
            { 16, 18, 19, },

            { 20, 21, 22, },
            { 20, 22, 23, },
        };

        static float s_HeartColors[][4] =
        {
            
            { 0.9f, 0.92f, 0.16f, 1.0f, },
            { 0.9f, 0.92f, 0.16f, 1.0f, },
            { 0.9f, 0.92f, 0.16f, 1.0f, },
            { 0.9f, 0.92f, 0.16f, 1.0f, },
            //upper Part
            { 0.4f, 0.92f, 0.0f, 1.0f, },
            { 0.4f, 0.92f, 0.0f, 1.0f, },
            { 0.4f, 0.92f, 0.0f, 1.0f, },
            { 0.4f, 0.92f, 0.0f, 1.0f, },

            { 0.9f, 0.92f, 0.16f, 1.0f, },
            { 0.5f, 0.5f, 0.16f, 1.0f, },
            { 0.5f, 0.5f, 0.16f, 1.0f, },
            { 0.9f, 0.92f, 0.16f, 1.0f, },

            { 0.9f, 0.92f, 0.16f, 1.0f, },
           { 0.5f, 0.5f, 0.16f, 1.0f, },
            { 0.5f, 0.5f, 0.16f, 1.0f, },
            { 0.9f, 0.92f, 0.16f, 1.0f, },
            //upper side
            { 0.9f, 0.92f, 0.16f, 1.0f, },
            { 0.5f, 0.5f, 0.16f, 1.0f, },
            { 0.5f, 0.5f, 0.16f, 1.0f, },
            { 0.9f, 0.92f, 0.16f, 1.0f, },

            { 0.9f, 0.92f, 0.16f, 1.0f, },
            { 0.9f, 0.92f, 0.16f, 1.0f, },
            { 0.9f, 0.92f, 0.16f, 1.0f, },
            { 0.9f, 0.92f, 0.16f, 1.0f, },
        };

        static float s_FifthLevelColors[][4] =
        {

            { 1, 0.0f, 0.0f, 1.0f, },
            { 1, 0.0f, 0.0f, 1.0f, },
            { 1, 0.0f, 0.0f, 1.0f, },
            { 1, 0.0f, 0.0f, 1.0f, },
            //upper Part
            { 1, 0.0f, 0.0f, 1.0f, },
            { 1, 0.0f, 0.0f, 1.0f, },
            { 1, 0.0f, 0.0f, 1.0f, },
            { 1, 0.0f, 0.0f, 1.0f, },

               { 1, 0.0f, 0.0f, 1.0f, },
         { 0.7f, 0.0f, 0.0f, 1.0f, },
            { 0.7f, 0.0f, 0.0f, 1.0f, },
            { 1, 0.0f, 0.0f, 1.0f, },

          { 1, 0.0f, 0.0f, 1.0f, },
          { 0.7f, 0.0f, 0.0f, 1.0f, },
            { 0.7f, 0.0f, 0.0f, 1.0f, },
            { 1, 0.0f, 0.0f, 1.0f, },
            //upper side
       { 1, 0.0f, 0.0f, 1.0f, },
            { 0.7f, 0.0f, 0.0f, 1.0f, },
            { 0.7f, 0.0f, 0.0f, 1.0f, },
            { 1, 0.0f, 0.0f, 1.0f, },

           { 1, 0.0f, 0.0f, 1.0f, },
           { 0.7f, 0.0f, 0.0f, 1.0f, },
            { 0.7f, 0.0f, 0.0f, 1.0f, },
            { 1, 0.0f, 0.0f, 1.0f, },
        };

        MeshInfo.m_pVertices = &s_HeartVertices[0][0];
        MeshInfo.m_pNormals = nullptr;                      // No normals.
        MeshInfo.m_pColors = &s_HeartColors[0][0];// &s_HeartColors[0][0];                      // No colors.
        MeshInfo.m_pTexCoords = nullptr;
        MeshInfo.m_NumberOfVertices = 24;
        MeshInfo.m_NumberOfIndices = 36;
        MeshInfo.m_pIndices = &s_HeartIndices[0][0];
        MeshInfo.m_pTexture = nullptr;

        CreateMesh(MeshInfo, &m_pHeartLifeBarMesh);
        // End Heart Mesh 
        // Fifth_Level Indicator
        MeshInfo.m_pVertices = &s_HeartVertices[0][0];
        MeshInfo.m_pNormals = nullptr;                      // No normals.
        MeshInfo.m_pColors = &s_FifthLevelColors[0][0];// &s_HeartColors[0][0];                      // No colors.
        MeshInfo.m_pTexCoords = nullptr;
        MeshInfo.m_NumberOfVertices = 24;
        MeshInfo.m_NumberOfIndices = 36;
        MeshInfo.m_pIndices = &s_HeartIndices[0][0];
        MeshInfo.m_pTexture = nullptr;

        CreateMesh(MeshInfo, &m_pFifthLevelMesh);
        // END Fifth Level Indicator
        
        // DRONES BODIES COLORS AND CREATE MESHES
        static float s_DroneBody_Vertices[][3] =
        {
 {		-0.8862009048461914,1.0000001192092896,-0.7125414013862610,},
{       -2.8625741004943848,1.0000000000000000,-0.6889146566390991,},
{       -2.8625741004943848,-0.9999999403953552,-0.6889146566390991,},
{       -0.8862009048461914,-0.9999999403953552,-0.7125414013862610,},
{       1.0905691385269165,-1.0000001192092896,0.3030114173889160,},
{       -0.8862009048461914,-0.9999999403953552,-0.7125414013862610,},
{       -2.8625741004943848,-0.9999999403953552,-0.6889146566390991,},
{       -1.0000000000000000,-1.0000000000000000,1.0000000000000000,},
{       -1.0000000000000000,-1.0000000000000000,1.0000000000000000,},
{       -2.8625741004943848,-0.9999999403953552,-0.6889146566390991,},
{       -2.8625741004943848,1.0000000000000000,-0.6889146566390991,},
{       -1.0000000000000000,1.0000000000000000,1.0000000000000000,},
{       -1.0000000000000000,1.0000000000000000,1.0000000000000000,},
{       1.0905691385269165,0.9999998807907104,0.3030114173889160,},
{       1.0905691385269165,-1.0000001192092896,0.3030114173889160,},
{       -1.0000000000000000,-1.0000000000000000,1.0000000000000000,},
{       1.0905691385269165,0.9999998807907104,0.3030114173889160,},
{       -0.8862009048461914,1.0000001192092896,-0.7125414013862610,},
{       -0.8862009048461914,-0.9999999403953552,-0.7125414013862610,},
{       1.0905691385269165,-1.0000001192092896,0.3030114173889160,},
{       -1.0000000000000000,1.0000000000000000,1.0000000000000000,},
{       -2.8625741004943848,1.0000000000000000,-0.6889146566390991,},
{       -0.8862009048461914,1.0000001192092896,-0.7125414013862610,},
{       1.0905691385269165,0.9999998807907104,0.3030114173889160,},
        };

        static int s_DroneBody_Indices[][3] =
        {
            {  0,  1,  2, },
            {  0,  2,  3, },

            {  4,  5,  6, },
            {  4,  6,  7, },

            {  8,  9, 10, },
            {  8, 10, 11, },

            { 12, 13, 14, },
            { 12, 14, 15, },

            { 16, 17, 18, },
            { 16, 18, 19, },

            { 20, 21, 22, },
            { 20, 22, 23, },
        };
        static float s_DroneTail_Vertices[][3] =
        {
    {		-0.8862009048461914,1.0000001192092896,-0.7125414013862610,},
{       -7.6950345039367676,1.0000001192092896,-0.1679576039314270,},
{       -7.6950345039367676,-0.9999998807907104,-0.1679576039314270,},
{       -0.8862009048461914,-0.9999999403953552,-0.7125414013862610,},
{       1.0905691385269165,-1.0000001192092896,0.3030114173889160,},
{       -0.8862009048461914,-0.9999999403953552,-0.7125414013862610,},
{       -7.6950345039367676,-0.9999998807907104,-0.1679576039314270,},
{       -8.5461025238037109,-1.0000001192092896,-0.0380263328552246,},
{       -8.5461025238037109,-1.0000001192092896,-0.0380263328552246,},
{       -7.6950345039367676,-0.9999998807907104,-0.1679576039314270,},
{       -7.6950345039367676,1.0000001192092896,-0.1679576039314270,},
{       -8.5461025238037109,0.9999998807907104,-0.0380263328552246,},
{       -8.5461025238037109,0.9999998807907104,-0.0380263328552246,},
{       1.0905691385269165,0.9999998807907104,0.3030114173889160,},
{       1.0905691385269165,-1.0000001192092896,0.3030114173889160,},
{       -8.5461025238037109,-1.0000001192092896,-0.0380263328552246,},
{       1.0905691385269165,0.9999998807907104,0.3030114173889160,},
{       -0.8862009048461914,1.0000001192092896,-0.7125414013862610,},
{       -0.8862009048461914,-0.9999999403953552,-0.7125414013862610,},
{       1.0905691385269165,-1.0000001192092896,0.3030114173889160,},
{       -8.5461025238037109,0.9999998807907104,-0.0380263328552246,},
{       -7.6950345039367676,1.0000001192092896,-0.1679576039314270,},
{       -0.8862009048461914,1.0000001192092896,-0.7125414013862610,},
{       1.0905691385269165,0.9999998807907104,0.3030114173889160,},
        };

        static int s_DroneTail_Indices[][3] =
        {
            {  0,  1,  2, },
            {  0,  2,  3, },

            {  4,  5,  6, },
            {  4,  6,  7, },

            {  8,  9, 10, },
            {  8, 10, 11, },

            { 12, 13, 14, },
            { 12, 14, 15, },

            { 16, 17, 18, },
            { 16, 18, 19, },

            { 20, 21, 22, },
            { 20, 22, 23, },
        };
        static float s_DroneForegroundColor[][4] =
        {
            // downside with ship foreground
            { 0, 0.4f, 0.6f, 1.0f, },
            { 0, 0.4f, 0.6f, 1.0f, },
            { 0, 0.4f, 0.6f, 1.0f, },
            { 0, 0.4f, 0.6f, 1.0f, },

            //backside / invisible on Foreground fly route
            { 0, 0.6f, 1.0f, 1.0f, },
            { 0, 0.5f, 0.8f, 1.0f, },
            { 0, 0.5f, 0.8f, 1.0f, },
            { 0, 0.6f, 1.0f, 1.0f, },
            
            //left front of ship
            { 1, 0.6f, 1.0f, 1.0f, },
            { 1, 0.5f, 0.8f, 1.0f, },
            { 1, 0.5f, 0.8f, 1.0f, },
            { 1, 0.6f, 1.0f, 1.0f, },
            
            //upper side
             { 1, 0.6f, 1.0f, 1.0f, },
            { 1, 0.5f, 0.5f, 1.0f, },
            { 1, 0.5f, 0.5f, 1.0f, },
            { 1, 0.6f, 1.0f, 1.0f, },

            //right back of ship
            { 0, 0.2f, 0.6f, 1.0f, },
            { 0, 0.4f, 0.6f, 1.0f, },
            { 0, 0.4f, 0.6f, 1.0f, },
            { 0, 0.2f, 0.6f, 1.0f, },

              { 0, 0.6f, 1.0f, 1.0f, },
            { 0, 0.5f, 0.8f, 1.0f, },
            { 0, 0.5f, 0.8f, 1.0f, },
            { 0, 0.6f, 1.0f, 1.0f, },
            
        };
        static float s_DroneBackgroundColor[][4] =
        {
            { 0, 0.3f, 0.5f, 1.0f, },
            { 0, 0.15f, 0.25f, 1.0f, },
            { 0, 0.15f, 0.25f, 1.0f, },
            { 0, 0.3f, 0.5f, 1.0f, },
      
             { 0, 0.3f, 0.5f, 1.0f, },
               { 0, 0.15f, 0.25f, 1.0f, },
            { 0, 0.15f, 0.25f, 1.0f, },
            { 0, 0.3f, 0.5f, 1.0f, },

             { 0, 0.3f, 0.5f, 1.0f, },
             { 0, 0.15f, 0.25f, 1.0f, },
            { 0, 0.15f, 0.25f, 1.0f, },
            { 0, 0.3f, 0.5f, 1.0f, },

             { 0, 0.3f, 0.5f, 1.0f, },
              { 0, 0.15f, 0.25f, 1.0f, },
            { 0, 0.15f, 0.25f, 1.0f, },
            { 0, 0.3f, 0.5f, 1.0f, },

             { 0, 0.3f, 0.5f, 1.0f, },
             { 0, 0.15f, 0.25f, 1.0f, },
            { 0, 0.15f, 0.25f, 1.0f, },
            { 0, 0.3f, 0.5f, 1.0f, },

             { 0, 0.3f, 0.5f, 1.0f, },
             { 0, 0.15f, 0.25f, 1.0f, },
            { 0, 0.15f, 0.25f, 1.0f, },
            { 0, 0.3f, 0.5f, 1.0f, },

             { 0, 0.3f, 0.5f, 1.0f, },
              { 0, 0.15f, 0.25f, 1.0f, },
            { 0, 0.15f, 0.25f, 1.0f, },
            { 0, 0.3f, 0.5f, 1.0f, },
        };
        static float s_MainEnemyColor[][4] =
        {      
             { 0, 0.8f, 0.5f, 1.0f, },
             { 0, 1.0f, 0.5f, 1.0f, },
             { 0, 1.0f, 0.5f, 1.0f, },
             { 0, 0.8f, 0.5f, 1.0f, },


             { 0, 0.8f, 0.5f, 1.0f, },
             { 0, 1.0f, 0.5f, 1.0f, },
             { 0, 0.8f, 0.5f, 1.0f, },
             { 0, 1.0f, 0.5f, 1.0f, },

             
              { 0, 1.0f, 0.5f, 1.0f, },
             { 0, 1.0f, 0.5f, 1.0f, },
             { 0, 1.0f, 0.5f, 1.0f, },
             { 0, 1.0f, 0.5f, 1.0f, },

            { 0, 1.0f, 0.5f, 1.0f, },
             { 0, 0.8f, 0.5f, 1.0f, },
             { 0, 0.8f, 0.5f, 1.0f, },
             { 0, 1.0f, 0.5f, 1.0f, },

            { 0, 1.0f, 0.5f, 1.0f, },
             { 0, 0.8f, 0.5f, 1.0f, },
             { 0, 0.8f, 0.5f, 1.0f, },
             { 0, 1.0f, 0.5f, 1.0f, },

              { 0, 1.0f, 0.5f, 1.0f, },
             { 0, 0.8f, 0.5f, 1.0f, },
             { 0, 0.8f, 0.5f, 1.0f, },
             { 0, 1.0f, 0.5f, 1.0f, },
        };

        //BODY
        MeshInfo.m_pVertices = &s_DroneBody_Vertices[0][0];
        MeshInfo.m_pNormals = nullptr;                      
        MeshInfo.m_pColors = &s_DroneForegroundColor[0][0];;// 
        MeshInfo.m_pTexCoords = nullptr;
        MeshInfo.m_NumberOfVertices = 24;
        MeshInfo.m_NumberOfIndices = 36;
        MeshInfo.m_pIndices = &s_DroneBody_Indices[0][0];
        MeshInfo.m_pTexture = nullptr;

        CreateMesh(MeshInfo, &m_pDroneBodyMeshForeground);

        MeshInfo.m_pVertices = &s_DroneBody_Vertices[0][0];
        MeshInfo.m_pNormals = nullptr;
        MeshInfo.m_pColors = &s_DroneBackgroundColor[0][0];// 
        MeshInfo.m_pTexCoords = nullptr;
        MeshInfo.m_NumberOfVertices = 24;
        MeshInfo.m_NumberOfIndices = 36;
        MeshInfo.m_pIndices = &s_DroneBody_Indices[0][0];
        MeshInfo.m_pTexture = nullptr;

        CreateMesh(MeshInfo, &m_pDroneBodyMeshBackground);

        //TAILS
        MeshInfo.m_pVertices = &s_DroneTail_Vertices[0][0];
        MeshInfo.m_pNormals = nullptr;
        MeshInfo.m_pColors = &s_DroneForegroundColor[0][0];// 
        MeshInfo.m_pTexCoords = nullptr;
        MeshInfo.m_NumberOfVertices = 24;
        MeshInfo.m_NumberOfIndices = 36;
        MeshInfo.m_pIndices = &s_DroneTail_Indices[0][0];
        MeshInfo.m_pTexture = nullptr;

        CreateMesh(MeshInfo, &m_pDroneTailMeshForeground);


        MeshInfo.m_pVertices = &s_DroneTail_Vertices[0][0];
        MeshInfo.m_pNormals = nullptr;
        MeshInfo.m_pColors = &s_DroneBackgroundColor[0][0];// 
        MeshInfo.m_pTexCoords = nullptr;
        MeshInfo.m_NumberOfVertices = 24;
        MeshInfo.m_NumberOfIndices = 36;
        MeshInfo.m_pIndices = &s_DroneTail_Indices[0][0];
        MeshInfo.m_pTexture = nullptr;

        CreateMesh(MeshInfo, &m_pDroneTailMeshBackground);

        //
        // creating random Enemy
        MeshInfo.m_pVertices = &s_DroneTail_Vertices[0][0];
        MeshInfo.m_pNormals = nullptr;
        MeshInfo.m_pColors = &s_MainEnemyColor[0][0];// 
        MeshInfo.m_pTexCoords = nullptr;
        MeshInfo.m_NumberOfVertices = 24;
        MeshInfo.m_NumberOfIndices = 36;
        MeshInfo.m_pIndices = &s_DroneTail_Indices[0][0];
        MeshInfo.m_pTexture = nullptr;

        CreateMesh(MeshInfo, &m_pEnemyMesh);


        // -----------------------------------------------------------------------------
        // Define the mesh and its material. The material defines the look of the
        // surface covering the mesh. If the material should contain normals, colors, or
        // texture coordinates then their number has to match the number of vertices.
        // If you do not support normals in a mesh, YoshiX will not apply lighting to
        // this mesh. A textured mesh always has to contain texture coordinates and a
        // handle to a texture. A mesh always has to contain vertices and indices.
        // -----------------------------------------------------------------------------
        //SMeshInfo MeshInfo;

        MeshInfo.m_pVertices = &s_CubeVertices[0][0];
        MeshInfo.m_pNormals = nullptr;                          // No normals
        MeshInfo.m_pColors = nullptr;
        MeshInfo.m_pTexCoords = &s_QuadTexCoords[0][0];                          // No texture coordinates
        MeshInfo.m_NumberOfVertices = 24;
        MeshInfo.m_NumberOfIndices = 36;
        MeshInfo.m_pIndices = &s_CubeIndices[0][0];
        MeshInfo.m_pTexture = m_pWallTexture;                       

        CreateMesh(MeshInfo, &m_pCubeMesh);

        //creating pyramid (Mountain)
        MeshInfo.m_pVertices = &s_PyramidVertices[0][0];
        MeshInfo.m_pNormals = nullptr;                          
        MeshInfo.m_pColors = nullptr;
        MeshInfo.m_pTexCoords = &s_QuadTexCoords[0][0];
        MeshInfo.m_NumberOfVertices = 24;
        MeshInfo.m_NumberOfIndices = 36;
        MeshInfo.m_pIndices = &s_CubeIndices[0][0];
        MeshInfo.m_pTexture = m_pMountainTexture;

        CreateMesh(MeshInfo, &m_pPyramidMesh);

        //creating RocketFront
        MeshInfo.m_pVertices = &s_PyramidVertices[0][0];
        MeshInfo.m_pNormals = nullptr;
        MeshInfo.m_pColors = &s_PyramidColors[0][0];
        MeshInfo.m_pTexCoords = nullptr;
        MeshInfo.m_NumberOfVertices = 24;
        MeshInfo.m_NumberOfIndices = 36;
        MeshInfo.m_pIndices = &s_CubeIndices[0][0];
        MeshInfo.m_pTexture = nullptr;

        CreateMesh(MeshInfo, &m_pRocketFrontMesh);

        //creating RocketBody
        MeshInfo.m_pVertices = &s_CubeVertices[0][0];
        MeshInfo.m_pNormals = nullptr;                          // No normals
        MeshInfo.m_pColors = &s_PyramidColors[0][0];                          //&s_GroundColors[0][0];
        MeshInfo.m_pTexCoords = nullptr;                          // No texture coordinates
        MeshInfo.m_NumberOfVertices = 24;
        MeshInfo.m_NumberOfIndices = 36;
        MeshInfo.m_pIndices = &s_CubeIndices[0][0];
        MeshInfo.m_pTexture = nullptr;                          // No texture

        CreateMesh(MeshInfo, &m_pRocketBodyMesh);

        //creating ground with gras mesh
        MeshInfo.m_pVertices = &s_GroundCubeVertices[0][0];
        MeshInfo.m_pNormals = nullptr;                          // No normals
        MeshInfo.m_pColors = nullptr;                          //&s_GroundColors[0][0];
        MeshInfo.m_pTexCoords = &s_QuadTexCoords[0][0];                          // No texture coordinates
        MeshInfo.m_NumberOfVertices = 24;
        MeshInfo.m_NumberOfIndices = 36;
        MeshInfo.m_pIndices = &s_CubeIndices[0][0];
        MeshInfo.m_pTexture = m_pGroundTexture;                          // No texture

        CreateMesh(MeshInfo, &m_pGroundCubeMesh);

        //creating ground with rock ground only
        MeshInfo.m_pVertices = &s_GroundCubeVertices[0][0];
        MeshInfo.m_pNormals = nullptr;                          // No normals
        MeshInfo.m_pColors = &s_rockBrownOnlyColors[0][0];
        MeshInfo.m_pTexCoords = nullptr;                          // No texture coordinates
        MeshInfo.m_NumberOfVertices = 24;
        MeshInfo.m_NumberOfIndices = 36;
        MeshInfo.m_pIndices = &s_CubeIndices[0][0];
        MeshInfo.m_pTexture = nullptr;                          // No texture

        CreateMesh(MeshInfo, &m_pGroaundBrownMesh);

        // Thruster Triangles
        MeshInfo.m_pVertices = &s_TriangleVertices[0][0];
        MeshInfo.m_pNormals = nullptr;
        MeshInfo.m_pColors = &s_TriangleColors[0][0];
        MeshInfo.m_pTexCoords = nullptr;
        MeshInfo.m_NumberOfVertices = 3;
        MeshInfo.m_NumberOfIndices = 3;   //immer durch 3 teilbar (weil alles durch Dreiecke dargestellt wird)
        MeshInfo.m_pIndices = &s_TriangleIndices[0][0];
        MeshInfo.m_pTexture = &m_pWallTexture;  //man kann wohl auch eine Textur drüber ziehen
        CreateMesh(MeshInfo, &m_pTriangleMesh);
        
        // Rocket Wings Triangles
        MeshInfo.m_pVertices = &s_TriangleVertices[0][0];
        MeshInfo.m_pNormals = nullptr;
        MeshInfo.m_pColors = &s_WingColors[0][0];
        MeshInfo.m_pTexCoords = nullptr;
        MeshInfo.m_NumberOfVertices = 3;
        MeshInfo.m_NumberOfIndices = 3;   //immer durch 3 teilbar (weil alles durch Dreiecke dargestellt wird)
        MeshInfo.m_pIndices = &s_TriangleIndices[0][0];
        MeshInfo.m_pTexture = nullptr;  //man kann wohl auch eine Textur drüber ziehen

        CreateMesh(MeshInfo, &m_pRocketWingsMesh);


        return true;
    }

    // -----------------------------------------------------------------------------

    bool CApplication::InternOnReleaseMeshes()
    {
        // -----------------------------------------------------------------------------
        // Important to release the mesh again when the application is shut down.
        // -----------------------------------------------------------------------------
        ReleaseMesh(m_pCubeMesh);
        ReleaseMesh(m_pTriangleMesh);
        ReleaseMesh(m_pGroundCubeMesh);
        ReleaseMesh(m_pGroaundBrownMesh);
        ReleaseMesh(m_pBackgroundMesh);
        ReleaseMesh(m_pPyramidMesh); 
        ReleaseMesh(m_pRocketFrontMesh);
        ReleaseMesh(m_pRocketBodyMesh);
        ReleaseMesh(m_pRocketWingsMesh);
        ReleaseMesh(m_pHeartLifeBarMesh);
        ReleaseMesh(m_pFifthLevelMesh);
        ReleaseMesh(m_pDroneBodyMeshForeground);
        ReleaseMesh(m_pDroneTailMeshForeground);
        ReleaseMesh(m_pDroneBodyMeshBackground);
        ReleaseMesh(m_pDroneTailMeshBackground);
        ReleaseMesh(m_pEnemyMesh);


        return true;
    }

    // -----------------------------------------------------------------------------

    bool CApplication::InternOnResize(int _Width, int _Height)
    {
        float ProjectionMatrix[16];

        // -----------------------------------------------------------------------------
        // The projection matrix defines the size of the camera frustum. The YoshiX
        // camera has the shape of a pyramid with the eye position at the top of the
        // pyramid. The horizontal view angle is defined by the vertical view angle
        // and the ratio between window width and window height.
        // -----------------------------------------------------------------------------
        GetProjectionMatrix(m_FieldOfViewY, static_cast<float>(_Width) / static_cast<float>(_Height), 0.1f, 100.0f, ProjectionMatrix);
        SetProjectionMatrix(ProjectionMatrix);

        return true;
    }

    // -----------------------------------------------------------------------------

    bool CApplication::InternOnUpdate()
    {
        //Camera Settings
        float Eye[3];
        float At[3];
        float Up[3];

        float ViewMatrix[16];

        // -----------------------------------------------------------------------------
        // Define position and orientation of the camera in the world.
        // -----------------------------------------------------------------------------
        Eye[0] = 0.0f; At[0] = 0.0f; Up[0] = 0.0f;
        Eye[1] = 2.0f; At[1] = 0.0f; Up[1] = 1.0f;
        Eye[2] = -30.0f; At[2] = 0.0f; Up[2] = 0.0f;

        GetViewMatrix(Eye, At, Up, ViewMatrix);
        
        SetViewMatrix(ViewMatrix);

        return true;
    }

    // -----------------------------------------------------------------------------

        //Helping Globals
    // ------------------------------------------------------
    float groundLevel = -14.5f;
    // -> Rocket Position
    float g_X = -10.0f;
    float g_Y = 0.0f;
    // -> Rocket Spawn Postition
    float g_X_Spawn = -15.0f;
    float g_Y_Spawn = 0.0f;
    // -> Projectile Position
    float g_projectile_X = 0.0f;
    float g_projectile_Y = 0.0f;
    // -> Ground-Object Position
    float g_ground_X = 0.0f;
    float g_ground_Y = -14.0f;
    // -> Background Position
    float g_background_X = 0.0f;
    float g_background_Y = 0.0f;
    // -> Background Position 2nd
    float g_backgroundSec_X = 70.0f;
    float g_backgroundSec_Y = 0.0f;
    // -> Ground floor Position
    float g_floorground_X = 0.0f;
    float g_floorground_Y = 0.0f;
    // -> Enemy1 Fly Position
    float g_enemy1_X = 0.0f;
    float g_enemy1_Y = 0.0f;
    // -> Enemy Leader Drone Fly Position
    float g_droneleader_X = 0.0f;
    float g_droneleader_Y = 0.0f;
    // -> Position of ParticleEffects
    float g_particle_X = 0.0f;
    float g_particle_Y = 0.0f;
    float g_particle2_X = 0.0f;
    float g_particle2_Y = 0.0f;
    float g_hitparticle_X = 0.0f;
    float g_hitparticle_Y = 0.0f;
    // -> Thrust Rotation
    float rotationAngle;
    // -> FallingSpeed 
    float g_Step = 0.025f;
    float levelSpeed_Step = 0.1f;
    float standardEnemySpeed = 0.1f;
    float backgroundSpeed_Step = 0.005f;
    // -> Index Thrusters
    int thrusterIndex = 0;
    // -> enemySpawn X-Position
    float enemySpawnX = 35;
    float droneSpawnX = -35; //if so this can just be "-enemySpawn"
    // -> Bools
    bool isAccelerating = false;
    bool isShooting = false;
    bool isSpawning = false;
    bool isEnemy1Spawning = false;
    bool isEnemyDroneApproaching = false;
    bool isEnemyDroneAttacking = false;
    bool isGameOver = false;
    bool isLevelChanging = false;
    bool isParticleEffectActive = false;
    bool isHitEffectActive = false;

    float shoot_Step = 0.7f;
    // -> Timer to save current Time for duration of effects
    double currentTime = 0.0f;
    double levelTime = 0.0f;
    double tempTimeLevelIncreaser = 0.0f;
    double particleTime = 0.0f;
    double hitTime = 0.0f;
    // -> Levelborders
    int upperBorder = 15;
    int lowerBorder = - 14;
    int leftBorder = -22;
    int rightBorder = 22;
    // -> randomNumber
    float randomValue = 0;
    float randomSizeValue = 1;
    float randomRotationValue = 0;
    float randomEnemy1SpeedValue = 0;
    float randomEnemyDronesSpeedValue = 0;
    float randomDroneYOffset = 0;
    float randomDroneY2Offset = 0;

    // -> collision checks
    bool isOnGround = false;
    
    float particleSize = 0.2f;
    // -> GameLogic Elements
    int lifeCounter = 3;
    int levelCounter = 1;
    float speedAccelerator = 1.2f; // more Speed with higher Level
    float overallSpeedMultiplicator = 1.0f;
    float levelTimeOffset = 0.0f; // saves the offset on resets on R or gameOver

    float currentEndtime = 0.0f;
    float bestTime = 0.0f;
    float timeInLevel = 0.0f;
    



    // ------------------------------------------------------
    bool CApplication::checkCollision()
    {
        // --------------------------------------------------------------------------------
        // Checks for all possible Collisions  -> Ship Coordinates => g_X / g_Y
        // This function also decreases the life counter if the player gets in contact with
        // and enemy.
        // --------------------------------------------------------------------------------
       
        // Reset the ship on ground contact
        if (g_Y < -13.9f)
        {
            //Reset Player to middle of Level -> maybe consider resetting Enemys as well
            g_hitparticle_X = g_X;
            g_hitparticle_Y = g_Y;
            isHitEffectActive = true;
            hitTime = GetTimeInSeconds();

            g_Y = g_Y_Spawn;
            g_X = g_X_Spawn;

            isEnemy1Spawning = false;
            isEnemyDroneAttacking = false;
            lifeCounter--;
        }
        // Reset the ship on mountain contact
        if (g_Y < g_ground_Y + (3.5 * randomSizeValue) && 
            (g_X < g_ground_X + (1.5f * randomSizeValue) && g_X > g_ground_X - (1.5f * randomSizeValue)))
        {
            g_hitparticle_X = g_X;
            g_hitparticle_Y = g_Y;
            isHitEffectActive = true;
            hitTime = GetTimeInSeconds();

            g_Y = g_Y_Spawn;
            g_X = g_X_Spawn;
            isEnemy1Spawning = false;
            isEnemyDroneAttacking = false;
            lifeCounter--;
        }
        // Reset the ship on enemy contact
        if ((g_Y < g_enemy1_Y + 1.5f && g_Y > g_enemy1_Y - 1.5f) && 
            (g_X < g_enemy1_X + enemySpawnX + 2 && g_X > g_enemy1_X + enemySpawnX - 2))
        {
            g_hitparticle_X = g_X;
            g_hitparticle_Y = g_Y;
            isHitEffectActive = true;
            hitTime = GetTimeInSeconds();

            g_Y = g_Y_Spawn;
            g_X = g_X_Spawn;

            // resetting the enemy -> in order to give player the chance to get back in the game
            isEnemy1Spawning = false;
            isEnemyDroneAttacking = false;
            lifeCounter--;
        }
        // Reset the enemy ship on contact with projectile
        if (g_projectile_X > g_enemy1_X - 1 + enemySpawnX &&
            g_projectile_X < g_enemy1_X + 1 + enemySpawnX &&
            g_projectile_X != 0)
        {
            if (g_projectile_Y > g_enemy1_Y - 1 && g_projectile_Y < g_enemy1_Y + 1)
            {
                isEnemy1Spawning = false;
                isHitEffectActive = true;
                hitTime = GetTimeInSeconds();
                g_hitparticle_X = g_enemy1_X + enemySpawnX;
                g_hitparticle_Y = g_enemy1_Y;
            }
        }

        // Reset the ship on drone contact (they have to be in attack mode)
        if (((g_Y < g_droneleader_Y + 1 && g_Y > g_droneleader_Y-1) || 
            (g_Y < g_droneleader_Y + 1 + randomDroneYOffset && g_Y > g_droneleader_Y - 1 + randomDroneYOffset) ||
            (g_Y < g_droneleader_Y + 1 - randomDroneY2Offset && g_Y > g_droneleader_Y - 1 - randomDroneY2Offset)) && isEnemyDroneAttacking)
        {
            if (g_X < g_droneleader_X + enemySpawnX + 1 && g_X > g_droneleader_X+enemySpawnX - 1)
            {
                g_hitparticle_X = g_X;
                g_hitparticle_Y = g_Y;
                isHitEffectActive = true;
                hitTime = GetTimeInSeconds();

                g_Y = g_Y_Spawn;
                g_X = g_X_Spawn;
                isEnemyDroneAttacking = false;
                lifeCounter--;

                
            }
        }

       

        return true;
    }

    bool CApplication::levelController()
    {
        // -------------------------------------------------------------
        // Logic behind this:
        // Every 10 s the speed-multiplicator is increased by 10%
        // A counter starts every 10 s and if it reaches 10 its adds up the 10%.
        // -------------------------------------------------------------

        levelTime = GetTimeInSeconds() - levelTimeOffset;

        if (!isLevelChanging)
        {
            timeInLevel = levelTime;
            isLevelChanging = true;
        }

        if (levelTime - timeInLevel > 10)
        {
            // maximum Level cap is a multiplicator of 3.5f
            if (overallSpeedMultiplicator < 3.5f)
            {
                overallSpeedMultiplicator *= speedAccelerator;
            }
            //std::cout << "Updated Speed: " << overallSpeedMultiplicator << std::endl;
            isLevelChanging = false;
            levelCounter++;
        }

        //std::cout << levelTime <<  " --- "<<levelTime-timeInLevel << std::endl;
        //std::cout << overallSpeedMultiplicator << std::endl;


        return true;
    }

    bool CApplication::drawCurrentLevel(float _X, float _Y)
    {
        //----------------------------
        // Draws the remaining Lifes from right to left on the upper right side of the screen
        // ---------------------------
        float WorldMatrix[16];
        float ScaleMatrix[16];
        float RotationMatrix[16];
        float TmpMatrix[16];


        float TranslationMatrix[16];
        float containerOffset = 1.0f;

        for (int i = 1; i < levelCounter+1; i++)
        {
            GetTranslationMatrix(_X + ((i-1) * containerOffset), _Y, 0.0f, TranslationMatrix);
            GetScaleMatrix(0.3f, ScaleMatrix);
            GetRotationXMatrix(180, RotationMatrix);

            MulMatrix(ScaleMatrix, TranslationMatrix, TmpMatrix);
            MulMatrix(RotationMatrix, TmpMatrix, WorldMatrix);

            SetWorldMatrix(WorldMatrix); 
            // red color on every fifth level for readability
            if (i % 5 == 0 && i != 0) 
            {
                DrawMesh(m_pFifthLevelMesh);
            }
            else 
            { 
                DrawMesh(m_pHeartLifeBarMesh);
            }
        }

       
        return true;
    }


    bool CApplication::drawLifeContainter(float _X, float _Y)
    {
        //----------------------------
        // Draws the remaining Lifes from right to left on the upper right side of the screen
        // As "Lifebar" The spaceship is used.
        // ---------------------------
        float WorldMatrix[16];
        float ScaleMatrix[16];
        float RotationMatrix[16];
        float TmpMatrix[16];
        float TranslationMatrix[16];
        float containerOffset = 2.0f;
        for (int i = lifeCounter; i > 0; i--)
        {
            GetTranslationMatrix(_X-(i*containerOffset), _Y, 0.0f, TranslationMatrix);
            GetScaleMatrix(0.09f,0.15f,0.09f, ScaleMatrix);
            MulMatrix(ScaleMatrix, TranslationMatrix, WorldMatrix);
            SetWorldMatrix(WorldMatrix);
            DrawMesh(m_pRocketFrontMesh);  //TODO Replace this Placeholder-Mesh
            

            GetTranslationMatrix(_X - (i * containerOffset), _Y-0.7f, 0.0f, TranslationMatrix);
            GetScaleMatrix(0.25f, ScaleMatrix);
            MulMatrix(ScaleMatrix, TranslationMatrix, WorldMatrix);
            SetWorldMatrix(WorldMatrix);
            DrawMesh(m_pRocketBodyMesh);  //TODO Replace this Placeholder-Mesh

            GetTranslationMatrix(_X - (i * containerOffset) -0.5f, _Y - 1.2f, 0.0f, TranslationMatrix);
            GetScaleMatrix(0.25f, ScaleMatrix);
            GetRotationZMatrix(140, RotationMatrix);
            MulMatrix(ScaleMatrix, TranslationMatrix, TmpMatrix);
            MulMatrix(RotationMatrix, TmpMatrix, WorldMatrix);

            SetWorldMatrix(WorldMatrix);
            DrawMesh(m_pRocketWingsMesh);  //TODO Replace this Placeholder-Mesh

            GetTranslationMatrix(_X - (i * containerOffset) + 0.5f, _Y - 1.2f, 0.0f, TranslationMatrix);
            GetScaleMatrix(0.25f, ScaleMatrix);
            GetRotationZMatrix(210, RotationMatrix);
            MulMatrix(ScaleMatrix, TranslationMatrix, TmpMatrix);
            MulMatrix(RotationMatrix, TmpMatrix, WorldMatrix);

            SetWorldMatrix(WorldMatrix);
            DrawMesh(m_pRocketWingsMesh);  //TODO Replace this Placeholder-Mesh

        }

        return true;
    }

    bool CApplication::spawnEnemy()
    {
        if (!isEnemy1Spawning)
        {
            randomEnemy1SpeedValue = static_cast<float>((15 + (rand() % (30 - 15 + 1))))/100;
            g_enemy1_X = 0;
            g_enemy1_Y = (lowerBorder + (rand() % (upperBorder - lowerBorder + 1)));
            isEnemy1Spawning = true;
        }
        else
        {
            float WorldMatrix[16];
            float RotationMatrix[16];
            float TranslationMatrix[16];
            float TmpMatrix[16];
            float ScaleMatrix[16];

            GetTranslationMatrix(enemySpawnX + g_enemy1_X, g_enemy1_Y, 0.0f, TranslationMatrix);
            GetRotationXMatrix(270, RotationMatrix);
            GetScaleMatrix(1 * 0.5f, 1, 1, ScaleMatrix);

            MulMatrix(RotationMatrix, TranslationMatrix, TmpMatrix);
            MulMatrix(ScaleMatrix, TmpMatrix, WorldMatrix);

            SetWorldMatrix(WorldMatrix);
            DrawMesh(m_pEnemyMesh);
            
            // Wingpart
            GetTranslationMatrix(enemySpawnX + g_enemy1_X-1, g_enemy1_Y+0.3f, 0.0f, TranslationMatrix);
            GetRotationZMatrix(110, RotationMatrix);
            GetScaleMatrix(0.6f, ScaleMatrix);

            MulMatrix(RotationMatrix, TranslationMatrix, TmpMatrix);
            MulMatrix(ScaleMatrix, TmpMatrix, WorldMatrix);

            SetWorldMatrix(WorldMatrix);
            DrawMesh(m_pRocketWingsMesh);

            g_enemy1_X -= randomEnemy1SpeedValue * overallSpeedMultiplicator; //TODO: Allgemeiner Accelerator für mehr Schwierigkeit desto länger man spielt

            if (g_enemy1_X < -70)
            {
                isEnemy1Spawning = false;
                g_enemy1_X = 0;
            }
        }

        //to get the real coordinates of the enemy for collision do -> g_enemy1_X +enemySpawnX !!!!!!
        
        return true;
    }

    bool CApplication::spawnEnemy_attackDrones()
    {
        // -----------------------------------------------------------------------------
        // These drone-ships are a little special. They are in the background first, flying along with the player, after they 
        // leave the are on the right side of the level they turn around approaching the player very fast.
        // They stay on the same height as they flew in the background to give the player a chance to react to these kind of tactics.
        // Drones are not affected by the Accelerator with a higher level. But on higher level there will spawn more drones.
        // -----------------------------------------------------------------------------
      
        //TODO: Add a logic to spawn more drones on a longer running game
        if (!isEnemyDroneApproaching && !isEnemyDroneAttacking)
        {
            randomEnemyDronesSpeedValue = static_cast<float>((15 + (rand() % (20 - 15 + 1)))) / 100;
            randomDroneYOffset = (2 + (rand() % (10 - 2 + 1)));
            randomDroneY2Offset = (2 + (rand() % (15 - 2 + 1)));
            g_droneleader_X = 0;
            g_droneleader_Y = (lowerBorder + (rand() % (upperBorder - lowerBorder + 1)));
            isEnemyDroneApproaching = true;
        }
        else
        {
            float WorldMatrix[16];
            float RotationMatrix[16];
            float TranslationMatrix[16];
            float TmpMatrix[16];
            float ScaleMatrix[16];
            float rescaleXAxis = 0.5f;

            if (!isEnemyDroneAttacking)
            {
                int angle = 90;
                float backgroundScale = 0.5f;

                //1st Drone
                GetTranslationMatrix(droneSpawnX + g_droneleader_X, g_droneleader_Y, 0.5f, TranslationMatrix);
                GetRotationXMatrix(angle, RotationMatrix);
                GetScaleMatrix(backgroundScale * rescaleXAxis, backgroundScale, backgroundScale, ScaleMatrix);
                MulMatrix(RotationMatrix, TranslationMatrix, TmpMatrix);
                MulMatrix(ScaleMatrix, TmpMatrix, WorldMatrix);

                SetWorldMatrix(WorldMatrix);
                DrawMesh(m_pDroneTailMeshBackground); 


                //2nd Drone
                GetTranslationMatrix(droneSpawnX + g_droneleader_X, g_droneleader_Y + randomDroneYOffset, 0.5f, TranslationMatrix);
                GetRotationXMatrix(angle, RotationMatrix);
                GetScaleMatrix(backgroundScale * rescaleXAxis, backgroundScale, backgroundScale, ScaleMatrix);
                MulMatrix(RotationMatrix, TranslationMatrix, TmpMatrix);
                MulMatrix(ScaleMatrix, TmpMatrix, WorldMatrix);

                SetWorldMatrix(WorldMatrix);
                DrawMesh(m_pDroneTailMeshBackground); 
            
                //3rd Drone
                GetTranslationMatrix(droneSpawnX + g_droneleader_X, g_droneleader_Y - randomDroneY2Offset, 0.5f, TranslationMatrix);
                GetRotationXMatrix(angle, RotationMatrix);
                GetScaleMatrix(backgroundScale * rescaleXAxis, backgroundScale, backgroundScale, ScaleMatrix);
                MulMatrix(RotationMatrix, TranslationMatrix, TmpMatrix);
                MulMatrix(ScaleMatrix, TmpMatrix, WorldMatrix);

                SetWorldMatrix(WorldMatrix);
                DrawMesh(m_pDroneTailMeshBackground); 
              
                g_droneleader_X += randomEnemyDronesSpeedValue * overallSpeedMultiplicator;
            }
            else
            {
                int angle = 270;
                //1st Drone
                GetTranslationMatrix(enemySpawnX + g_droneleader_X, g_droneleader_Y, 0.0f, TranslationMatrix);
                GetRotationXMatrix(angle, RotationMatrix);
                GetScaleMatrix(1*rescaleXAxis,1,1, ScaleMatrix);
                MulMatrix(RotationMatrix, TranslationMatrix, TmpMatrix);
                MulMatrix(ScaleMatrix, TmpMatrix, WorldMatrix);

                SetWorldMatrix(WorldMatrix);
                DrawMesh(m_pDroneTailMeshForeground);
                
                //2nd Drone
                GetTranslationMatrix(enemySpawnX + g_droneleader_X, g_droneleader_Y + randomDroneYOffset, 0.0f, TranslationMatrix);
                GetRotationXMatrix(angle, RotationMatrix);
                GetScaleMatrix(1 * rescaleXAxis, 1, 1, ScaleMatrix);

                MulMatrix(RotationMatrix, TranslationMatrix, TmpMatrix);
                MulMatrix(ScaleMatrix, TmpMatrix, WorldMatrix);

                SetWorldMatrix(WorldMatrix);
                DrawMesh(m_pDroneTailMeshForeground);
                
                //3rd Drone
                GetTranslationMatrix(enemySpawnX + g_droneleader_X, g_droneleader_Y - randomDroneY2Offset, 0.0f, TranslationMatrix);
                GetRotationXMatrix(angle, RotationMatrix);
                GetScaleMatrix(1 * rescaleXAxis, 1, 1, ScaleMatrix);

                MulMatrix(RotationMatrix, TranslationMatrix, TmpMatrix);
                MulMatrix(ScaleMatrix, TmpMatrix, WorldMatrix);

                SetWorldMatrix(WorldMatrix);
                DrawMesh(m_pDroneTailMeshForeground);
                
                g_droneleader_X -= randomEnemyDronesSpeedValue*2.5f;
            }

            if (g_droneleader_X > 70)
            {
                isEnemyDroneApproaching = false;
                isEnemyDroneAttacking = true;
                g_droneleader_X = 0;
            }

            if (g_droneleader_X < -70)
            {
                isEnemyDroneAttacking = false;
                g_droneleader_X = 0;
            }
        }

        return true;
    }

    bool CApplication::buildGround()
    {
        float startX = -35.0f;
        float groundOffSet = -2.0f;
        float WorldMatrix[16];
     
        for (int i = 1; i < 35; i++)
        {
            GetTranslationMatrix(startX - (groundOffSet * i) + g_floorground_X, -16.5f, 0.0f, WorldMatrix);
            SetWorldMatrix(WorldMatrix);
            DrawMesh(m_pGroundCubeMesh);

            GetTranslationMatrix(startX - (groundOffSet * i) + g_floorground_X + 68.0f, -16.5f, 0.0f, WorldMatrix);
            SetWorldMatrix(WorldMatrix);
            DrawMesh(m_pGroundCubeMesh);
        }

        g_floorground_X -= levelSpeed_Step * overallSpeedMultiplicator;
        if (g_floorground_X < -70)
        {
            g_floorground_X = 0;
        }
        

        return true;
    }

    bool CApplication::spawnGroundObject()
    {
        //std::cout << randomValue << " - " << randomSizeValue << " - "  <<  randomRotationValue <<std::endl;
        if (!isSpawning)
        {
            g_ground_X = rightBorder + 5;
            isSpawning = true;
            randomValue = rand() % 50; //sets the respawn randomly to make it look more generic
            randomSizeValue = static_cast<float>((50 + (rand() % (200 - 50 + 1))))/100; //sets the size between 1 and 0.5 randomly
            randomRotationValue = static_cast<float>((0 + (rand() % (360 - 0 + 1)))); //sets the size between 1 and 360 randomly
        }
        else
        {
            //TODO -> random Size and therefore different position to groundlevel
            float WorldMatrix[16];
            float RotationMatrix[16];
            float TranslationMatrix[16];
            float TmpMatrix[16];
            float ScaleMatrix[16];

            GetTranslationMatrix(g_ground_X, g_ground_Y, 0.0f, TranslationMatrix);
            GetRotationYMatrix(randomRotationValue, RotationMatrix);
            GetScaleMatrix(randomSizeValue, ScaleMatrix);

            MulMatrix(RotationMatrix, TranslationMatrix, TmpMatrix);
            MulMatrix(ScaleMatrix, TmpMatrix, WorldMatrix);

            SetWorldMatrix(WorldMatrix);
            DrawMesh(m_pPyramidMesh);

            g_ground_X -= levelSpeed_Step * overallSpeedMultiplicator;

            if (g_ground_X < leftBorder - 5 - randomValue)
            {
                isSpawning = false;
            }
        }

        return true;
    }

    bool CApplication::showThrusters()
    {
        if (isAccelerating)
        {
            float WorldMatrix[16];
            float RotationMatrix[16];
            float TranslationMatrix[16];
            float InverseTranslationMatrix[16];
            float TmpMatrix[16];
            float ScaleMatrix[16];

            //switch case für den aktuellen Thruster
            switch (thrusterIndex)
            {
                case 1:
                    if (GetTimeInSeconds() - currentTime < 0.2f)
                    {
                        GetTranslationMatrix(g_X - 2.7f, g_Y, 0.0f, TranslationMatrix);
                        GetRotationZMatrix(90, RotationMatrix);
                        GetTranslationMatrix(g_X, g_Y, 0.0f, InverseTranslationMatrix);

                        MulMatrix(TranslationMatrix, RotationMatrix, TmpMatrix);
                        MulMatrix(RotationMatrix, TranslationMatrix, WorldMatrix);


                        SetWorldMatrix(WorldMatrix);
                        DrawMesh(m_pTriangleMesh);
                    }
                    else
                    {
                        isAccelerating = false;
                    }
                    break;
                case 2:
                    if (GetTimeInSeconds() - currentTime < 0.2f)
                    {
                        GetTranslationMatrix(g_X, g_Y+1.8f, 0.0f, TranslationMatrix);
                        GetRotationZMatrix(0, RotationMatrix);
                        GetScaleMatrix(0.5f, ScaleMatrix);
                        MulMatrix(TranslationMatrix, RotationMatrix, TmpMatrix);
                        MulMatrix(ScaleMatrix, TmpMatrix, WorldMatrix);


                        SetWorldMatrix(WorldMatrix);
                        DrawMesh(m_pTriangleMesh);
                    }
                    else
                    {
                        isAccelerating = false;
                    }
                    break;
                case 3:
                    if (GetTimeInSeconds() - currentTime < 0.2f)
                    {
                        GetTranslationMatrix(g_X, g_Y-1.8f, 0.0f, TranslationMatrix);
                        GetRotationZMatrix(180, RotationMatrix);
                        GetScaleMatrix(0.5f, ScaleMatrix);
                        
                        MulMatrix(RotationMatrix, TranslationMatrix,TmpMatrix);
                        MulMatrix(ScaleMatrix, TmpMatrix, WorldMatrix);


                        SetWorldMatrix(WorldMatrix);
                        DrawMesh(m_pTriangleMesh);
                    }
                    else
                    {
                        isAccelerating = false;
                    }
                    break;
                case 4:
                    if (GetTimeInSeconds() - currentTime < 0.2f)
                    {
                        GetTranslationMatrix(g_X + 1.2f, g_Y-1.0f, 0.0f, TranslationMatrix);
                        GetRotationZMatrix(230, RotationMatrix);
                        GetScaleMatrix(0.5f, ScaleMatrix);

                        MulMatrix(RotationMatrix, TranslationMatrix, TmpMatrix);
                        MulMatrix(ScaleMatrix, TmpMatrix, WorldMatrix);

                        SetWorldMatrix(WorldMatrix);
                        DrawMesh(m_pTriangleMesh);

                        GetTranslationMatrix(g_X + 1.2f, g_Y + 1.0f, 0.0f, TranslationMatrix);
                        GetRotationZMatrix(310, RotationMatrix);
                        GetScaleMatrix(0.5f, ScaleMatrix);

                        MulMatrix(RotationMatrix, TranslationMatrix, TmpMatrix);
                        MulMatrix(ScaleMatrix, TmpMatrix, WorldMatrix);

                        SetWorldMatrix(WorldMatrix);
                        DrawMesh(m_pTriangleMesh);
                    }
                    else
                    {
                        isAccelerating = false;
                    }
                    break;
                case 5:
                    break;
                case 6:
                    break;
                default:
                    break;
            }
        }
        return true;
    }

    bool CApplication::shootProjectile()
    {
        if (isShooting)
        {
            float WorldMatrix[16];
            float RotationMatrix[16];
            float TranslationMatrix[16];
            float InverseTranslationMatrix[16];
            float TmpMatrix[16];
            float ScaleMatrix[16];


            if (g_projectile_X > 35)
            {
                isShooting = false;
            }
            

            GetTranslationMatrix(g_projectile_X+1, g_projectile_Y, 0.0f, TranslationMatrix);
            GetRotationZMatrix(270, RotationMatrix);
            GetScaleMatrix(0.4f, 1.0f, 0.2f, ScaleMatrix);

            MulMatrix(RotationMatrix,TranslationMatrix , TmpMatrix);
            MulMatrix(ScaleMatrix, TmpMatrix, WorldMatrix);

            SetWorldMatrix(WorldMatrix);
            DrawMesh(m_pTriangleMesh);

            g_projectile_X += shoot_Step;
            
        }


        return true;
    }

    bool CApplication::moveBackground()
    {
        // moves the background and resets the background on end
        float WorldMatrix[16];
        
        GetTranslationMatrix(g_background_X, g_background_Y, 1.0f, WorldMatrix);
        SetWorldMatrix(WorldMatrix);
        DrawMesh(m_pBackgroundMesh);

        GetTranslationMatrix(g_backgroundSec_X, g_backgroundSec_Y, 1.0f, WorldMatrix);
        SetWorldMatrix(WorldMatrix);
        DrawMesh(m_pBackgroundMesh);

        if (g_background_X < -70)
        {
            g_background_X = 69.9f;
        }
        if (g_backgroundSec_X < -70)
        {
            g_backgroundSec_X = 69.9f;
        }

        g_background_X -= backgroundSpeed_Step * overallSpeedMultiplicator;
        g_backgroundSec_X -= backgroundSpeed_Step * overallSpeedMultiplicator;




        return true;
    }

    bool CApplication::particleEffects()
    {
        float WorldMatrix[16];
        float RotationMatrix[16];
        float TranslationMatrix[16];
        float InverseTranslationMatrix[16];
        float TmpMatrix[16];
        float ScaleMatrix[16];

        float effect_Step = 0.1f;
        float explosion_effect_Step = 0.05f;
        
        if (isParticleEffectActive)
        {
            if (GetTimeInSeconds() - particleTime < 0.2f)
            {
                //1 -> up right
                GetTranslationMatrix(g_particle_X, g_particle_Y, 0.0f, TranslationMatrix);
                GetRotationZMatrix(-45, RotationMatrix);
                GetScaleMatrix(0.2f, 0.1f, 0.2f, ScaleMatrix);

                MulMatrix(RotationMatrix, TranslationMatrix, TmpMatrix);
                MulMatrix(ScaleMatrix, TmpMatrix, WorldMatrix);

                SetWorldMatrix(WorldMatrix);
                DrawMesh(m_pTriangleMesh);

                g_particle_X += effect_Step;
                g_particle_Y += effect_Step;

                //2 -> down right
                GetTranslationMatrix(g_particle2_X, g_particle2_Y, 0.0f, TranslationMatrix);
                GetRotationZMatrix(-45, RotationMatrix);
                GetScaleMatrix(0.2f, 0.1f, 0.2f, ScaleMatrix);

                MulMatrix(RotationMatrix, TranslationMatrix, TmpMatrix);
                MulMatrix(ScaleMatrix, TmpMatrix, WorldMatrix);

                SetWorldMatrix(WorldMatrix);
                DrawMesh(m_pTriangleMesh);

                g_particle2_X += effect_Step;
                g_particle2_Y -= effect_Step;
            }
            else
            {
                isParticleEffectActive = false;
            }
        }

        if (isHitEffectActive)
        {
            //std::cout << "effect" << std::endl;;
            if (GetTimeInSeconds() - hitTime < 0.5f)
            {
                for (int i = 0; i < 8; i++)
                {
                    GetTranslationMatrix(g_hitparticle_X, g_hitparticle_Y, 0.0f, TranslationMatrix);
                    GetRotationZMatrix(45*i, RotationMatrix);
                    GetScaleMatrix(particleSize, ScaleMatrix);

                    MulMatrix(RotationMatrix, TranslationMatrix, TmpMatrix);
                    MulMatrix(ScaleMatrix, TmpMatrix, WorldMatrix);

                    SetWorldMatrix(WorldMatrix);
                    DrawMesh(m_pTriangleMesh);
                }
                    particleSize += explosion_effect_Step;
                
            }
            else
            {
                isHitEffectActive = false;
                particleSize = 0.2f;
            }
        }
        
        return true;
    }

    

    bool CApplication::InternOnFrame()
    {
        float WorldMatrix[16];
        float RotationMatrix[16];
        float TranslationMatrix[16];
        float ScaleMatrix[16];
        float InverseTranslationMatrix[16];
        float TmpMatrix[16];

        
        //Front_Rocket
        GetTranslationMatrix(g_X + 1.6f, g_Y, 0.0f, TranslationMatrix);
        GetRotationZMatrix(270, RotationMatrix);
        GetTranslationMatrix(g_X, g_Y, 0.0f, InverseTranslationMatrix);
        GetScaleMatrix(0.3f, ScaleMatrix);

        MulMatrix(ScaleMatrix, TranslationMatrix , TmpMatrix);
        MulMatrix(RotationMatrix, TmpMatrix, WorldMatrix);

        SetWorldMatrix(WorldMatrix);
        DrawMesh(m_pRocketFrontMesh);

        //Body_Rocket
        GetTranslationMatrix(g_X, g_Y, 0.0f, TranslationMatrix);
        GetScaleMatrix(0.85f, ScaleMatrix);
        MulMatrix(ScaleMatrix, TranslationMatrix, WorldMatrix);
        SetWorldMatrix(WorldMatrix);
        DrawMesh(m_pRocketBodyMesh);

        //Wings_Back Rocket
        GetTranslationMatrix(g_X - 1.4f, g_Y - 1.0f, -0.1f, TranslationMatrix);
        GetRotationZMatrix(130, RotationMatrix);
        GetScaleMatrix(0.8f, ScaleMatrix);

        MulMatrix(RotationMatrix, TranslationMatrix, TmpMatrix);
        MulMatrix(ScaleMatrix, TmpMatrix, WorldMatrix);

        SetWorldMatrix(WorldMatrix);
        DrawMesh(m_pRocketWingsMesh);

        GetTranslationMatrix(g_X - 1.4f, g_Y + 1.0f, -0.1f, TranslationMatrix);
        GetRotationZMatrix(50, RotationMatrix);
        GetScaleMatrix(0.8f, ScaleMatrix);

        MulMatrix(RotationMatrix, TranslationMatrix, TmpMatrix);
        MulMatrix(ScaleMatrix, TmpMatrix, WorldMatrix);

        SetWorldMatrix(WorldMatrix);
        DrawMesh(m_pRocketWingsMesh);
        
        
     
        if (!lifeCounter <= 0) //On GameOver only show the Ship -> and some score infos
        {
            // Build the playground -> maybe export to other cpp-File
            buildGround();
            // shoot the projectile
            shootProjectile();
            // spawn a mountain every time one has passed by
            spawnGroundObject();
            // spawns flying enemy;
            spawnEnemy();
            // spawns flying enemy-drones at first in the background and then turning to hit the player
            spawnEnemy_attackDrones();
            // move the background slowly to not get sick
            moveBackground();
            // check for collisions with all possible collision-objects
            checkCollision();
            // draw the life container according to the lifes remaining
            drawLifeContainter(23, 16);
            // level counter, the current level is related to time the player is playing
            levelController();

            //Falling until reached ground
            if (g_Y > lowerBorder)
            {
                g_Y -= g_Step;
            }
        }
        // rotate the triangle mesh and put it unter the "rocket", only visible if "WASD" is pressed for a fraction of a second
        showThrusters();
        drawCurrentLevel(-21.5f,16);
        // show particle effects on contact
        if (isParticleEffectActive || isHitEffectActive)
        {
            particleEffects();//DEBUG
        }

        // Respect the Levelborders pal!
        if (g_Y < lowerBorder){g_Y = lowerBorder;}
        if (g_Y > upperBorder){g_Y = upperBorder;}
        if (g_X < leftBorder){g_X = leftBorder;}
        if (g_X > rightBorder){g_X = rightBorder;}

        
        double CurrentRealTime = GetTimeInSeconds();

        

        
        for (;;)
        {
            if (GetTimeInSeconds() - CurrentRealTime > 0.012)
            {
                break;
            }
        }
        //
        return true;
    }

    bool CApplication::InternOnKeyEvent(unsigned int _Key, bool _IsKeyDown, bool _IsAltDown)
    {
        // -----------------------------------------------------------------------------
        // Pressing the given key implies the condition to become true.
        // _IsKeyDown if false key has been released!!!
        // -----------------------------------------------------------------------------
        if (_Key == 'W')
        {
         
            g_Y += 0.4f;
            isAccelerating = true;
            thrusterIndex = 3;
            currentTime = GetTimeInSeconds();
        }
        if (_Key == 'S')
        {
      
            g_Y -= 0.2f;
            isAccelerating = true;
            thrusterIndex = 2;
            currentTime = GetTimeInSeconds();
        }
        if (_Key == 'D')
        {
            
            g_X += 0.4f;
            isAccelerating = true;
            thrusterIndex = 1;
            currentTime = GetTimeInSeconds();
        }
        if (_Key == 'A')
        {
    
            isAccelerating = true;
            thrusterIndex = 4;
            currentTime = GetTimeInSeconds();
            g_X -= 0.4f;
        }
        if (_Key == ' ')
        {
            if (!isShooting)
            {
                g_projectile_X = g_X;
                g_projectile_Y = g_Y;
                // Setting up effect for shooting
                particleTime = GetTimeInSeconds();
                isParticleEffectActive = true;
                g_particle_X = g_X + 2.5f;
                g_particle_Y = g_Y;
                g_particle2_X = g_X + 2.5f;
                g_particle2_Y = g_Y;
                
                isShooting = true;
            }
        }
        if (_Key == 'R' || _Key == 'r')
        {
            // Reset - Restart all Button
            lifeCounter = 3;

            isAccelerating = false;
            isShooting = false;
            isSpawning = false;
            isEnemy1Spawning = false;
            isEnemyDroneApproaching = false;
            isEnemyDroneAttacking = false;
            isGameOver = false;
            isLevelChanging = false;
            overallSpeedMultiplicator = 1.0f;
            levelTimeOffset = GetTimeInSeconds();
            levelCounter = 1;

            g_X = -12;
            g_Y = 0;
        }

        return true;
    }
} 

// namespace

void main()
{
    GetFrequency();
    StartTime();
    srand((unsigned)time(0)); //creating random spawn references

    CApplication Application;

    RunApplication(800, 600, "YoshiX Example", &Application);
}