// Include C++ headers
#include <iostream>
#include <string>
#include <cmath>
#include <cstdlib> // For rand()
#include <chrono>
#include <thread>
#include <windows.h>


//#include <external/sfml/include/SFML/Audio.hpp> 

// Include GLEW
#include <GL/glew.h>

// Include GLFW
#include <glfw3.h>

//#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"
#include <SOIL.h> 
// Include GLM
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>



// Shader loading utilities and other
#include <common/shader.h>
#include <common/util.h>
#include <common/camera.h>
#include <common/texture.h>
#include <common/terrain.h>
#include <common/model.h>
#include "particles/IntParticleEmitter.h"
#include "particles/DustEmitter.h"
#include "particles/DebrisEmitter.h"


using namespace std::chrono; 
//

#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"
using namespace std;
using namespace glm;

// Function prototypes
void initialize();
void createContext();
float getHeight(float vx, float vz);
void mainLoop();
void free();
struct dirLight; 

#define W_WIDTH 1024
#define W_HEIGHT 768
#define TITLE "MOUNTAIN HIKING"

// Global variables
GLFWwindow* window;
Camera* camera;
GLuint terrain_shaderProgram, skybox_shaderProgram, blur_shaderProgram, veg_shaderProgram;
GLuint terrainProjLoc, terrainViewLoc, terrainModelLoc;
GLuint vegProjLoc, vegViewLoc, vegModelLoc;
GLuint skyProjLoc, skyViewLoc, skyModelLoc;

// for the terrain
unsigned int terrainVAO, terrainVBO, terrainIBO;
int width, height, nrChannels;
vector<unsigned > indices;
vector<float> vertices; 
vector<vec3> normals;
vector<float> heights;

// for vegetation
GLuint stemVAO, leavesVAO;
GLuint stemVerticesVBO, stemUVVBO, leavesVerticesVBO, leavesUVVBO;
GLuint stemTexture, leavesTexture;
std::vector<vec3> stemVertices, stemNormals, leavesVertices, leavesNormals;
std::vector<vec2> stemUVs, leavesUVs;
GLuint vegtextureSampler;
mat4 vegmodel = mat4(1.0f);
std::vector<glm::mat4> treeInstances;

//sf::Sound windSound;

GLuint vegtimeUniform;
GLuint vegrotUniform;
GLuint vegvoxUniform;
GLuint vegcolorUniform;
GLuint vegrottimeUniform;
GLuint rotUniform;
GLuint voxUniform;
GLuint timeUniform;
GLuint rottimeUniform;
GLuint terrainTexture;
GLuint snowTexture;
GLuint waterTexture;
GLuint terrainTextureSampler;
GLuint snowTextureSampler;
GLuint waterTextureSampler;
vector<float> texCoords;
vector<float> texCoords2;

// FOR THE BLUR
unsigned int framebuffer;
unsigned int textureColorBuffer;
unsigned int quadVAO, quadVBO;
GLuint screenTextureloc;
GLuint windSpeedloc;
GLuint blurFactorloc;

// FOR PARTICLES
GLuint particleShaderProgram;
GLuint pvMatrixloc;

// for the skybox
unsigned int skyboxVAO, skyboxVBO;
GLuint skyboxTexture;
vector<string> skyboxfaces;
mat4 skyModelMat; 

GLuint lightDirectionLoc; 


struct dirLight {
    vec4 La;
    vec4 Ld;
    vec4 Ls;
    vec3 lightDirection;
    float power;
};


dirLight sun{
    vec4{ 1, 1, 1, 1 },
    vec4{ 1, 1, 1, 1 },
    vec4{ 1, 1, 1, 1 },
    normalize(vec3(- 0.5f, -0.2f, -0.5f)),
    30.0f
};



GLuint loadTexture(const char* path);
// Standard acceleration due to gravity
const float g = 9.80665f;
int humanHeight = 5;
int camspeedmax = 50;
float yScale = 100.0f / 256.0f;
int rotationalDistortion = 0;
int voxelising = 0;

vec3 winddirection = vec3(-1.0f, 0.0f, 0.0f);

mat4 terrainmodel = mat4(1.0f); 

float getHeight(float worldX, float worldZ) {
    int x0 = floor(worldX + width / 2.0f);
    int x1 = std::min(x0 + 1, width - 1);
    int z0 = floor(worldZ + height / 2.0f);
    int z1 = std::min(z0 + 1, height - 1);

    // clamp the indices to stay within bounds
    x0 = std::max(0, std::min(x0, width - 1));
    z0 = std::max(0, std::min(z0, height - 1));
    x1 = std::max(0, std::min(x1, width - 1));
    z1 = std::max(0, std::min(z1, height - 1));

    float h00 = heights[z0 * width + x0];
    float h01 = heights[z1 * width + x0];
    float h10 = heights[z0 * width + x1];
    float h11 = heights[z1 * width + x1];

    // compute fractional offsets within the grid cell
    float dx = (worldX + width / 2.0f) - x0;
    float dz = (worldZ + height / 2.0f) - z0;

    // bilinear interpolation
    float heightBottom = (1.0f - dx) * h00 + dx * h10;
    float heightTop = (1.0f - dx) * h01 + dx * h11;
    float interpolatedHeight = (1.0f - dz) * heightBottom + dz * heightTop;

    return interpolatedHeight;
}

