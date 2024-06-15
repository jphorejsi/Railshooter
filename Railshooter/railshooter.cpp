#include <glad/glad.h>
#include <GLFW/glfw3.h>
#define STB_IMAGE_IMPLEMENTATION


#include <map>
#include <string>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <learnopengl/shader_m.h>
#include <learnopengl/model.h>
#include <iostream>

//random
#include <cstdlib>
#include <ctime>

#include <ft2build.h>
#include FT_FREETYPE_H  


float RandomFloat(float a, float b) {
    float random = ((float)rand()) / (float)RAND_MAX;
    float diff = b - a;
    float r = random * diff;
    return a + r;
}

struct Projectile {
    glm::vec3 position;
    glm::vec3 velocity;
    glm::vec3 scale; //implement
    bool active = true;
    float pitch;
    float yaw;
    float roll;
};

struct Building {
    glm::vec3 position;
    glm::vec3 scale;
    bool active = false;
};

struct Meteor {
    glm::vec3 position;
    glm::vec3 velocity;
    glm::vec3 scale;
    bool active = false;
    float rotationValue = 0;
    float rotationVelocity;
};

struct Particle {
    glm::vec3 position;
    glm::vec3 velocity;
    glm::vec3 scale;
    float lifeSpan = 3.0f;
    bool active = false;
};

struct Stud {
    glm::vec3 position;
    bool active = false;
};

/// Holds all state information relevant to a character as loaded using FreeType
struct Character {
    unsigned int TextureID; // ID handle of the glyph texture
    glm::ivec2   Size;      // Size of glyph
    glm::ivec2   Bearing;   // Offset from baseline to left/top of glyph
    unsigned int Advance;   // Horizontal offset to advance to next glyph
};

std::map<GLchar, Character> Characters;

std::vector<Projectile> projectiles;
std::vector<Building> buildings;
std::vector<Meteor> meteors;
std::vector<Particle> particles;
std::vector<Stud> studs;


void framebuffer_size_callback(GLFWwindow* window, int width, int height);
void processInput(GLFWwindow* window);

// settings
const unsigned int SCR_WIDTH = 800;
const unsigned int SCR_HEIGHT = 600;
float threshold = 0.001;
int maxBuildingCount = 20;
int maxBuildingATime = 3;
int maxMeteorCount = 20;
int maxMeteorATime = 5;
float speed = 120.0f;
int score = 0;
int health = 3;

// timing
float buildingElapsedTime = 0.0f;
float meteorElapsedTime = 0.0f;
float studsElapsedTime = 0.0f;
float deltaTime = 0.0f;
float lastFrame = 0.0f;
float buildingSpawnMinTime = 1.0f;
float buildingSpawnMaxTime = 5.0f;
float meteorSpawnMinTime = 1.0f;
float meteorSpawnMaxTime = 5.0f;

//ship
glm::vec3 shipUp;
glm::vec3 shipFront;
glm::vec3 shipPosition = glm::vec3(0.0f, 50.0f, 0.0f);

//camera
glm::vec3 cameraPosition;
glm::vec3 cameraUp;
glm::vec3 cameraFront;
float trail = 0.99f;

//control
bool spaceWasPressed = false;
float pitch = 0.0f, yaw = 0.0f, roll = 0.0f;

//particles
int spawnParticles = 10;
int maxParticleCount = 100;

//studs
int studsWidth = 8;
int maxStuds = 42;

//text
unsigned int VAO, VBO;

void spawnParts(glm::vec3 position) {
    int particlesToSpawn = std::min(spawnParticles, static_cast<int>(particles.size()));
    for (int i = 0; i < particlesToSpawn; i++) {
        for (auto& particle : particles) {
            if (!particle.active) {
                particle.active = true;
                particle.lifeSpan = 3.0f;
                particle.position = position;
                particle.velocity = glm::vec3(RandomFloat(-10.0f, 10.0f), RandomFloat(-10.0f, 10.0f), RandomFloat(-10.0f, 10.0f));
                break; // Exit inner loop once particle is spawned
            }
        }
    }
}

void spawnPartsTowards(glm::vec3 position) {
    int particlesToSpawn = std::min(spawnParticles, static_cast<int>(particles.size()));
    for (int i = 0; i < particlesToSpawn; i++) {
        for (auto& particle : particles) {
            if (!particle.active) {
                particle.active = true;
                particle.lifeSpan = 3.0f;
                particle.position = position;
                particle.velocity = glm::vec3(RandomFloat(-10.0f, 10.0f), RandomFloat(-10.0f, 10.0f), RandomFloat(-100.0f, 0.0f));
                break; // Exit inner loop once particle is spawned
            }
        }
    }
}


void projectileMeteorCollision() {
    for (auto& meteor : meteors) {
        if (meteor.active == false) {
            continue;
        }
        for (auto& projectile : projectiles) {
            if (projectile.active == false) {
                continue;
            }
            float meteorX = meteor.scale.x * 40;
            float meteorY = meteor.scale.y * 40;
            float meteorZ = meteor.scale.z = 2;
            float projectileX = 1;
            float projectileY = 1;
            float projectileZ = 1;
            glm::vec3 projectileSize = glm::vec3(projectileX, projectileY, projectileZ);
            glm::vec3 meteorSize = glm::vec3(meteorX, meteorY, meteorZ);
            glm::vec3 projMin = projectile.position - (projectileSize * 0.5f);
            glm::vec3 projMax = projectile.position + (projectileSize * 0.5f);
            glm::vec3 meteorMin = meteor.position - (meteorSize * 0.5f);
            glm::vec3 meteorMax = meteor.position + (meteorSize * 0.5f);
            bool overlapX = projMax.x >= meteorMin.x && projMin.x <= meteorMax.x;
            bool overlapY = projMax.y >= meteorMin.y && projMin.y <= meteorMax.y;
            bool overlapZ = projMax.z >= meteorMin.z && projMin.z <= meteorMax.z;
            if (overlapX && overlapY && overlapZ) {
                score += 1;
                meteor.active = false;
                projectile.active = false;
                spawnParts(meteor.position);
            }
        }
    }
}

void shipMeteorCollision() {
    for (auto& meteor : meteors) {
        if (meteor.active == false) {
            continue;
        }
        float meteorX = meteor.scale.x * 40;
        float meteorY = meteor.scale.y * 40;
        float meteorZ = meteor.scale.z = 2;
        glm::vec3 shipSize = glm::vec3(1, 1, 1);
        glm::vec3 meteorSize = glm::vec3(meteorX, meteorY, meteorZ);
        glm::vec3 shipMin = shipPosition - (shipSize * 0.5f);
        glm::vec3 shipMax = shipPosition + (shipSize * 0.5f);
        glm::vec3 meteorMin = meteor.position - (meteorSize * 0.5f);
        glm::vec3 meteorMax = meteor.position + (meteorSize * 0.5f);
        bool overlapX = shipMax.x >= meteorMin.x && shipMin.x <= meteorMax.x;
        bool overlapY = shipMax.y >= meteorMin.y && shipMin.y <= meteorMax.y;
        bool overlapZ = shipMax.z >= meteorMin.z && shipMin.z <= meteorMax.z;
        if (overlapX && overlapY && overlapZ) {
            health -= 1;
            meteor.active = false;
            spawnPartsTowards(shipPosition);
        }
    }
}

// render line of text
// -------------------
void RenderText(Shader& shader, std::string text, float x, float y, float scale, glm::vec3 color)
{
    // activate corresponding render state	
    shader.use();
    glUniform3f(glGetUniformLocation(shader.ID, "textColor"), color.x, color.y, color.z);
    glActiveTexture(GL_TEXTURE0);
    glBindVertexArray(VAO);

    // iterate through all characters
    std::string::const_iterator c;
    for (c = text.begin(); c != text.end(); c++)
    {
        Character ch = Characters[*c];

        float xpos = x + ch.Bearing.x * scale;
        float ypos = y - (ch.Size.y - ch.Bearing.y) * scale;

        float w = ch.Size.x * scale;
        float h = ch.Size.y * scale;
        // update VBO for each character
        float vertices[6][4] = {
            { xpos,     ypos + h,   0.0f, 0.0f },
            { xpos,     ypos,       0.0f, 1.0f },
            { xpos + w, ypos,       1.0f, 1.0f },

            { xpos,     ypos + h,   0.0f, 0.0f },
            { xpos + w, ypos,       1.0f, 1.0f },
            { xpos + w, ypos + h,   1.0f, 0.0f }
        };
        // render glyph texture over quad
        glBindTexture(GL_TEXTURE_2D, ch.TextureID);
        // update content of VBO memory
        glBindBuffer(GL_ARRAY_BUFFER, VBO);
        glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(vertices), vertices); // be sure to use glBufferSubData and not glBufferData

        glBindBuffer(GL_ARRAY_BUFFER, 0);
        // render quad
        glDrawArrays(GL_TRIANGLES, 0, 6);
        // now advance cursors for next glyph (note that advance is number of 1/64 pixels)
        x += (ch.Advance >> 6) * scale; // bitshift by 6 to get value in pixels (2^6 = 64 (divide amount of 1/64th pixels by 64 to get amount of pixels))
    }
    glBindVertexArray(0);
    glBindTexture(GL_TEXTURE_2D, 0);
}