void createContext() {

    // TERRAIN !!

    terrain_shaderProgram = loadShaders(
        "TerrainShading.vertexshader",
        "TerrainShading.fragmentshader");

    terrainProjLoc = glGetUniformLocation(terrain_shaderProgram, "projection");
    terrainViewLoc = glGetUniformLocation(terrain_shaderProgram, "view");
    terrainModelLoc = glGetUniformLocation(terrain_shaderProgram, "model");
    lightDirectionLoc = glGetUniformLocation(terrain_shaderProgram, "lightDirection"); 
    timeUniform = glGetUniformLocation(terrain_shaderProgram, "time");
    rottimeUniform = glGetUniformLocation(terrain_shaderProgram, "rottime");
    rotUniform = glGetUniformLocation(terrain_shaderProgram, "rotationalDistortion");
    voxUniform = glGetUniformLocation(terrain_shaderProgram, "voxelising");

    
    //unsigned char* data = stbi_load("heightmap1.png", &width, &height, &nrChannels, 0);
    unsigned char* data = SOIL_load_image("heightmap1.png", &width, &height, &nrChannels, SOIL_LOAD_L);
    if (data)
    {
        std::cout << "Loaded heightmap of size " << height << " x " << width << std::endl;
    }
    else
    {
        std::cout << "Failed to load texture" << std::endl;

    }
    
    unsigned bytePerPixel = nrChannels;

    
    heights.resize(width * height); // because otherwise it gives me an error of index out of range


    for (int z = 0; z < height; ++z) {
        for (int x = 0; x < width; ++x) {
            

            //int flippedZ = z; // height - 1 - z; // flip the heightmap in vertex generation επειδη φορτωνεται αναποδα η εικονα
            float heightValue = data[z * width + x] * yScale; // height is between [0,100]
            float vz = -height / 2.0f + z ; // to transfer the center of the terrain 
            float vx = -width / 2.0f + x ;  //in the center of the world
            
            heights[z * width + x] = heightValue;

            vertices.push_back(vx);     // vx
            vertices.push_back(heightValue);  // vy (height) 
            vertices.push_back(vz);     // vz
        }
    }
    
    // compute the normals
    
    vector<vec3> normals(width * height);//, glm::vec3(0.0f));

    for (int z = 0; z < height - 1; ++z) {
        for (int x = 0; x < width - 1; ++x) {
            // Current vertex
            vec3 v0 = vec3(x, heights[z * width + x], z);
            // Vertex directly below (z+1)
            vec3 v1 = vec3(x, heights[(z + 1) * width + x], z + 1);
            // Vertex to the right (x+1)
            vec3 v2 = vec3(x + 1, heights[z * width + x + 1], z);

            // Compute the normal using the cross product
            vec3 normal = normalize(cross(v1 - v0, v2 - v0));

            // Add this normal to all vertices involved in the triangle
            normals[z * width + x] += normal;
            normals[(z + 1) * width + x] += normal;
            normals[z * width + x + 1] += normal;
        }
    }

    // Normalize all accumulated normals
    for (int i = 0; i < normals.size(); ++i) {
        normals[i] = normalize(normals[i]);
    }
    SOIL_free_image_data(data); 

    for (int z = 0; z < height - 1; ++z) {
        for (int x = 0; x < width - 1; ++x) {
            int topLeft = z * width + x;
            int topRight = topLeft + 1;
            int bottomLeft = (z + 1) * width + x;
            int bottomRight = bottomLeft + 1;

            // Triangle 1
            indices.push_back(topLeft);
            indices.push_back(bottomLeft);
            indices.push_back(topRight);

            // Triangle 2
            indices.push_back(topRight);
            indices.push_back(bottomLeft);
            indices.push_back(bottomRight);
        }
    }
    
    // terrain textures 

    terrainTexture = loadTexture("grassrock.jpg"); // function in texture.cpp
    snowTexture = loadTexture("snow.jpg");
    waterTexture = loadTexture("water.jpg");

    terrainTextureSampler = glGetUniformLocation(terrain_shaderProgram, "terrainTextureSampler");
    snowTextureSampler = glGetUniformLocation(terrain_shaderProgram, "snowTextureSampler");
    waterTextureSampler = glGetUniformLocation(terrain_shaderProgram, "waterTextureSampler");
    
    
    //  mapping the texture in the terrain
    
    int textureRepeatFactor = 10.0f; // Repeat the texture 10 times in each axes in the terrain
    vector<float> predefinedAngles = { 0.0f, pi<float>() / 2, pi<float>() };

    vector<float> randomAngles(textureRepeatFactor * textureRepeatFactor); // a random angle for each texture repeatition

    for (int i = 0; i < randomAngles.size(); ++i) { 
        int randomIndex = rand() % predefinedAngles.size();  // Random index [0, predefinedAngles.size() - 1]
        randomAngles[i] = predefinedAngles[randomIndex];     // Pick a random predefined angle
    }

    // Compute texture coordinates with rotation
    for (int z = 0; z <= height; ++z) {
        for (int x = 0; x <= width; ++x) {

            int aindex = floor((z / height) * textureRepeatFactor); // decade index
            int bindex = floor((x /width) * textureRepeatFactor); // unit index
            if (aindex > textureRepeatFactor-1) { aindex = 9; }
            if (bindex > textureRepeatFactor-1){ bindex = 9; }
            float angle = randomAngles[ aindex* textureRepeatFactor + bindex ];
            float cosTheta = cos(radians(angle));
            float sinTheta = sin(radians(angle));

            // Base texture coordinates
            float u = (float)x / width * textureRepeatFactor;
            float v = (float)z / height * textureRepeatFactor;
            
            // Rotate texture coordinates
            float uRotated = u * cosTheta - v * sinTheta; 
            float vRotated = u * sinTheta + v * cosTheta;

            texCoords.push_back(uRotated);
            texCoords.push_back(vRotated);
        }
    }
    // texture coordinates 2 (higher frequency)
    
    int textureRepeatFactor2 = 50.0f;

    vector<float> randomAngles2(textureRepeatFactor2 * textureRepeatFactor2); // a random angle for each texture repeatition

    for (int i = 0; i < randomAngles2.size(); ++i) {
        int randomIndex = rand() % predefinedAngles.size();  // Random index [0, predefinedAngles.size() - 1]
        randomAngles2[i] = predefinedAngles[randomIndex];     // Pick a random predefined angle
    }

    for (int z = 0; z <= height; ++z) {
        for (int x = 0; x <= width; ++x) {

            int aindex = floor((z / height) * textureRepeatFactor2); // decade index
            int bindex = floor((x / width) * textureRepeatFactor2); // unit index
            if (aindex > textureRepeatFactor2 - 1) { aindex = 9; }
            if (bindex > textureRepeatFactor2 - 1) { bindex = 9; }
            float angle = randomAngles2[aindex * textureRepeatFactor2 + bindex];
            float cosTheta = cos(radians(angle));
            float sinTheta = sin(radians(angle));

            // Base texture coordinates
            float u = (float)x / width * textureRepeatFactor2;
            float v = (float)z / height * textureRepeatFactor2;

            // Rotate texture coordinates
            float uRotated = u * cosTheta - v * sinTheta;
            float vRotated = u * sinTheta + v * cosTheta;

            texCoords2.push_back(uRotated);
            texCoords2.push_back(vRotated);
        }
    }


    // VEGETATION

    veg_shaderProgram = loadShaders("vegetation.vertexshader", "vegetation.fragmentshader");
    vegViewLoc = glGetUniformLocation(veg_shaderProgram, "Vi");
    vegProjLoc = glGetUniformLocation(veg_shaderProgram, "Pr");
    vegModelLoc = glGetUniformLocation(veg_shaderProgram, "Mo");
    vegtimeUniform = glGetUniformLocation(veg_shaderProgram, "time");
    vegrottimeUniform = glGetUniformLocation(veg_shaderProgram, "rottime");
    vegrotUniform = glGetUniformLocation(veg_shaderProgram, "rotationalDistortion");
    vegvoxUniform = glGetUniformLocation(veg_shaderProgram, "voxelising");
    vegcolorUniform = glGetUniformLocation(veg_shaderProgram, "vegcolor");

    vegtextureSampler = glGetUniformLocation(veg_shaderProgram, "textureSampler");

    // **Load Stem Model**
    loadOBJ("models/tree/MapleTreeStem.obj", stemVertices, stemUVs, stemNormals);

    glGenVertexArrays(1, &stemVAO);
    glBindVertexArray(stemVAO);

    // Stem vertices VBO
    glGenBuffers(1, &stemVerticesVBO);
    glBindBuffer(GL_ARRAY_BUFFER, stemVerticesVBO);
    glBufferData(GL_ARRAY_BUFFER, stemVertices.size() * sizeof(glm::vec3), &stemVertices[0], GL_STATIC_DRAW);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, NULL);
    glEnableVertexAttribArray(0);

    // Stem UVs VBO
    glGenBuffers(1, &stemUVVBO);
    glBindBuffer(GL_ARRAY_BUFFER, stemUVVBO);
    glBufferData(GL_ARRAY_BUFFER, stemUVs.size() * sizeof(glm::vec2), &stemUVs[0], GL_STATIC_DRAW);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 0, NULL);
    glEnableVertexAttribArray(1);

    // Load stem texture
    stemTexture = loadTexture("models/tree/maple_bark.png");

    // **Load Leaves Model**
    loadOBJ("models/tree/MapleTreeLeaves.obj", leavesVertices, leavesUVs, leavesNormals);

    glGenVertexArrays(1, &leavesVAO);
    glBindVertexArray(leavesVAO);

    // Leaves vertices VBO
    glGenBuffers(1, &leavesVerticesVBO);
    glBindBuffer(GL_ARRAY_BUFFER, leavesVerticesVBO);
    glBufferData(GL_ARRAY_BUFFER, leavesVertices.size() * sizeof(glm::vec3), &leavesVertices[0], GL_STATIC_DRAW);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, NULL);
    glEnableVertexAttribArray(0);

    // Leaves UVs VBO
    glGenBuffers(1, &leavesUVVBO);
    glBindBuffer(GL_ARRAY_BUFFER, leavesUVVBO);
    glBufferData(GL_ARRAY_BUFFER, leavesUVs.size() * sizeof(glm::vec2), &leavesUVs[0], GL_STATIC_DRAW);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 0, NULL);
    glEnableVertexAttribArray(1);

    // Load leaves texture
    leavesTexture = loadTexture("models/tree/maple_leaf.png");
    
    // Add tree instances at different positions
    for (int i = 0; i < 800; ++i) {
        
        glm::mat4 modelMatrix = glm::mat4(1.0f);

        // Translate trees to a grid layout
        float randomX = static_cast<float>(rand() % width - width /2); 
        float randomZ = static_cast<float>(rand() % height - height/2); // Range: [-920/2, 920/2]
        float randomY = getHeight(randomX, randomZ);
        if (randomY > 5 && randomY < 80) { //i dont want trees in the sea! or in high attitude
            modelMatrix = glm::translate(modelMatrix, glm::vec3(randomX, randomY, randomZ));

            // scale trees to random sizes between 0.5 and 0.2
            float randomScale = 0.2f - static_cast<float>(rand()) / static_cast<float>(RAND_MAX / 0.1f);
            modelMatrix = glm::scale(modelMatrix, glm::vec3(randomScale)); 

            treeInstances.push_back(modelMatrix); 
        }       
    }
    

    // PARTICLES
    particleShaderProgram = loadShaders(
        "particles/ParticleShader.vertexshader",
        "particles/ParticleShader.fragmentshader");
    pvMatrixloc = glGetUniformLocation(particleShaderProgram, "PV");

    


    // Sky box !!

    skybox_shaderProgram = loadShaders(
        "Skybox.vertexshader",
        "Skybox.fragmentshader");

    skyViewLoc = glGetUniformLocation(skybox_shaderProgram, "view");
    skyProjLoc = glGetUniformLocation(skybox_shaderProgram, "projection");
    skyModelLoc = glGetUniformLocation(skybox_shaderProgram, "model");

    skyboxfaces = {
        "DaylightBox_Right.bmp", "DaylightBox_Left.bmp",
        "DaylightBox_Top.bmp", "DaylightBox_Bottom.bmp",
        "DaylightBox_Front.bmp", "DaylightBox_Back.bmp"};
    skyboxTexture = loadCubemap(skyboxfaces);

    // 36 vertices(6 faces * 2 triangles per face * 3 vertices per triangle).
    float skyboxVertices[] = {
        // Back face
        -1.0f,  1.0f, -1.0f,  // Top-left
        -1.0f, -1.0f, -1.0f,  // Bottom-left
         1.0f, -1.0f, -1.0f,  // Bottom-right
         1.0f, -1.0f, -1.0f,  // Bottom-right
         1.0f,  1.0f, -1.0f,  // Top-right
        -1.0f,  1.0f, -1.0f,  // Top-left

        // Front face
        -1.0f, -1.0f,  1.0f,  // Bottom-left
        -1.0f,  1.0f,  1.0f,  // Top-left
         1.0f,  1.0f,  1.0f,  // Top-right
         1.0f,  1.0f,  1.0f,  // Top-right
         1.0f, -1.0f,  1.0f,  // Bottom-right
        -1.0f, -1.0f,  1.0f,  // Bottom-left

        // Left face
        -1.0f,  1.0f,  1.0f,  // Top-right
        -1.0f,  1.0f, -1.0f,  // Top-left
        -1.0f, -1.0f, -1.0f,  // Bottom-left
        -1.0f, -1.0f, -1.0f,  // Bottom-left
        -1.0f, -1.0f,  1.0f,  // Bottom-right
        -1.0f,  1.0f,  1.0f,  // Top-right

        // Right face
         1.0f,  1.0f, -1.0f,  // Top-left
         1.0f,  1.0f,  1.0f,  // Top-right
         1.0f, -1.0f,  1.0f,  // Bottom-right
         1.0f, -1.0f,  1.0f,  // Bottom-right
         1.0f, -1.0f, -1.0f,  // Bottom-left
         1.0f,  1.0f, -1.0f,  // Top-left

         // Top face
         -1.0f,  1.0f, -1.0f,  // Top-left
          1.0f,  1.0f, -1.0f,  // Top-right
          1.0f,  1.0f,  1.0f,  // Bottom-right
          1.0f,  1.0f,  1.0f,  // Bottom-right
         -1.0f,  1.0f,  1.0f,  // Bottom-left
         -1.0f,  1.0f, -1.0f,  // Top-left

         // Bottom face
         -1.0f, -1.0f, -1.0f,  // Top-right
          1.0f, -1.0f, -1.0f,  // Top-left
          1.0f, -1.0f,  1.0f,  // Bottom-left
          1.0f, -1.0f,  1.0f,  // Bottom-left
         -1.0f, -1.0f,  1.0f,  // Bottom-right
         -1.0f, -1.0f, -1.0f   // Top-right
    };
    
    float mult = std::max(width, height); 
    /*
    for (int i = 0; i < sizeof(skyboxVertices) / sizeof(skyboxVertices[0]); i++) {
        skyboxVertices[i] *= (mult/2); //to be out of my terrain
    }
    */
    skyModelMat = mat4(
        10*width/2, 0.0f, 0.0f, 0.0f,  // First row  //if i wanted the box to be in 
        0.0f, 10*mult/2, 0.0f, 0.0f,  // Second row
        0.0f, 0.0f, 10*height/2, 0.0f,  // Third row 
        0.0f, 0.0f, 0.0f, 1.0f   // Fourth row
    );
    glUniformMatrix4fv(skyModelLoc, 1, GL_FALSE, &skyModelMat[0][0]);
    
    
    
    // SOUND

    PlaySound("wind.wav", NULL, SND_ASYNC | SND_FILENAME | SND_LOOP);

    

    // first, configure the terrainVAO (and terrainVBO + terrainIBO)
    
    glGenVertexArrays(1, &terrainVAO);
    glBindVertexArray(terrainVAO);
    
    glGenBuffers(1, &terrainVBO);
    glBindBuffer(GL_ARRAY_BUFFER, terrainVBO);
    glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(float), &vertices[0], GL_STATIC_DRAW);

    // position attribute
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);

    glGenBuffers(1, &terrainIBO);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, terrainIBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(unsigned), &indices[0], GL_STATIC_DRAW);
    

    // Texture Coordinates VBO
    unsigned int texCoordVBO;
    glGenBuffers(1, &texCoordVBO); 
    glBindBuffer(GL_ARRAY_BUFFER, texCoordVBO); 
    glBufferData(GL_ARRAY_BUFFER, texCoords.size() * sizeof(float), texCoords.data(), GL_STATIC_DRAW); 
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(1);  // location 1 for texture coordinates
    
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, terrainTexture);

    // Activate texture1
    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D, snowTexture);

    // Activate texture2 
    glActiveTexture(GL_TEXTURE2);
    glBindTexture(GL_TEXTURE_2D, waterTexture);

    glUniform1i(terrainTextureSampler, 0);
    glUniform1i(snowTextureSampler, 1);
    glUniform1i(waterTextureSampler, 2);
    
    glUniform3f(lightDirectionLoc, sun.lightDirection.x, sun.lightDirection.y, sun.lightDirection.z);

    unsigned int texCoord2VBO;
    glGenBuffers(1, &texCoord2VBO);
    glBindBuffer(GL_ARRAY_BUFFER, texCoord2VBO);
    glBufferData(GL_ARRAY_BUFFER, texCoords2.size() * sizeof(float), texCoords2.data(), GL_STATIC_DRAW);
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(2);  // location 2 for texture coordinates 2
    


    // Normals VBO
    unsigned int normalVBO;
    glGenBuffers(1, &normalVBO);
    glBindBuffer(GL_ARRAY_BUFFER, normalVBO);
    glBufferData(GL_ARRAY_BUFFER, normals.size() * sizeof(vec3), normals.data(), GL_STATIC_DRAW);
    glVertexAttribPointer(3, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(3); //  location 3 for normals

    // VAO and VBO for skybox:
    glGenVertexArrays(1, &skyboxVAO);
    glGenBuffers(1, &skyboxVBO);
    glBindVertexArray(skyboxVAO);
    glBindBuffer(GL_ARRAY_BUFFER, skyboxVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(skyboxVertices), &skyboxVertices, GL_STATIC_DRAW);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);



    // for the blur 
    blur_shaderProgram = loadShaders(
        "Blur.vertexshader",
        "Blur.fragmentshader");

    screenTextureloc = glGetUniformLocation(blur_shaderProgram, "screenTexture");
    windSpeedloc = glGetUniformLocation(blur_shaderProgram, "windSpeed");
    blurFactorloc = glGetUniformLocation(blur_shaderProgram, "blurFactor");
 

    glGenFramebuffers(1, &framebuffer);
    glBindFramebuffer(GL_FRAMEBUFFER, framebuffer);

    // Create a texture to hold the framebuffer color
    glGenTextures(1, &textureColorBuffer);
    glBindTexture(GL_TEXTURE_2D, textureColorBuffer);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, W_WIDTH, W_HEIGHT, 0, GL_RGB, GL_UNSIGNED_BYTE, NULL); 
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, textureColorBuffer, 0);
    
    // Create a renderbuffer for depth and stencil attachments (optional)
    unsigned int rbo;
    glGenRenderbuffers(1, &rbo);
    glBindRenderbuffer(GL_RENDERBUFFER, rbo);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, W_WIDTH, W_HEIGHT); 
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, rbo);
    
    // Check if framebuffer is complete
    if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
        std::cout << "ERROR::FRAMEBUFFER:: Framebuffer is not complete!" << std::endl;

    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    // set up the quad in wich i display the blured scene
    float quadVertices[] = {
        // Positions      // Texture Coords
        -1.0f,  1.0f,  0.0f,  1.0f,
        -1.0f, -1.0f,  0.0f,  0.0f,
         1.0f, -1.0f,  1.0f,  0.0f,

        -1.0f,  1.0f,  0.0f,  1.0f,
         1.0f, -1.0f,  1.0f,  0.0f,
         1.0f,  1.0f,  1.0f,  1.0f
    };

    
    glGenVertexArrays(1, &quadVAO);
    glGenBuffers(1, &quadVBO);
    glBindVertexArray(quadVAO);
    glBindBuffer(GL_ARRAY_BUFFER, quadVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(quadVertices), &quadVertices, GL_STATIC_DRAW);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)(2 * sizeof(float)));


    // glfw: swap buffers and poll IO events (keys pressed/released, mouse moved etc.)
    // -------------------------------------------------------------------------------
    glfwSwapBuffers(window);
    glfwPollEvents();
}