int main()
{
    // glfw: initialize and configure
    // ------------------------------
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

#ifdef __APPLE__
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
#endif

    // glfw window creation
    // --------------------
    GLFWwindow* window = glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT, "LearnOpenGL", NULL, NULL);
    if (window == NULL)
    {
        std::cout << "Failed to create GLFW window" << std::endl;
        glfwTerminate();
        return -1;
    }
    glfwMakeContextCurrent(window);
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);

    // tell GLFW to capture our mouse
    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

    // glad: load all OpenGL function pointers
    // ---------------------------------------
    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
    {
        std::cout << "Failed to initialize GLAD" << std::endl;
        return -1;
    }

    // tell stb_image.h to flip loaded texture's on the y-axis (before loading model).
    stbi_set_flip_vertically_on_load(true);

    // configure global opengl state
    // -----------------------------
    glEnable(GL_DEPTH_TEST);

    // build and compile shaders
    // -------------------------
    Shader ourShader("1.model_loading.vs", "1.model_loading.fs");
    Shader groundShader("ground.vs", "ground.fs");
    Shader skyShader("sky.vs", "sky.fs");
    Shader flatShader("flat.vs", "flat.fs");
    Shader meteorShader("meteor.vs", "meteor.fs");
    Shader particleShader("particle.vs", "particle.fs");

    Shader textShader("text.vs","text.fs");
    glm::mat4 projection = glm::ortho(0.0f, static_cast<float>(SCR_WIDTH), 0.0f, static_cast<float>(SCR_HEIGHT));
    textShader.use();
    glUniformMatrix4fv(glGetUniformLocation(textShader.ID, "projection"), 1, GL_FALSE, glm::value_ptr(projection));

    // load models
    Model ourModel("resources/objects/arwing/arwing SNES.obj");
    Model projectileModel("resources/objects/projectile/projectile2.obj");

    
    // FreeType
      // --------
    FT_Library ft;
    // All functions return a value different than 0 whenever an error occurred
    if (FT_Init_FreeType(&ft))
    {
        std::cout << "ERROR::FREETYPE: Could not init FreeType Library" << std::endl;
        return -1;
    }

    // find path to font
    std::string font_name = "resources/fonts/arial.ttf";
    if (font_name.empty())
    {
        std::cout << "ERROR::FREETYPE: Failed to load font_name" << std::endl;
        return -1;
    }

    // load font as face
    FT_Face face;
    if (FT_New_Face(ft, font_name.c_str(), 0, &face)) {
        std::cout << "ERROR::FREETYPE: Failed to load font" << std::endl;
        return -1;
    }
    else {
        // set size to load glyphs as
        FT_Set_Pixel_Sizes(face, 0, 48);

        // disable byte-alignment restriction
        glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

        // load first 128 characters of ASCII set
        for (unsigned char c = 0; c < 128; c++)
        {
            // Load character glyph 
            if (FT_Load_Char(face, c, FT_LOAD_RENDER))
            {
                std::cout << "ERROR::FREETYTPE: Failed to load Glyph" << std::endl;
                continue;
            }
            // generate texture
            unsigned int texture;
            glGenTextures(1, &texture);
            glBindTexture(GL_TEXTURE_2D, texture);
            glTexImage2D(
                GL_TEXTURE_2D,
                0,
                GL_RED,
                face->glyph->bitmap.width,
                face->glyph->bitmap.rows,
                0,
                GL_RED,
                GL_UNSIGNED_BYTE,
                face->glyph->bitmap.buffer
            );
            // set texture options
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
            // now store character for later use
            Character character = {
                texture,
                glm::ivec2(face->glyph->bitmap.width, face->glyph->bitmap.rows),
                glm::ivec2(face->glyph->bitmap_left, face->glyph->bitmap_top),
                static_cast<unsigned int>(face->glyph->advance.x)
            };
            Characters.insert(std::pair<char, Character>(c, character));
        }
        glBindTexture(GL_TEXTURE_2D, 0);
    }
    // destroy FreeType once we're finished
    FT_Done_Face(face);
    FT_Done_FreeType(ft);


    // configure VAO/VBO for texture quads
// -----------------------------------
    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);
    glBindVertexArray(VAO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(float) * 6 * 4, NULL, GL_DYNAMIC_DRAW);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, 4 * sizeof(float), 0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);



    float squareVerts[] = { //verticies, normals, TCs
        -20.0f, 0.0f, 20.0f,  0.0f, 1.0f, 0.0f, 0.0f, 1.0f,  // Top Left
         20.0f, 0.0f, 20.0f,  0.0f, 1.0f, 0.0f, 1.0f, 1.0f, // Top Right
         20.0f, 0.0f, -20.0f, 0.0f, 1.0f, 0.0f, 1.0f, 0.0f, // Bottom Right

        -20.0f, 0.0f, 20.0f,  0.0f, 1.0f, 0.0f, 0.0f, 1.0f, // Top Left
         20.0f, 0.0f, -20.0f, 0.0f, 1.0f, 0.0f, 1.0f, 0.0f, // Bottom Right
        -20.0f, 0.0f, -20.0f, 0.0f, 1.0f, 0.0f,  0.0f, 0.0f // Bottom Left
    };

    float cubeVerts[] = {
        // positions          // normals           // texture coords
        -0.5f, -0.5f, -0.5f,  0.0f,  0.0f, -1.0f,  0.0f,  0.0f,
         0.5f, -0.5f, -0.5f,  0.0f,  0.0f, -1.0f,  1.0f,  0.0f,
         0.5f,  0.5f, -0.5f,  0.0f,  0.0f, -1.0f,  1.0f,  1.0f,
         0.5f,  0.5f, -0.5f,  0.0f,  0.0f, -1.0f,  1.0f,  1.0f,
        -0.5f,  0.5f, -0.5f,  0.0f,  0.0f, -1.0f,  0.0f,  1.0f,
        -0.5f, -0.5f, -0.5f,  0.0f,  0.0f, -1.0f,  0.0f,  0.0f,

        -0.5f, -0.5f,  0.5f,  0.0f,  0.0f,  1.0f,  0.0f,  0.0f,
         0.5f, -0.5f,  0.5f,  0.0f,  0.0f,  1.0f,  1.0f,  0.0f,
         0.5f,  0.5f,  0.5f,  0.0f,  0.0f,  1.0f,  1.0f,  1.0f,
         0.5f,  0.5f,  0.5f,  0.0f,  0.0f,  1.0f,  1.0f,  1.0f,
        -0.5f,  0.5f,  0.5f,  0.0f,  0.0f,  1.0f,  0.0f,  1.0f,
        -0.5f, -0.5f,  0.5f,  0.0f,  0.0f,  1.0f,  0.0f,  0.0f,

        -0.5f,  0.5f,  0.5f, -1.0f,  0.0f,  0.0f,  1.0f,  0.0f,
        -0.5f,  0.5f, -0.5f, -1.0f,  0.0f,  0.0f,  1.0f,  1.0f,
        -0.5f, -0.5f, -0.5f, -1.0f,  0.0f,  0.0f,  0.0f,  1.0f,
        -0.5f, -0.5f, -0.5f, -1.0f,  0.0f,  0.0f,  0.0f,  1.0f,
        -0.5f, -0.5f,  0.5f, -1.0f,  0.0f,  0.0f,  0.0f,  0.0f,
        -0.5f,  0.5f,  0.5f, -1.0f,  0.0f,  0.0f,  1.0f,  0.0f,

         0.5f,  0.5f,  0.5f,  1.0f,  0.0f,  0.0f,  1.0f,  0.0f,
         0.5f,  0.5f, -0.5f,  1.0f,  0.0f,  0.0f,  1.0f,  1.0f,
         0.5f, -0.5f, -0.5f,  1.0f,  0.0f,  0.0f,  0.0f,  1.0f,
         0.5f, -0.5f, -0.5f,  1.0f,  0.0f,  0.0f,  0.0f,  1.0f,
         0.5f, -0.5f,  0.5f,  1.0f,  0.0f,  0.0f,  0.0f,  0.0f,
         0.5f,  0.5f,  0.5f,  1.0f,  0.0f,  0.0f,  1.0f,  0.0f,

        -0.5f, -0.5f, -0.5f,  0.0f, -1.0f,  0.0f,  0.0f,  1.0f,
         0.5f, -0.5f, -0.5f,  0.0f, -1.0f,  0.0f,  1.0f,  1.0f,
         0.5f, -0.5f,  0.5f,  0.0f, -1.0f,  0.0f,  1.0f,  0.0f,
         0.5f, -0.5f,  0.5f,  0.0f, -1.0f,  0.0f,  1.0f,  0.0f,
        -0.5f, -0.5f,  0.5f,  0.0f, -1.0f,  0.0f,  0.0f,  0.0f,
        -0.5f, -0.5f, -0.5f,  0.0f, -1.0f,  0.0f,  0.0f,  1.0f,

        -0.5f,  0.5f, -0.5f,  0.0f,  1.0f,  0.0f,  0.0f,  1.0f,
         0.5f,  0.5f, -0.5f,  0.0f,  1.0f,  0.0f,  1.0f,  1.0f,
         0.5f,  0.5f,  0.5f,  0.0f,  1.0f,  0.0f,  1.0f,  0.0f,
         0.5f,  0.5f,  0.5f,  0.0f,  1.0f,  0.0f,  1.0f,  0.0f,
        -0.5f,  0.5f,  0.5f,  0.0f,  1.0f,  0.0f,  0.0f,  0.0f,
        -0.5f,  0.5f, -0.5f,  0.0f,  1.0f,  0.0f,  0.0f,  1.0f
    };



    // draw in wireframe
    //glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);

    unsigned int squareVBO, cubeVBO, staticSquareVBO, staticCubeVBO;
    unsigned int staticFlatSquareVAO;

    unsigned int texturedSquareVAO;
    unsigned int staticTexturedSquareVAO;

    unsigned int flatCubeVAO;

    //squares
    glGenVertexArrays(1, &texturedSquareVAO); //Meteors
    glGenVertexArrays(1, &staticTexturedSquareVAO); //Sky
    glGenVertexArrays(1, &staticFlatSquareVAO); //Ground
    glGenBuffers(1, &squareVBO);
    glGenBuffers(1, &staticSquareVBO);
    
    //Create VAO and VBO for textured dynamic squares (METEORS)
    glBindBuffer(GL_ARRAY_BUFFER, squareVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(squareVerts), squareVerts, GL_DYNAMIC_DRAW);
    glBindVertexArray(texturedSquareVAO);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(6 * sizeof(float)));
    glEnableVertexAttribArray(1);

    //Create VAO and VBO for flat static squares (GROUND)
    glBindBuffer(GL_ARRAY_BUFFER, staticSquareVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(squareVerts), squareVerts, GL_STATIC_DRAW);
    glBindVertexArray(staticFlatSquareVAO);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);

    //Create VAO for textured static squares (SKY)
    glBindVertexArray(staticTexturedSquareVAO);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(6 * sizeof(float)));
    glEnableVertexAttribArray(1);

    //Cubes
    glGenVertexArrays(1, &flatCubeVAO); //buildings, particles and projectiles
    glGenBuffers(1, &cubeVBO);

    //Create VAO and VBO for dynamic cubes
    glBindBuffer(GL_ARRAY_BUFFER, cubeVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(cubeVerts), cubeVerts, GL_DYNAMIC_DRAW);
    glBindVertexArray(flatCubeVAO);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(6 * sizeof(float)));
    glEnableVertexAttribArray(2);

    //load and create textures
    unsigned int skyTexture;
    glActiveTexture(GL_TEXTURE0);
    glGenTextures(1, &skyTexture);
    glBindTexture(GL_TEXTURE_2D, skyTexture);
    // set the texture wrapping parameters
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);	// set texture wrapping to GL_REPEAT (default wrapping method)
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    // set texture filtering parameters
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    // load image, create texture and generate mipmaps
    int width, height, nrChannels;
    stbi_set_flip_vertically_on_load(true); // tell stb_image.h to flip loaded texture's on the y-axis.
    unsigned char* data = stbi_load("resources/textures/space2.jpg", &width, &height, &nrChannels, 0);
    if (data)
    {
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
        glGenerateMipmap(GL_TEXTURE_2D);
    }
    else
    {
        std::cout << "Failed to load texture" << std::endl;
    }
    stbi_image_free(data);
    skyShader.use();
    glUniform1i(glGetUniformLocation(skyShader.ID, "texture1"), 0);


    //now for meteor texture
    unsigned int meteorTexture;
    glActiveTexture(GL_TEXTURE1);
    glGenTextures(1, &meteorTexture);
    glBindTexture(GL_TEXTURE_2D, meteorTexture);
    // set the texture wrapping parameters
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);	// set texture wrapping to GL_REPEAT (default wrapping method)
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    // set texture filtering parameters
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    // load image, create texture and generate mipmaps
    stbi_set_flip_vertically_on_load(true); // tell stb_image.h to flip loaded texture's on the y-axis.
    data = stbi_load("resources/textures/meteor.png", &width, &height, &nrChannels, 0);
    if (data)
    {
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);
        glGenerateMipmap(GL_TEXTURE_2D);
    }
    else
    {
        std::cout << "Failed to load texture" << std::endl;
    }
    stbi_image_free(data);
    meteorShader.use();
    glUniform1i(glGetUniformLocation(meteorShader.ID, "texture1"), 1);


    for (int i = 0; i < maxBuildingCount; i++) {
        Building toPush;
        buildings.push_back(toPush);
    }

    for (int i = 0; i < maxMeteorCount; i++) {
        Meteor toPush;
        meteors.push_back(toPush);
    }

    for (int i = 0; i < maxParticleCount; i++) {
        Particle toPush;
        particles.push_back(toPush);
    }

    for (int i = 0; i < maxStuds; i++) {
        Stud toPush;
        studs.push_back(toPush);
    }

    // Initialize random seed
    std::srand(std::time(0));
    float buildingRandTime = RandomFloat(1.0f, 5.0f);
    float meteorRandTime = RandomFloat(1.0f, 5.0f);

    // render loop
    while (!glfwWindowShouldClose(window) && health > 0)
    {
        // per-frame time logic
        float currentFrame = static_cast<float>(glfwGetTime());
        deltaTime = currentFrame - lastFrame;
        lastFrame = currentFrame;
        buildingElapsedTime += deltaTime;
        meteorElapsedTime += deltaTime;
        studsElapsedTime += deltaTime;

        // input
        processInput(window);

        projectileMeteorCollision();
        shipMeteorCollision();

        // render
        glClearColor(0.05f, 0.05f, 0.05f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        // view/projection transformations
        glm::mat4 projection = glm::perspective(45.0f, (float)SCR_WIDTH / (float)SCR_HEIGHT, 0.1f, 450.0f);
        //Find and set ship vectors
        glm::mat4 rotationYaw = glm::rotate(glm::mat4(1.0f), glm::radians(yaw), glm::vec3(0.0f, 1.0f, 0.0f)); // Yaw around y-axis
        glm::mat4 rotationPitch = glm::rotate(glm::mat4(1.0f), glm::radians(pitch), glm::vec3(1.0f, 0.0f, 0.0f)); // Pitch around x-axis
        glm::mat4 rotationRoll = glm::rotate(glm::mat4(1.0f), glm::radians(roll), glm::vec3(0.0f, 0.0f, -1.0f)); // Roll around -z axis
        glm::mat4 cameraRotation = rotationPitch * rotationRoll;
        glm::mat4 combinedRotation = rotationYaw * rotationPitch * rotationRoll;
        shipFront = glm::normalize(glm::vec3(combinedRotation * glm::vec4(0.0f, 0.0f, -1.0f, 1.0f)));
        shipUp = glm::normalize(glm::vec3(combinedRotation * glm::vec4(0.0f, 1.0f, 0.0f, 1.0f)));
        //Find and set camera vectors
        cameraUp = glm::normalize(glm::vec3(rotationPitch * rotationRoll * glm::vec4(0.0f, 1.0f, 0.0f, 1.0f)));
        cameraFront = glm::normalize(glm::vec3(rotationPitch * glm::vec4(0.0f, 0.0f, -1.0f, 1.0f)));
        cameraFront = cameraFront * 0.5f;
        //Update ship position
        shipPosition.x += shipFront.x * deltaTime * speed;
        shipPosition.y += shipFront.y * deltaTime * speed;
        //limit position
        shipPosition.x = glm::clamp(shipPosition.x, -50.0f, 50.0f);
        shipPosition.y = glm::clamp(shipPosition.y, 1.0f, 100.0f);

        //Calculate trailing camera position
        cameraPosition.x = (shipPosition.x - 0.0f) * trail;
        cameraPosition.y = (shipPosition.y - 50.0f) * trail;
        // Update the view matrix
        glm::mat4 view = glm::lookAt(
            glm::vec3(cameraPosition.x, cameraPosition.y + 50, 1.5f), // Camera position
            glm::vec3(cameraPosition.x + cameraFront.x, cameraPosition.y + 50 + cameraFront.y, -1.5f + cameraFront.z), //look ahead
            glm::vec3(cameraUp.x * -0.25f, cameraUp.y, cameraUp.z) // Up direction
        );

        // Update projectiles
        for (auto& proj : projectiles) {
            if (!proj.active) continue;
            proj.position += proj.velocity * deltaTime;
            if (proj.position.y > 150 || proj.position.y < -1 || proj.position.z < -300) proj.active = false; // Deactivate when out of bounds
        }

        // Update particles
        for (auto& particle : particles) {
            if (!particle.active) {
                continue;
            }
            if (particle.lifeSpan <= 0) {
                particle.active = false;
                break;
            }
            particle.lifeSpan -= deltaTime;
            particle.position += particle.velocity * deltaTime;
            particle.position.z += speed * deltaTime;
            if (particle.position.y > 150 || particle.position.y < -1 || particle.position.z < -300 || particle.position.z > 5) particle.active = false;
        }

        // Create buildings
        if (buildingElapsedTime >= buildingRandTime) {
            buildingRandTime = RandomFloat(buildingSpawnMinTime, buildingSpawnMaxTime);
            buildingElapsedTime = 0.0f;

            // Generate a random number (1 or 2)
            int randValue = (std::rand() % maxBuildingATime) + 1;
            for (int i = 0; i < randValue; i++) {
                for (auto& build : buildings) {
                    if (!build.active) {
                        build.active = true;
                        build.position = glm::vec3((std::rand() % 141) - 70, 0, -400); //random x variable
                        float randomScaleX = (std::rand() % 16) + 5;
                        float randomScaleY = (std::rand() % 16) + 5;
                        float randomScaleZ = (std::rand() % 16) + 5;
                        build.scale = glm::vec3(randomScaleX, randomScaleY, randomScaleZ);
                        break;
                    }
                }
            }
        }

        // Create meteors
        if (meteorElapsedTime >= meteorRandTime) {
            meteorRandTime = RandomFloat(1.0f, 5.0f);
            meteorElapsedTime = 0.0f;

            // Generate a random number (1 or 2)
            int randomValue = (std::rand() % 2) + 1;
            for (int i = 0; i < randomValue; i++) {
                for (auto& meteor : meteors) {
                    if (!meteor.active) {
                        meteor.active = true;
                        meteor.rotationVelocity = RandomFloat(-100.0, 100.0f);
                        meteor.position = glm::vec3((std::rand() % 141) - 70, (std::rand() % 61 + 20), -400);
                        float randomScale = RandomFloat(0.3, 0.7);
                        meteor.scale = glm::vec3(randomScale, randomScale, randomScale);
                        float randomXVelocity = RandomFloat(-10.0f, 10.0f);
                        float randomYVelocity = RandomFloat(-1.0f, 10.0f);
                        float randomZVelocity = RandomFloat(-10.0f, 10.0f);
                        meteor.velocity = glm::vec3(randomXVelocity, randomYVelocity, randomZVelocity);
                        break;
                    }
                }
            }
        }

        //Create studs
        if (studsElapsedTime >= 1.0f) {
            studsElapsedTime = 0.0f;
            for (int i = 0; i < studsWidth; i++) {
                float ID = -1;
                for (int j = 0; j < studs.size(); j++) {
                    if (!studs[j].active) {
                        ID = j;
                        break;
                    }
                }
                if (ID == -1) {
                    std::cout << "bad\n";
                    ID = 1;
                }
                studs[ID].active = true;
                studs[ID].position = glm::vec3(i * 50 - 175, 0, -300);
            }
        }

        //Update studs
        for (auto& stud : studs) {
            if (stud.active) {
                stud.position.z += 120.0f * deltaTime;
                if (stud.position.z > 5) {
                    stud.active = false;
                }
            }
        }


        //Update buildings
        for (auto& build : buildings) {
            if (build.active) {
                build.position.z += 120.0f * deltaTime;
                if (build.position.z > 5) {
                    build.active = false;
                }
            }
        }

        //Update meteors
        for (auto& meteor : meteors) {
            if (meteor.active) {
                meteor.position.x += meteor.velocity.x * deltaTime;
                meteor.position.y += meteor.velocity.y * deltaTime;
                meteor.position.z += (meteor.velocity.z + 120.f) * deltaTime;
                meteor.rotationValue += meteor.rotationVelocity * deltaTime;
                if (meteor.position.z > 5) {
                    meteor.active = false;
                }
            }
        }


        // Draw the model
        ourShader.use();
        ourShader.setMat4("projection", projection);
        ourShader.setMat4("view", view);
        glm::mat4 model = glm::mat4(1.0f);
        model = glm::translate(model, glm::vec3(shipPosition.x, shipPosition.y, 0.0f));
        model = glm::scale(model, glm::vec3(0.05f, 0.05f, 0.05f));
        model = glm::rotate(model, glm::radians(pitch), glm::vec3(1.0f, 0.0f, 0.0f));
        model = glm::rotate(model, glm::radians(yaw), glm::vec3(0.0f, 1.0f, 0.0f));
        model = glm::rotate(model, glm::radians(roll), glm::vec3(0.0f, 0.0f, 1.0f));
        ourShader.setMat4("model", model);
        ourModel.Draw(ourShader);

        //Draw projectiles
        flatShader.use();
        for (const auto& proj : projectiles) {
            if (!proj.active)
                continue;
            flatShader.setMat4("projection", projection);
            flatShader.setMat4("view", view);
            flatShader.setVec3("color", glm::vec3(0, 183.0f/255.0f, 255.0f/255.0f));
            model = glm::mat4(1.0f);
            model = glm::translate(model, proj.position);
            model = glm::scale(model, glm::vec3(0.15f, 0.15f, 0.15f));
            model = glm::rotate(model, glm::radians(proj.pitch), glm::vec3(1, 0, 0));
            model = glm::rotate(model, glm::radians(proj.yaw), glm::vec3(0, 1, 0));
            model = glm::rotate(model, glm::radians(proj.roll), glm::vec3(0, 0, 1));
            model = glm::rotate(model, glm::radians(90.0f), glm::vec3(1, 0, 0));
            flatShader.setMat4("model", model);
            projectileModel.Draw(flatShader);
        }

        // Draw buildings
        flatShader.use();
        glBindVertexArray(flatCubeVAO);
        for (const auto& build : buildings) {
            if (!build.active)
                continue;
            flatShader.setMat4("projection", projection);
            flatShader.setMat4("view", view);
            flatShader.setVec3("color", glm::vec3(89.0f/255.0f, 89.0f/255.0f, 89.0f/255.0f));
            glm::mat4 model = glm::mat4(1.0f);
            model = glm::translate(model, glm::vec3(build.position.x, build.scale.y /2, build.position.z));
            model = glm::scale(model, build.scale);
            flatShader.setMat4("model", model);
            glDrawArrays(GL_TRIANGLES, 0, 36);
        }

        //Draw particles
        particleShader.use();
        glBindVertexArray(flatCubeVAO);
        for (const auto& particle : particles) {
            if (!particle.active)
                continue;
            particleShader.setMat4("projection", projection);
            particleShader.setMat4("view", view);
            model = glm::mat4(1.0f);
            model = glm::translate(model, particle.position);
            model = glm::scale(model, glm::vec3(0.5f, 0.5f, 0.5f));
            particleShader.setMat4("model", model);
            glDrawArrays(GL_TRIANGLES, 0, 36);
        }

        // Draw meteors
        meteorShader.use();
        glActiveTexture(GL_TEXTURE1);
        glBindTexture(GL_TEXTURE_2D, meteorTexture);
        glBindVertexArray(texturedSquareVAO);
        for (const auto& meteor : meteors) {
            if (!meteor.active)
                continue;
            meteorShader.setMat4("projection", projection);
            meteorShader.setMat4("view", view);
            glm::mat4 model = glm::mat4(1.0f);
            model = glm::translate(model, meteor.position);
            model = glm::scale(model, meteor.scale);
            model = glm::rotate(model, glm::radians(90.0f), glm::vec3(1.0f, 0.0f, 0.0f));
            model = glm::rotate(model, glm::radians(meteor.rotationValue), glm::vec3(0.0f, 1.0f, 0.0f));
            meteorShader.setMat4("model", model);
            glDrawArrays(GL_TRIANGLES, 0, 6);
        }

        //Draw studs
        flatShader.use();
        glBindVertexArray(flatCubeVAO);
        for (const auto& stud : studs) {
            if (!stud.active) {
                continue;
            }
            flatShader.setVec3("color", glm::vec3(1.0f, 1.0f, 1.0f));
            flatShader.setMat4("projection", projection);
            flatShader.setMat4("view", view);
            glm::mat4 model = glm::mat4(1.0f);
            model = glm::translate(model, stud.position);
            //model = glm::scale(model, meteor.scale);
            flatShader.setMat4("model", model);
            glDrawArrays(GL_TRIANGLES, 0, 36);
        }

        // Draw the ground
        groundShader.use();
        groundShader.setMat4("projection", projection);
        groundShader.setMat4("view", view);
        model = glm::mat4(1.0f);
        model = glm::translate(model, glm::vec3(0.0f, 0.0f, -190.0f));
        model = glm::scale(model, glm::vec3(18.0f, 1.0f, 10.0f));
        groundShader.setMat4("model", model);
        glBindVertexArray(staticFlatSquareVAO);
        glDrawArrays(GL_TRIANGLES, 0, 6);

        // Draw the sky
        skyShader.use();
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, skyTexture);
        skyShader.setMat4("projection", projection);
        skyShader.setMat4("view", view);
        model = glm::mat4(1.0f);
        model = glm::translate(model, glm::vec3(0.0f, 50.0f, -400.0f));
        model = glm::rotate(model, glm::radians(90.0f), glm::vec3(1.0f, 0.0f, 0.0f));
        model = glm::scale(model, glm::vec3(20.0f, 1.0f, 20.0f));
        skyShader.setMat4("model", model);
        glBindVertexArray(staticTexturedSquareVAO);
        glDrawArrays(GL_TRIANGLES, 0, 6);

        //draw text
        std::string scoreText = "SCORE: " + std::to_string(score);
        std::string healthText = "HEALTH: " + std::to_string(health);
        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
        RenderText(textShader, scoreText, 20.0f, 570.0f, 0.5f, glm::vec3(0.5, 0.8f, 0.2f));
        RenderText(textShader, healthText, 500.0f, 570.0f, 0.5f, glm::vec3(0.3, 0.7f, 0.9f));
        glDisable(GL_BLEND);

        // glfw: swap buffers and poll IO events (keys pressed/released, mouse moved etc.)
        glfwSwapBuffers(window);
        glfwPollEvents();
    }
    // glfw: terminate, clearing all previously allocated GLFW resources.
    glfwTerminate();
    return 0;
}