void free() {
    glfwTerminate();
}



void mainLoop() {
    float t = static_cast<float>(glfwGetTime());
    float rottime = 0;
    int freeze = 0;
    camera->position = vec3(0, 0, 0);
    
    // Set dust cloud center and radius
    glm::vec3 dust_center(-300.0f, 40.0f, -300.0f); // Example center position
    glm::vec3 dust_center2(150.0f, 30.0f, -150.0f);
    float dust_radius = 200.0f; // Example radius
    float dust_radius2 = 150.0f; // Example radius
    auto* snowball = new Drawable("models/sphere.obj");
    
    DustEmitter dust_emitter1 = DustEmitter(snowball, 10000, dust_center, dust_radius); // snow in Parnitha
    DustEmitter dust_emitter2 = DustEmitter(snowball, 5000, dust_center2, dust_radius2); // snow in Pendeli
    do {
        // calculate dt
        float currentTime = static_cast<float>(glfwGetTime());
        float dt = currentTime - t;
       
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        
        glBindFramebuffer(GL_FRAMEBUFFER, framebuffer);  // Bind the FBO for blur
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);


        // camera
        float h = getHeight(camera->position.x, camera->position.z);

        camera->update();
        // change speed due to high altitude
        camera->speed = camspeedmax * (1 - 0.9f *h / 100.0f); // so that the speed range is [max/10 , max]
        // make the camera follow the terrain
        camera->position.y = h + humanHeight; 

        // implement view bobbing
        // different frequencies because i dont want a synchronized oscillation, i want it to seem random
        vec3 bobbing = vec3(0.05f * sin(50 * t), 0.005f * sin(55 * t), 0.05f * sin(45 * t));  
        camera->position += bobbing * (h / 100); // because the more high i am, the more bobbing i want

        mat4 projectionMatrix = camera->projectionMatrix;
        mat4 viewMatrix = camera->viewMatrix; 
        
        // sound
        DWORD newVolume = (h * 0xFFFF) / 100.0f; // change volume depending on the height
        waveOutSetVolume(0, (0x1111 << 16) | newVolume);
        
     
        
        
        t += dt;

        glEnable(GL_BLEND);
        //glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
        // draw terrain
        
        //glFrontFace(GL_CW); 

        glUseProgram(terrain_shaderProgram);
        
        
        // Activate texture0
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, terrainTexture);
        
        // Activate texture1
        glActiveTexture(GL_TEXTURE1);
        glBindTexture(GL_TEXTURE_2D, snowTexture);
        
        // Activate texture2 
        glActiveTexture(GL_TEXTURE2);
        glBindTexture(GL_TEXTURE_2D, waterTexture);

        glUniform1i(terrainTextureSampler, 0);
        glUniform1i(snowTextureSampler, 1);
        glUniform1i(waterTextureSampler, 2);
        glUniform1f(timeUniform, t/10);

        // toggle rotational distortion
        if (glfwGetKey(window, GLFW_KEY_Z) == GLFW_PRESS) {
            rotationalDistortion = 1- rotationalDistortion;
            rottime = 0;
        }
        // freeze time in distortion
        if (glfwGetKey(window, GLFW_KEY_X) == GLFW_PRESS) {
            freeze = 1 - freeze;
        }
        if (rotationalDistortion == 1) {
            if (freeze == 0) { rottime += dt; }
        }
        glUniform1f(rottimeUniform, rottime);
        glUniform1i(rotUniform, rotationalDistortion);

        // toggle voxelising 
        if (glfwGetKey(window, GLFW_KEY_V) == GLFW_PRESS) { // voxelising on-off
            if (voxelising == 0) { voxelising = 1; }
            else{ voxelising = 0; }
        }
        if (glfwGetKey(window, GLFW_KEY_B) == GLFW_PRESS) {
            if (voxelising == 1) { voxelising = 2; }  // gap between voxels
            else if (voxelising == 2) { voxelising = 1; }
        }
        if (glfwGetKey(window, GLFW_KEY_G) == GLFW_PRESS) {
            if (voxelising == 1) { voxelising = 3; }  // another way
            else if (voxelising == 2) { voxelising = 3; }
            else if (voxelising == 3) { voxelising = 1; }
        }
        glUniform1i(voxUniform, voxelising);
        
        sun.lightDirection = normalize(vec3(sin(t), cos(t), 0.0f ));  // moves left and right , up and down , Static in the z-axis
        glUniform3f(lightDirectionLoc,  sun.lightDirection.x , sun.lightDirection.y , sun.lightDirection.z);
        
        glBindVertexArray(terrainVAO); 

        
        
        // the model matrix of the terrain is identical to the world 
       
        // Set uniform matrices (model, view, projection)
        glUniformMatrix4fv(terrainModelLoc, 1, GL_FALSE, &terrainmodel[0][0]);
        glUniformMatrix4fv(terrainViewLoc, 1, GL_FALSE, &viewMatrix[0][0]);
        glUniformMatrix4fv(terrainProjLoc, 1, GL_FALSE, &projectionMatrix[0][0]);


        glDrawElements(GL_TRIANGLES, indices.size(), GL_UNSIGNED_INT, 0);
        
        //glFrontFace(GL_CCW); 

        // VEGETATION 
        glUseProgram(veg_shaderProgram);
        glUniform1f(vegtimeUniform, t);
        glUniform1i(vegrotUniform, rotationalDistortion);
        glUniform1i(vegvoxUniform, voxelising);
        glUniform1f(vegrottimeUniform, rottime);
        for (const auto& modelMatrix : treeInstances) {
            // Combine matrices for this instance
            glUniformMatrix4fv(vegModelLoc, 1, GL_FALSE, &modelMatrix[0][0]);
            glUniformMatrix4fv(vegViewLoc, 1, GL_FALSE, &viewMatrix[0][0]);
            glUniformMatrix4fv(vegProjLoc, 1, GL_FALSE, &projectionMatrix[0][0]);
            // **Render Stem**
            glUniform1i(vegcolorUniform, 0);
            glActiveTexture(GL_TEXTURE0); 
            glBindTexture(GL_TEXTURE_2D, stemTexture); 
            glUniform1i(vegtextureSampler, 0);

            glBindVertexArray(stemVAO); 
            glDrawArrays(GL_TRIANGLES, 0, stemVertices.size()); 
            
            // **Render Leaves**
            glUniform1i(vegcolorUniform, 1);
            glActiveTexture(GL_TEXTURE0);
            glBindTexture(GL_TEXTURE_2D, leavesTexture);
            glUniform1i(vegtextureSampler, 0);

            glBindVertexArray(leavesVAO);
            glDrawArrays(GL_TRIANGLES, 0, leavesVertices.size());
        }

        // PARTICLES 

        glUseProgram(particleShaderProgram);
        mat4 PV = projectionMatrix * viewMatrix;
        glUniformMatrix4fv(pvMatrixloc, 1, GL_FALSE, &PV[0][0]);
        dust_emitter1.updateParticles(currentTime, dt, camera->position);
        dust_emitter1.renderParticles();
        dust_emitter2.updateParticles(currentTime, dt, camera->position);
        dust_emitter2.renderParticles();
        
        // SKYBOX draw

        //glDepthFunc(GL_LEQUAL); // Depth test passes if fragment depth <= current depth
        glDisable(GL_CULL_FACE); // because the side faces of the cube were only visible from outside

        glUseProgram(skybox_shaderProgram);

        mat4 view = mat4(mat3(viewMatrix)); // remove translation so the skybox seems to be far away

        glUniformMatrix4fv(skyViewLoc, 1, GL_FALSE, &view[0][0]);
        glUniformMatrix4fv(skyProjLoc, 1, GL_FALSE, &projectionMatrix[0][0]);
        glUniformMatrix4fv(skyModelLoc, 1, GL_FALSE, &skyModelMat[0][0]);

        glBindVertexArray(skyboxVAO);
        glBindTexture(GL_TEXTURE_CUBE_MAP, skyboxTexture);
        glDrawArrays(GL_TRIANGLES, 0, 36);
        glBindVertexArray(0);
        //glDepthFunc(GL_LESS); // Restore default depth function
        glEnable(GL_CULL_FACE);

    
        glBindVertexArray(0);

        glBindFramebuffer(GL_FRAMEBUFFER, 0);  // Unbind the FBO

        // Bind the motion blur shader
        glUseProgram(blur_shaderProgram);
        glBindVertexArray(quadVAO);

        // Pass the framebuffer texture to the shader
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, textureColorBuffer);
        
        glUniform1i(screenTextureloc, 0);

        // compute the blur depending on the direction of the camera and direction of the wind
        float blurfactor = dot(normalize(-winddirection), normalize(camera->direction)); // i want the max blur=1 when when the camera is facing the wind
        blurfactor = std::max(0.0f,blurfactor); // because i dont want blur when i turn my back on the wind
        
        glUniform1f(blurFactorloc, blurfactor);       
        glUniform1f(windSpeedloc, h/100.0f); // max speed = 1 when i am at the highest peak

        // Render the quad
        glDrawArrays(GL_TRIANGLES, 0, 6);


        glfwSwapBuffers(window);
        glfwPollEvents();
    } while (glfwGetKey(window, GLFW_KEY_ESCAPE) != GLFW_PRESS &&
             glfwWindowShouldClose(window) == 0);
}