// process all input: query GLFW whether relevant keys are pressed/released this frame and react accordingly
// ---------------------------------------------------------------------------------------------------------
void processInput(GLFWwindow* window)
{
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        glfwSetWindowShouldClose(window, true);
    if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
        if (pitch < 25.0f)
        pitch += 70.0f * deltaTime;
    if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
        if (pitch > -25.0f)
        pitch += -70.0f * deltaTime;
    if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS) {
        if (yaw < 25.0f)
            yaw += 70.0f * deltaTime;
        if (roll < 15.0f)
            roll += 42.0f * deltaTime;
    }
    if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS) {
        if (yaw > -25.0f)
            yaw += -70.0f * deltaTime;
        if (roll > -15.0f)
            roll += -42.0f * deltaTime;
    }
    if (glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS && !spaceWasPressed) {
        spaceWasPressed = true;
        // Create a new projectile
        Projectile newProjectile;
        newProjectile.position = glm::vec3(shipPosition.x, shipPosition.y, 0.0f); // Launch from current position
        // Calculate the ship's velocity
        glm::vec3 shipVelocity = shipFront * speed;
        //glm::vec3 velo = glm::vec3(shipFront.x * 100, shipFront.y * 100, shipFront.z * 100);
        newProjectile.velocity = shipVelocity + shipFront * 100.0f;
        newProjectile.pitch = pitch;
        newProjectile.roll = roll;
        newProjectile.yaw = yaw;
        projectiles.push_back(newProjectile);
    }

    if (glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_RELEASE) {
        spaceWasPressed = false;
    }
    if (glfwGetKey(window, GLFW_KEY_W) == GLFW_RELEASE)
        if (pitch > 0.0f) {
            pitch += -70.0f * deltaTime;
            if (pitch < threshold)
                pitch = 0.0f;
        }
    if (glfwGetKey(window, GLFW_KEY_S) == GLFW_RELEASE)
        if (pitch < 0.0f) {
            pitch += 70.0f * deltaTime;
            if (pitch > -threshold)
                pitch = 0.0f;
        }
    if (glfwGetKey(window, GLFW_KEY_A) == GLFW_RELEASE) {
        if (yaw > 0.0f) {
            yaw += -70.0f * deltaTime;
            if (yaw < threshold)
                yaw = 0.0f;
        }
        if (roll > 0.0f) {
            roll += -42.0f * deltaTime;
            if (roll < threshold)
                roll = 0.0f;
        }
    }
    if (glfwGetKey(window, GLFW_KEY_D) == GLFW_RELEASE) {
        if (yaw < 0.0f) {
            yaw += 70.0f * deltaTime;
            if (yaw > -threshold)
                yaw = 0.0f;
        }
        if (roll < 0.0f) {
            roll += 42.0f * deltaTime;
            if (roll > -threshold)
                roll = 0.0f;
        }
    }
}

// glfw: whenever the window size changed (by OS or user resize) this callback function executes
// ---------------------------------------------------------------------------------------------
void framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
    // make sure the viewport matches the new window dimensions; note that width and 
    // height will be significantly larger than specified on retina displays.
    glViewport(0, 0, width, height);
}