void initialize() {
    // Initialize GLFW
    if (!glfwInit()) {
        throw runtime_error("Failed to initialize GLFW\n");
    }

    glfwWindowHint(GLFW_SAMPLES, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE); // To make MacOS happy
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    // Open a window and create its OpenGL context
    window = glfwCreateWindow(W_WIDTH, W_HEIGHT, TITLE, NULL, NULL);
    if (window == NULL) {
        glfwTerminate();
        throw runtime_error(string(string("Failed to open GLFW window.") +
                            " If you have an Intel GPU, they are not 3.3 compatible." +
                            "Try the 2.1 version.\n"));
    }
    glfwMakeContextCurrent(window);

    // Start GLEW extension handler
    glewExperimental = GL_TRUE;

    // Initialize GLEW
    if (glewInit() != GLEW_OK) {
        glfwTerminate();
        throw runtime_error("Failed to initialize GLEW\n");
    }

    // Ensure we can capture the escape key being pressed below
    glfwSetInputMode(window, GLFW_STICKY_KEYS, GL_TRUE);

    // Hide the mouse and enable unlimited movement
    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

    // Set the mouse at the center of the screen
    glfwPollEvents();
    glfwSetCursorPos(window, W_WIDTH / 2, W_HEIGHT / 2);

    // Gray background color
    glClearColor(0.5f, 0.5f, 0.5f, 0.0f);

    // Enable depth test
    glEnable(GL_DEPTH_TEST);
    // Accept fragment if it closer to the camera than the former one
    glDepthFunc(GL_LESS);

    // Cull triangles which normal is not towards the camera
    //glDisable (GL_CULL_FACE);
    glEnable(GL_CULL_FACE);
    //glFrontFace(GL_CW);
    // glFrontFace(GL_CCW);

    // enable point size when drawing points
    glEnable(GL_PROGRAM_POINT_SIZE);

    // Log
    logGLParameters();

    // Create camera
    camera = new Camera(window);
}



int main(void) {
    try {
        initialize();
        createContext();
        mainLoop();
        free();
    } catch (exception& ex) {
        cout << ex.what() << endl;
        getchar();
        free();
        return -1;
    }
    return 0;
}