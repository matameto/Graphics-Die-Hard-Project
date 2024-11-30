#include "TextureBuilder.h"
#include "Model_3DS.h"
#include "GLTexture.h"
#include "OBJModel.h"
#include <glut.h>
#include <cmath>
#include <iostream>
#include <vector>
#define SDL_MAIN_HANDLED
#include <SDL.h>
#include <SDL_mixer.h>

enum GameState {
    START_SCREEN,
    LEVEL_1,
    LEVEL_2,
    END_SCREEN_WIN, 
    END_SCREEN_LOSE
};

GameState currentGameState = START_SCREEN;

// Window dimensions
int WIDTH = 1280;
int HEIGHT = 720;

// Camera and player settings
float playerX = 0.0f, playerY = 1.8f, playerZ = 5.0f;  // Player position (Y is height)
float playerYaw = -90.0f;                              // Horizontal rotation (yaw angle)
float playerPitch = 0.0f;                              // Vertical rotation (pitch angle)
float playerRotation = 180.0f;                         // Actual player model rotation

// Movement speed and sensitivity
float playerSpeed = 0.3f;
float mouseSensitivity = 0.3f;
float keyRotationSpeed = 2.0f;                         // Degrees per frame for arrow key rotation

// Mouse tracking
float lastMouseX = WIDTH / 2.0f;
float lastMouseY = HEIGHT / 2.0f;
bool firstMouse = true;

// View settings
bool isThirdPerson = false;  // Toggle between first/third person view
float cameraDistance = 10.0f; // Distance behind player in third person
float cameraHeight = 4.0f;   // Height above player in third person


bool isMoving = false;
bool isMuzzleFlashActive = false;
float muzzleFlashTimer = 0.0f;
float recoilOffset = 0.0f;

bool isTransitioningToLevel2 = false; // Indicates if a transition is in progress
float transitionAlpha = 0.0f;        // Alpha value for the fade effect (0.0 = fully visible, 1.0 = fully black)
float transitionSpeed = 0.01f;       // Speed of the fade effect



float gameTime = 0.0f;       // Game time in seconds
float timeSpeed = 0.1f;      
float sunAngle = 0.0f;       // Angle of the sun (degrees)
float maxIntensity = 1.0f;   // Maximum light intensity
float minIntensity = 0.1f;   // Minimum light intensity
float lightIntensity = maxIntensity;  // Current light intensity
GLfloat lightPosition[] = { 0.0f, 10.0f, 0.0f, 1.0f }; // Initial light position
const float PI = 3.14159265358979323846;

int level2Score = 0;


// Models
Model_3DS model_house;
Model_3DS model_tree;
Model_3DS model_player;
Model_3DS model_rifle;  // New rifle model
Model_3DS model_barrel;
OBJModel model_target1;;
Model_3DS model_bullet;
Model_3DS model_light;
Model_3DS model_crate;
Model_3DS model_rubbish;

GLTexture sky_morning;  // Declare the sky texture globally
GLTexture sky_afternoon;
GLTexture sky_evening;
GLTexture sky_night;
GLTexture tex_ground;
GLTexture muzzle_flash;
GLTexture wall1;
GLTexture bullseye;
GLTexture tex_startScreen;
GLTexture ground_l2;
GLTexture ceiling;
GLTexture wall2;
GLTexture targetTexture;


// Sound variables : 
Mix_Chunk* shootSound = nullptr;
Mix_Chunk* hitSound = nullptr;
Mix_Music* idleMusic = nullptr;
Mix_Chunk* thudSound = nullptr;

struct CircularTarget {
    float x, y, z;      
    float radius;       
    bool isHit;         
    bool isFalling;     
    float rotationY;    
    float rotationZ;    
};
struct Target1 {
    float x, z;       
    float scale;      
    float rotationX;  
    bool isRotating;  
    bool isHit = false;   
    float rotationY = 0.0f; 
    float fallAngle = 0.0f; 
    bool isDisappearing = false; // Shrinking state
};

struct BarrelPosition {
    float x, z;     
    float scale;    
    float rotation; 
};

struct TreePosition {
    float x, z;
    float scale;
    float rotation;
};
struct Bullet {
    float x, y, z;  
    float directionX, directionY, directionZ; // where the bullet is going
    float  speed; // Velocity
};
struct Crate {
    float x, y, z;   
    float scale;     
    float rotation; 
};
struct Rubbish {
    float x, y, z;     
    float scale;       
    float rotation;    
};

struct BouncingTarget {
    float x, y, z;    
    float radius;     
    float bounceSpeed; 
    float bounceHeight; 
    float timeOffset; 
    bool isHit = false;   
    float scale = 1.0f;   
    bool isDisappearing = false; 
};

std::vector<BouncingTarget> bouncingTargets = {
    {60.0f, 0.0f, -60.0f, 1.2f, 0.2f, 0.6f, 1.5f},  
    {-50.0f, 0.0f, 50.0f, 1.0f, 0.2f, 0.5f, 0.7f},  
    {80.0f, 0.0f, 80.0f, 0.9f, 0.3f, 0.4f, 1.2f},   
    {-80.0f, 0.0f, -80.0f, 1.4f, 0.2f, 0.3f, 1.0f},
    {40.0f, 0.0f, -70.0f, 0.7f, 0.25f, 0.5f, 0.8f}, 
    {-70.0f, 0.0f, 40.0f, 1.1f, 0.2f, 0.4f, 0.6f}   
};

std::vector<Rubbish> rubbishObjects = {
    {15.0f, -0.4f, -25.0f, 0.5f * 3.0f, 10.0f}, 
    {-45.0f, -0.4f, -50.0f, 0.7f * 3.0f, 120.0f}, 
    {80.0f, -0.4f, -30.0f, 0.9f * 3.0f, 90.0f},   
    {95.0f, -0.4f, -95.0f, 0.6f * 3.0f, 45.0f},  
    {-95.0f, -0.4f, -95.0f, 0.8f * 3.0f, 30.0f},  
    {-95.0f, -0.4f, 95.0f, 0.4f * 3.0f, 150.0f},  
    {95.0f, -0.4f, 95.0f, 1.0f * 3.0f, 300.0f},  
    {50.0f, -0.4f, -100.0f, 0.6f * 3.0f, 60.0f},  
    {-50.0f, -0.4f, -100.0f, 0.7f * 3.0f, 75.0f}, 
    {50.0f, -0.4f, 100.0f, 0.5f * 3.0f, 20.0f},   
    {-50.0f, -0.4f, 100.0f, 0.9f * 3.0f, 90.0f},  
};

std::vector<Crate> crates = {
    {10.0f, 0.0f, -20.0f, 0.8f, 0.0f},
    {-15.0f, 0.0f, 25.0f, 1.0f, 45.0f},
    {30.0f, 0.0f, -40.0f, 0.7f, 90.0f},
    {50.0f, 0.0f, 15.0f, 1.2f, 180.0f},
    {-25.0f, 0.0f, -35.0f, 0.9f, 270.0f},
    {20.0f, 0.0f, 30.0f, 1.1f, 60.0f},
    {-30.0f, 0.0f, 10.0f, 0.6f, 120.0f}
};

std::vector<Target1> targets = {
    {10.0f, -25.0f, 0.2f, 0.0f, true, false},   
    {-15.0f, 30.0f, 0.2f, 0.0f, false, false}, 
    {30.0f, -45.0f, 0.2f, 0.0f, true, false},  
    {50.0f, 10.0f, 0.2f, 0.0f, true, false},   
    {-25.0f, -40.0f, 0.2f, 0.0f, false, false},
    {20.0f, 25.0f, 0.2f, 0.0f, true, false},   
    {-30.0f, 5.0f, 0.2f, 0.0f, false, false}   
};

std::vector<BarrelPosition> barrels = {
    {15.0f, -15.0f, 0.001f, 0.0f},
    {-25.0f, 10.0f, 0.001f, 45.0f},
    {40.0f, -50.0f, 0.001f, 90.0f},
    {70.0f, -90.0f, 0.001f, 180.0f},
    {-60.0f, 50.0f, 0.001f, 270.0f},
    {100.0f, 15.0f, 0.001f, 30.0f}
};

std::vector<CircularTarget> bullseyeTargets = {
    {15.0f, 7.0f, -30.0f, 2.5f, false, false, 0.0f, 0.0f},
    {35.0f, 10.0f, -50.0f, 1.8f, false, false, 0.0f, 0.0f},
    {-45.0f, 6.0f, 25.0f, 2.0f, false, false, 0.0f, 0.0f},
    {10.0f, 8.0f, -20.0f, 2.5f, false, false, 0.0f, 0.0f},
    {-30.0f, 9.0f, 30.0f, 1.8f, false, false, 0.0f, 0.0f},
    {-20.0f, 2.5f, 10.0f, 3.0f, false, false, 0.0f, 0.0f},
    {0.0f, 2.0f, -15.0f, 2.8f, false, false, 0.0f, 0.0f},
    {40.0f, 2.5f, 5.0f, 3.5f, false, false, 0.0f, 0.0f},
    {-35.0f, 3.0f, -25.0f, 3.0f, false, false, 0.0f, 0.0f},
    {50.0f, 3.5f, -10.0f, 3.2f, false, false, 0.0f, 0.0f},
    {-55.0f, 5.0f, -15.0f, 1.8f, false, false, 0.0f, 0.0f}, 
    {60.0f, 2.8f, -30.0f, 3.3f, false, false, 0.0f, 0.0f},  
    {-20.0f, 6.5f, 40.0f, 2.2f, false, false, 0.0f, 0.0f},  
    {15.0f, 3.0f, -50.0f, 3.1f, false, false, 0.0f, 0.0f},   
    {65.0f, 7.0f, 20.0f, 2.5f, false, false, 0.0f, 0.0f},   
};

std::vector<TreePosition> trees = {
    {20.0f, -20.0f, 0.5f, 0.0f},
    {-40.0f, -30.0f, 0.6f, 45.0f},
    {50.0f, -70.0f, 0.4f, 90.0f},
    {80.0f, -100.0f, 0.5f, 180.0f},
    {100.0f, -120.0f, 0.55f, 270.0f},
    {-120.0f, 80.0f, 0.45f, 120.0f},
    {90.0f, 100.0f, 0.5f, 200.0f},
    {-50.0f, -90.0f, 0.6f, 150.0f},
    {120.0f, 60.0f, 0.4f, 80.0f},
    {-80.0f, 90.0f, 0.5f, 220.0f},
    {110.0f, -130.0f, 0.55f, 30.0f},
    {-130.0f, -70.0f, 0.45f, 160.0f},
    {140.0f, 110.0f, 0.5f, 290.0f},
    {-110.0f, 70.0f, 0.6f, 100.0f},
    {130.0f, 140.0f, 0.4f, 240.0f},
    {-70.0f, 130.0f, 0.5f, 70.0f}
};

float lastUpdateTime = 0.0f;  // Tracks the last time the bouncing targets were updated

std::vector<Bullet> bullets;

std::vector<GLfloat> lampPositionsX;  
std::vector<GLfloat> lampPositionsZ;  
std::vector<GLfloat> lampLightIntensity; 
std::vector<float> flickerSpeeds;     


bool initAudio() {
    if (SDL_Init(SDL_INIT_AUDIO) < 0) {
        std::cerr << "SDL could not initialize! SDL_Error: " << SDL_GetError() << "\n";
        return false;
    }

    if (Mix_OpenAudio(44100, MIX_DEFAULT_FORMAT, 2, 2048) < 0) {
        std::cerr << "SDL_mixer could not initialize! SDL_mixer Error: " << Mix_GetError() << "\n";
        return false;
    }
    return true;
}

void PlayIdleMusic() {
    if (idleMusic != nullptr) {
        if (Mix_PlayingMusic() == 0) {
            Mix_PlayMusic(idleMusic, -1); 
            std::cout << "Playing idle music.\n";
        }
    }
    else {
        std::cerr << "Idle music not loaded!\n";
    }
}

void StopIdleMusic() {
    if (Mix_PlayingMusic() != 0) { 
        Mix_HaltMusic(); 
        std::cout << "Idle music stopped.\n";
    }
}

void UpdateBouncingTargets() {
    for (auto& target : bouncingTargets) {
        if (target.isDisappearing) {
            if (target.scale > 0.0f) {
                target.scale -= 0.05f; // Shrink over time
            }
            else {
                target.scale = 0.0f; // Fully disappear
            }
        }
        else if (target.isHit) {
            float popSpeed = 5.0f; // Speed of popping animation
            target.scale = 1.0f + 0.2f * sin(gameTime * popSpeed);
        }
        else {
            // Regular bounce logic
            target.y = target.bounceHeight * fabs(sin((gameTime + target.timeOffset) * target.bounceSpeed));
        }
    }
}

void RenderBouncingTargets() {
    glEnable(GL_TEXTURE_2D);
    targetTexture.Use();

    for (const auto& target : bouncingTargets) {
        if (target.scale <= 0.0f) continue;

        glPushMatrix();
        glTranslatef(target.x, target.y + 1.5f, target.z);
        glScalef(target.scale, target.scale, target.scale); 
        GLUquadric* quad = gluNewQuadric();
        gluQuadricTexture(quad, GL_TRUE);
        gluSphere(quad, target.radius, 32, 32);
        gluDeleteQuadric(quad);

        glPopMatrix();
    }

    glDisable(GL_TEXTURE_2D);
}

void UpdateAmbientLight(bool isFlashing) {
    if (isFlashing) {
        GLfloat ambient[] = { 0.4f, 0.4f, 0.2f, 1.0f }; // Bright ambient light
        glLightModelfv(GL_LIGHT_MODEL_AMBIENT, ambient);
    }
    else {
        GLfloat ambient[] = { 0.2f, 0.2f, 0.2f, 1.0f }; // Default ambient light
        glLightModelfv(GL_LIGHT_MODEL_AMBIENT, ambient);
    }
}

void CalculateWeaponTipPosition(float& x, float& y, float& z) {
    float pitchRadians = playerPitch * (PI / 180.0f);
    float yawRadians = playerYaw * (PI / 180.0f);

    // More precise weapon tip offset in local weapon coordinates
    float weaponLength = 2.1f;  // Length of the rifle from base to tip
    float weaponOffsetX = 0.3f; // Slight X offset 
    float weaponOffsetY = -0.2f; // Slight Y offset
    float weaponOffsetZ = weaponLength;

    x = playerX +
        cos(yawRadians) * weaponOffsetZ -
        sin(yawRadians) * weaponOffsetX;

    y = playerY +
        weaponOffsetY +
        sin(pitchRadians) * weaponOffsetZ;

    z = playerZ +
        sin(yawRadians) * weaponOffsetZ +
        cos(yawRadians) * weaponOffsetX;
}

void CalculateMuzzleFlashPosition(float& x, float& y, float& z) {
    float pitchRadians = playerPitch * (PI / 180.0f);
    float yawRadians = playerYaw * (PI / 180.0f);

    float weaponLength = 2.1f; 
    float weaponOffsetX = 0.3f;
    float weaponOffsetY = -0.2f;
    float weaponOffsetZ = weaponLength;

    x = playerX +
        cos(yawRadians) * weaponOffsetZ -
        sin(yawRadians) * weaponOffsetX;

    y = playerY +
        weaponOffsetY +
        sin(pitchRadians) * weaponOffsetZ;

    z = playerZ +
        sin(yawRadians) * weaponOffsetZ +
        cos(yawRadians) * weaponOffsetX;
}

void Shoot() {
    Bullet bullet;
    float pitchRadians = playerPitch * (PI / 180.0f);
    float yawRadians = playerYaw * (PI / 180.0f);

    // Get precise weapon tip position
    CalculateWeaponTipPosition(bullet.x, bullet.y, bullet.z);

    // Calculate bullet direction based on camera pitch and yaw
    bullet.directionX = cos(pitchRadians) * cos(yawRadians);
    bullet.directionY = sin(pitchRadians);
    bullet.directionZ = cos(pitchRadians) * sin(yawRadians);

    bullet.speed = 1.5f; 

    bullets.push_back(bullet);
    if (shootSound != nullptr) {
        Mix_PlayChannel(-1, shootSound, 0);
    }
    isMuzzleFlashActive = true;
    muzzleFlashTimer = 0.05f;
    UpdateAmbientLight(true);

  
}
bool CheckBullseyeCollision(const Bullet& bullet, const CircularTarget& target) {
    float dx = bullet.x - target.x;
    float dz = bullet.z - target.z;
    float distanceSquared = dx * dx + dz * dz;

    return distanceSquared <= target.radius * target.radius;
}
void UpdateCamera() {
    float lookX = cos(playerYaw * (PI / 180.0f));
    float lookZ = sin(playerYaw * (PI / 180.0f));

    gluLookAt(
        playerX, playerY, playerZ,          // Camera position
        playerX + lookX, playerY, playerZ + lookZ, // Look-at point
        0.0f, 1.0f, 0.0f                   // Up vector
    );

    if (recoilOffset > 0.0f) recoilOffset -= 0.01f;

}
void myMouseMotion(int x, int y) {
    if (firstMouse) {
        lastMouseX = x;
        lastMouseY = y;
        firstMouse = false;
        return;
    }

    float xOffset = (x - lastMouseX) * mouseSensitivity;
    float yOffset = (lastMouseY - y) * mouseSensitivity;
    lastMouseX = x;
    lastMouseY = y;

    playerYaw += xOffset;
    playerPitch += yOffset;

    // Constrain pitch
    if (playerPitch > 89.0f) playerPitch = 89.0f;
    if (playerPitch < -89.0f) playerPitch = -89.0f;

    // Normalize yaw
    if (playerYaw > 360.0f) playerYaw -= 360.0f;
    if (playerYaw < 0.0f) playerYaw += 360.0f;

    // Update player rotation to match camera direction
    playerRotation = playerYaw + 180.0f;

    glutPostRedisplay();
}

void myMouse(int button, int state, int x, int y) {
    if (button == GLUT_LEFT_BUTTON && state == GLUT_DOWN) {
        Shoot();
    }
}

void RenderFirstPersonWeapon() {
    if (isThirdPerson) return;  // Only render in first person

    // Save current matrices
    glMatrixMode(GL_PROJECTION);
    glPushMatrix();
    glLoadIdentity();
    gluPerspective(45.0, (GLdouble)WIDTH / (GLdouble)HEIGHT, 0.1, 200.0);

    glMatrixMode(GL_MODELVIEW);
    glPushMatrix();
    glLoadIdentity();


    // Position the weapon
    glTranslatef(0.3f, -0.4f, -0.9f);  

    // Rotate and scale the weapon model
    glScalef(0.0005f, 0.0005f, 0.0005f);

    glRotatef(180.0f, 0.0f, 1.0f, 0.0f);  // Align weapon model
    glRotatef(270.0f, 0.0f, 1.0f, 0.0f);   // Point weapon forward
    // Draw the weapon
    model_rifle.Draw();

    // Restore matrices
    glMatrixMode(GL_PROJECTION);
    glPopMatrix();
    glMatrixMode(GL_MODELVIEW);
    glPopMatrix();
}
void RenderMuzzleFlashLight() {
    if (!isMuzzleFlashActive) return;

    GLfloat lightPos[] = { playerX + 0.3f, playerY - 0.1f, playerZ + 0.5f, 1.0f };
    GLfloat diffuse[] = { 2.0f, 2.0f, 1.8f, 1.0f }; // Bright yellow light
    GLfloat specular[] = { 1.0f, 1.0f, 0.8f, 1.0f }; // Slightly white specular

    glEnable(GL_LIGHT1); // Enable a secondary light
    glLightfv(GL_LIGHT1, GL_POSITION, lightPos);
    glLightfv(GL_LIGHT1, GL_DIFFUSE, diffuse);
    glLightfv(GL_LIGHT1, GL_SPECULAR, specular);

    // Ensure the light is disabled after rendering
    glDisable(GL_LIGHT1);
}

void RenderMuzzleFlash() {
    if (!isMuzzleFlashActive) return;

    float flashX, flashY, flashZ;
    CalculateMuzzleFlashPosition(flashX, flashY, flashZ);

    glPushMatrix();
    glTranslatef(flashX, flashY, flashZ);

    // Align with player's orientation
    glRotatef(playerYaw, 0.0f, 1.0f, 0.0f);
    glRotatef(-playerPitch, 1.0f, 0.0f, 0.0f);

    glScalef(1.4f, 1.4f, 1.4f);

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE);

    muzzle_flash.Use();
    glColor4f(2.0f, 2.0f, 2.0f, 1.0f);

    glBegin(GL_QUADS);
    glTexCoord2f(0.0f, 0.0f); glVertex3f(-0.1f, -0.1f, 0.0f);
    glTexCoord2f(1.0f, 0.0f); glVertex3f(0.1f, -0.1f, 0.0f);
    glTexCoord2f(1.0f, 1.0f); glVertex3f(0.1f, 0.1f, 0.0f);
    glTexCoord2f(0.0f, 1.0f); glVertex3f(-0.1f, 0.1f, 0.0f);
    glEnd();

    glDisable(GL_BLEND);
    glPopMatrix();
}

void RenderCeiling() {
    float ceilingHeight = 10.0f;

    glEnable(GL_TEXTURE_2D);
    glBindTexture(GL_TEXTURE_2D, ceiling.texture[0]);

    GLfloat mat_ambient[] = { 0.5f, 0.5f, 0.5f, 1.0f };
    GLfloat mat_diffuse[] = { 0.8f, 0.8f, 0.8f, 1.0f };
    glMaterialfv(GL_FRONT, GL_AMBIENT, mat_ambient);
    glMaterialfv(GL_FRONT, GL_DIFFUSE, mat_diffuse);

    glBegin(GL_QUADS);
    glNormal3f(0, -1, 0); // Correct normal for lighting
    glTexCoord2f(0, 0); glVertex3f(-100, ceilingHeight, -100);
    glTexCoord2f(10, 0); glVertex3f(100, ceilingHeight, -100);
    glTexCoord2f(10, 10); glVertex3f(100, ceilingHeight, 100);
    glTexCoord2f(0, 10); glVertex3f(-100, ceilingHeight, 100);
    glEnd();

    glDisable(GL_TEXTURE_2D);
}

void RenderWalls() {
    float groundMin = -100.0f;
    float groundMax = 100.0f;
    float wallHeight = 10.0f;  
    float wallThickness = 1.0f; 

    // glEnable(GL_TEXTURE_2D);
    //glBindTexture(GL_TEXTURE_2D, tex_wall.texture[0]);

     // North Wall (along Z-axis)
    glPushMatrix();
    wall1.Use();

    glTranslatef(0, wallHeight / 2, groundMax);
    glScalef(groundMax * 2, wallHeight, wallThickness);
    glColor3f(1, 1, 1); // White color to show full texture

    glBegin(GL_QUADS);
    // Front face
    glTexCoord2f(0, 0); glVertex3f(-0.5, -0.5, 0.5);
    glTexCoord2f(1, 0); glVertex3f(0.5, -0.5, 0.5);
    glTexCoord2f(1, 1); glVertex3f(0.5, 0.5, 0.5);
    glTexCoord2f(0, 1); glVertex3f(-0.5, 0.5, 0.5);

    // Back face
    glTexCoord2f(0, 0); glVertex3f(-0.5, -0.5, -0.5);
    glTexCoord2f(1, 0); glVertex3f(0.5, -0.5, -0.5);
    glTexCoord2f(1, 1); glVertex3f(0.5, 0.5, -0.5);
    glTexCoord2f(0, 1); glVertex3f(-0.5, 0.5, -0.5);
    glEnd();

    glPopMatrix();

    // South Wall (along Z-axis)
    glPushMatrix();
    glTranslatef(0, wallHeight / 2, groundMin);
    glScalef(groundMax * 2, wallHeight, wallThickness);

    glBegin(GL_QUADS);
    // Front face
    glTexCoord2f(0, 0); glVertex3f(-0.5, -0.5, 0.5);
    glTexCoord2f(1, 0); glVertex3f(0.5, -0.5, 0.5);
    glTexCoord2f(1, 1); glVertex3f(0.5, 0.5, 0.5);
    glTexCoord2f(0, 1); glVertex3f(-0.5, 0.5, 0.5);

    // Back face
    glTexCoord2f(0, 0); glVertex3f(-0.5, -0.5, -0.5);
    glTexCoord2f(1, 0); glVertex3f(0.5, -0.5, -0.5);
    glTexCoord2f(1, 1); glVertex3f(0.5, 0.5, -0.5);
    glTexCoord2f(0, 1); glVertex3f(-0.5, 0.5, -0.5);
    glEnd();

    glPopMatrix();

    // East Wall (along X-axis)
    glPushMatrix();
    glTranslatef(groundMax, wallHeight / 2, 0);
    glRotatef(90, 0, 1, 0);
    glScalef(groundMax * 2, wallHeight, wallThickness);

    glBegin(GL_QUADS);
    // Front face
    glTexCoord2f(0, 0); glVertex3f(-0.5, -0.5, 0.5);
    glTexCoord2f(1, 0); glVertex3f(0.5, -0.5, 0.5);
    glTexCoord2f(1, 1); glVertex3f(0.5, 0.5, 0.5);
    glTexCoord2f(0, 1); glVertex3f(-0.5, 0.5, 0.5);

    // Back face
    glTexCoord2f(0, 0); glVertex3f(-0.5, -0.5, -0.5);
    glTexCoord2f(1, 0); glVertex3f(0.5, -0.5, -0.5);
    glTexCoord2f(1, 1); glVertex3f(0.5, 0.5, -0.5);
    glTexCoord2f(0, 1); glVertex3f(-0.5, 0.5, -0.5);
    glEnd();

    glPopMatrix();

    // West Wall (along X-axis)
    glPushMatrix();
    glTranslatef(groundMin, wallHeight / 2, 0);
    glRotatef(90, 0, 1, 0);
    glScalef(groundMax * 2, wallHeight, wallThickness);

    glBegin(GL_QUADS);
    // Front face
    glTexCoord2f(0, 0); glVertex3f(-0.5, -0.5, 0.5);
    glTexCoord2f(1, 0); glVertex3f(0.5, -0.5, 0.5);
    glTexCoord2f(1, 1); glVertex3f(0.5, 0.5, 0.5);
    glTexCoord2f(0, 1); glVertex3f(-0.5, 0.5, 0.5);

    // Back face
    glTexCoord2f(0, 0); glVertex3f(-0.5, -0.5, -0.5);
    glTexCoord2f(1, 0); glVertex3f(0.5, -0.5, -0.5);
    glTexCoord2f(1, 1); glVertex3f(0.5, 0.5, -0.5);
    glTexCoord2f(0, 1); glVertex3f(-0.5, 0.5, -0.5);
    glEnd();

    glPopMatrix();

    glDisable(GL_TEXTURE_2D);
}

void RenderWallsL2() {
    float groundMin = -100.0f;
    float groundMax = 100.0f;
    float wallHeight = 10.0f;   // Height of the walls
    float wallThickness = 1.0f; // Thickness of the walls
    float roomGap = 15.0f;      // Width of the gap between rooms (for doors)

    wall2.Use();

    // North Wall
    glPushMatrix();
    glTranslatef(0, wallHeight / 2, groundMax);
    glScalef(groundMax * 2, wallHeight, wallThickness);
    glColor3f(1, 1, 1);
    glBegin(GL_QUADS);
    glTexCoord2f(0, 0); glVertex3f(-0.5, -0.5, 0.5);
    glTexCoord2f(1, 0); glVertex3f(0.5, -0.5, 0.5);
    glTexCoord2f(1, 1); glVertex3f(0.5, 0.5, 0.5);
    glTexCoord2f(0, 1); glVertex3f(-0.5, 0.5, 0.5);
    glEnd();
    glPopMatrix();

    // South Wall
    glPushMatrix();
    glTranslatef(0, wallHeight / 2, groundMin);
    glScalef(groundMax * 2, wallHeight, wallThickness);
    glBegin(GL_QUADS);
    glTexCoord2f(0, 0); glVertex3f(-0.5, -0.5, 0.5);
    glTexCoord2f(1, 0); glVertex3f(0.5, -0.5, 0.5);
    glTexCoord2f(1, 1); glVertex3f(0.5, 0.5, 0.5);
    glTexCoord2f(0, 1); glVertex3f(-0.5, 0.5, 0.5);
    glEnd();
    glPopMatrix();

    // East Wall
    glPushMatrix();
    glTranslatef(groundMax, wallHeight / 2, 0);
    glRotatef(90, 0, 1, 0);
    glScalef(groundMax * 2, wallHeight, wallThickness);
    glBegin(GL_QUADS);
    glTexCoord2f(0, 0); glVertex3f(-0.5, -0.5, 0.5);
    glTexCoord2f(1, 0); glVertex3f(0.5, -0.5, 0.5);
    glTexCoord2f(1, 1); glVertex3f(0.5, 0.5, 0.5);
    glTexCoord2f(0, 1); glVertex3f(-0.5, 0.5, 0.5);
    glEnd();
    glPopMatrix();

    // West Wall
    glPushMatrix();
    glTranslatef(groundMin, wallHeight / 2, 0);
    glRotatef(90, 0, 1, 0);
    glScalef(groundMax * 2, wallHeight, wallThickness);
    glBegin(GL_QUADS);
    glTexCoord2f(0, 0); glVertex3f(-0.5, -0.5, 0.5);
    glTexCoord2f(1, 0); glVertex3f(0.5, -0.5, 0.5);
    glTexCoord2f(1, 1); glVertex3f(0.5, 0.5, 0.5);
    glTexCoord2f(0, 1); glVertex3f(-0.5, 0.5, 0.5);
    glEnd();
    glPopMatrix();

   
    glPopMatrix();

    glDisable(GL_TEXTURE_2D);
}

void RenderGround() {
    glDisable(GL_LIGHTING);
    glColor3f(0.6, 0.8, 0.6);

    glEnable(GL_TEXTURE_2D);
    glBindTexture(GL_TEXTURE_2D, tex_ground.texture[0]);

    glBegin(GL_QUADS);
    glNormal3f(0, 1, 0);
    glTexCoord2f(0, 0); glVertex3f(-100, 0, -100);
    glTexCoord2f(10, 0); glVertex3f(100, 0, -100);
    glTexCoord2f(10, 10); glVertex3f(100, 0, 100);
    glTexCoord2f(0, 10); glVertex3f(-100, 0, 100);
    glEnd();

    glEnable(GL_LIGHTING);
}

void RenderGroundL2() {
    glDisable(GL_LIGHTING);
    //glColor3f(0.6, 0.8, 0.6);

    glEnable(GL_TEXTURE_2D);
    glBindTexture(GL_TEXTURE_2D, ground_l2.texture[0]);

    glBegin(GL_QUADS);
    glNormal3f(0, 1, 0);
    glTexCoord2f(0, 0); glVertex3f(-100, 0, -100);
    glTexCoord2f(10, 0); glVertex3f(100, 0, -100);
    glTexCoord2f(10, 10); glVertex3f(100, 0, 100);
    glTexCoord2f(0, 10); glVertex3f(-100, 0, 100);
    glEnd();

    glEnable(GL_LIGHTING);
}

void RenderPlayer() {
    if (!isThirdPerson) return;

    glPushMatrix();
    // Translate to the player's position
    glTranslatef(playerX, playerY - 1.8f, playerZ);

    // Rotate the player model to match camera direction
    glRotatef(playerRotation, 0.0f, 1.0f, 0.0f);

    // Scale the player model
    glScalef(0.01f, 0.01f, 0.01f);

    model_player.Draw();
    glPopMatrix();
}

void RenderTrees() {
    for (const auto& tree : trees) {
        glPushMatrix();
        glTranslatef(tree.x, 0, tree.z);
        glRotatef(tree.rotation, 0, 1, 0);
        glScalef(tree.scale, tree.scale, tree.scale);
        model_tree.Draw();
        glPopMatrix();
    }
}

void RenderBarrels() {
    for (const auto& barrel : barrels) {
        glPushMatrix();

        // Position each barrel
        glTranslatef(barrel.x, 0.0f, barrel.z);

        // Rotate the barrel
        glRotatef(barrel.rotation, 0.0f, 1.0f, 0.0f);

        // Scale the barrel
        glScalef(barrel.scale, barrel.scale, barrel.scale);

        // Draw the barrel model
        model_barrel.Draw();

        glPopMatrix();
    }
}

void RenderTargets() {
    for (const auto& target : targets) {
        if (target.scale <= 0.0f) continue; // Skip fully disappeared targets

        glPushMatrix();
        glTranslatef(target.x, 0.8f, target.z);
        glRotatef(target.fallAngle, 0.0f, 0.0f, 1.0f); // Apply falling rotation
        glScalef(target.scale, target.scale, target.scale * 0.1); // Shrink

        model_target1.Draw();
        glPopMatrix();
    }
}

void RenderBullets() {
    for (const auto& bullet : bullets) {
        glPushMatrix();

        // Translate to the bullet's current position
        glTranslatef(bullet.x, bullet.y, bullet.z);

        // Calculate yaw (rotation around Y-axis)
        float yawRadians = atan2(bullet.directionZ, bullet.directionX);
        float yawDegrees = yawRadians * (180.0f / PI); // Convert to degrees

        // Calculate pitch (rotation around X-axis)
        float horizontalLength = sqrt(bullet.directionX * bullet.directionX + bullet.directionZ * bullet.directionZ);
        float pitchRadians = atan2(bullet.directionY, horizontalLength);
        float pitchDegrees = pitchRadians * (180.0f / PI); // Convert to degrees

        // Apply rotations in the correct order
        glRotatef(-yawDegrees, 0.0f, 1.0f, 0.0f); // Align to the yaw direction
        glRotatef(pitchDegrees, 0.0f, 0.0f, 1.0f); // Align to the pitch direction

        // Scale the bullet
        glScalef(0.005f, 0.005f, 0.005f); // Debug scaling for visualization

        // Draw the bullet model
        model_bullet.Draw();

        glPopMatrix();
    }
}

void GetSkyBlendFactors(float gameTime, float& factor1, float& factor2, GLTexture& sky1, GLTexture& sky2) {
    // Normalize gameTime to 0–360
    float timeNormalized = fmod(gameTime, 360.0f);

    if (timeNormalized < 90.0f) { // Morning -> Afternoon
        factor1 = 1.0f - timeNormalized / 90.0f;
        factor2 = timeNormalized / 90.0f;
        sky1 = sky_morning;
        sky2 = sky_afternoon;
    }
    else if (timeNormalized < 180.0f) { // Afternoon -> Evening
        factor1 = 1.0f - (timeNormalized - 90.0f) / 90.0f;
        factor2 = (timeNormalized - 90.0f) / 90.0f;
        sky1 = sky_afternoon;
        sky2 = sky_evening;
    }
    else if (timeNormalized < 270.0f) { // Evening -> Night
        factor1 = 1.0f - (timeNormalized - 180.0f) / 90.0f;
        factor2 = (timeNormalized - 180.0f) / 90.0f;
        sky1 = sky_evening;
        sky2 = sky_night;
    }
    else { // Night -> Morning
        factor1 = 1.0f - (timeNormalized - 270.0f) / 90.0f;
        factor2 = (timeNormalized - 270.0f) / 90.0f;
        sky1 = sky_night;
        sky2 = sky_morning;
    }
}

void RenderSkySphere(GLuint texture) {
    glEnable(GL_TEXTURE_2D);
    glBindTexture(GL_TEXTURE_2D, texture);

    const int slices = 40; // Number of slices for the sphere
    const int stacks = 40; // Number of stacks for the sphere
    const float radius = 100.0f;

    for (int i = 0; i < stacks; ++i) {
        float theta1 = i * PI / stacks;
        float theta2 = (i + 1) * PI / stacks;

        glBegin(GL_QUAD_STRIP);
        for (int j = 0; j <= slices; ++j) {
            float phi = j * 2 * PI / slices;

            float x1 = sin(theta1) * cos(phi);
            float y1 = cos(theta1);
            float z1 = sin(theta1) * sin(phi);

            float x2 = sin(theta2) * cos(phi);
            float y2 = cos(theta2);
            float z2 = sin(theta2) * sin(phi);

            float u = (float)j / slices;
            float v1 = (float)i / stacks;
            float v2 = (float)(i + 1) / stacks;

            glTexCoord2f(u, v1); glVertex3f(x1 * radius, y1 * radius, z1 * radius);
            glTexCoord2f(u, v2); glVertex3f(x2 * radius, y2 * radius, z2 * radius);
        }
        glEnd();
    }
}

void RenderSky() {
    float factor1, factor2;
    GLTexture sky1, sky2;

    // Get blending factors and textures based on time
    GetSkyBlendFactors(gameTime, factor1, factor2, sky1, sky2);

    // Disable lighting and depth testing for the sky
    glDisable(GL_LIGHTING);
    glDisable(GL_DEPTH_TEST);
    glEnable(GL_TEXTURE_2D);

    // Render the first sky texture with its blending factor
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glColor4f(1.0f, 1.0f, 1.0f, factor1); // Set blend factor1
    RenderSkySphere(sky1.texture[0]);

    // Render the second sky texture blended with the first
    glColor4f(1.0f, 1.0f, 1.0f, factor2); // Set blend factor2
    RenderSkySphere(sky2.texture[0]);
    glDisable(GL_BLEND);

    // Restore OpenGL state
    glEnable(GL_LIGHTING);
    glEnable(GL_DEPTH_TEST);
}

void UpdateLighting(float currentGameTime) {
    // Update sun angle based on the provided game time
    float sunAngle = currentGameTime; // Sun angle progresses with time

    // Compute normalized time (0 to 1) based on the sun's angle
    float normalizedTime = (sin(sunAngle * PI / 180.0f) + 1.0f) / 1.5f;

    // Update light intensity based on normalized time
    lightIntensity = minIntensity + (maxIntensity - minIntensity) * normalizedTime;

    // Set light properties
    GLfloat diffuse[] = { lightIntensity, lightIntensity, lightIntensity, 1.0f };
    GLfloat ambient[] = { lightIntensity * 0.3f, lightIntensity * 0.3f, lightIntensity * 0.3f, 1.0f };

    glLightfv(GL_LIGHT0, GL_DIFFUSE, diffuse);
    glLightfv(GL_LIGHT0, GL_AMBIENT, ambient);
}

void RenderBullseyeTargets() {
    bullseye.Use(); // Bind the bullseye texture

    for (const auto& target : bullseyeTargets) {
        if (target.rotationZ == 90.0f) continue; // Skip fully fallen targets

        glPushMatrix();

        // Position the target
        glTranslatef(target.x, target.y, target.z);

        // Apply Y-axis rotation for spinning
        glRotatef(target.rotationY, 0.0f, 1.0f, 0.0f);

        // Apply Z-axis rotation for falling
        glRotatef(-target.rotationZ, 0.0f, 0.0f, 1.0f);

        // Rotate the disk to face upward
        glRotatef(90.0f, 0.0f, 1.0f, 0.0f);

        // Adjust brightness dynamically based on lighting intensity
        float adjustedBrightness = lightIntensity; // Use global lightIntensity
        glColor3f(adjustedBrightness, adjustedBrightness, adjustedBrightness);

        // Enable lighting and normals for proper interaction
        glEnable(GL_LIGHTING);
        glEnable(GL_NORMALIZE);

        // Set up a quadric for rendering
        GLUquadric* quad = gluNewQuadric();

        // Top face of the bullseye
        gluQuadricTexture(quad, GL_TRUE); // Enable texturing
        glNormal3f(0.0f, 0.0f, 1.0f); // Normal pointing up
        gluDisk(quad, 0.0, target.radius, 32, 1);

        // Extruded sides
        gluQuadricTexture(quad, GL_FALSE); // Disable texture for the sides
        glPushMatrix();
        glTranslatef(0.0f, 0.0f, -0.1f); // Move to start drawing the cylinder
        gluCylinder(quad, target.radius, target.radius, 0.1f, 32, 1); // Draw the sides
        glPopMatrix();

        // Bottom face of the bullseye
        glPushMatrix();
        glTranslatef(0.0f, 0.0f, -0.1f); // Move to the bottom face
        gluQuadricTexture(quad, GL_TRUE); // Re-enable texture
        glNormal3f(0.0f, 0.0f, -1.0f); // Normal pointing down
        gluDisk(quad, 0.0, target.radius, 32, 1);
        glPopMatrix();

        // Clean up the quadric
        gluDeleteQuadric(quad);

        glPopMatrix();
    }

    glDisable(GL_TEXTURE_2D); // Ensure textures are disabled after use
}


void RenderStartScreen() {
    // Disable lighting and enable 2D textures
    glDisable(GL_LIGHTING);
    glEnable(GL_TEXTURE_2D);

    // Switch to 2D orthographic projection
    glMatrixMode(GL_PROJECTION);
    glPushMatrix();
    glLoadIdentity();
    gluOrtho2D(0.0, WIDTH, 0.0, HEIGHT); // Match the screen dimensions

    glMatrixMode(GL_MODELVIEW);
    glPushMatrix();
    glLoadIdentity();

    // Render the textured quad to fill the screen
    tex_startScreen.Use(); // Bind the texture
    glBegin(GL_QUADS);
    glTexCoord2f(0.0f, 0.0f); glVertex2f(0.0f, 0.0f);
    glTexCoord2f(1.0f, 0.0f); glVertex2f(WIDTH, 0.0f);
    glTexCoord2f(1.0f, 1.0f); glVertex2f(WIDTH, HEIGHT);
    glTexCoord2f(0.0f, 1.0f); glVertex2f(0.0f, HEIGHT);
    glEnd();

    glDisable(GL_TEXTURE_2D);

    glDisable(GL_DEPTH_TEST);
    // Render the text
    glColor3f(1.0f, 1.0f, 1.0f); // White color for text

    // Position the title text
    const char* titleText = "Die Hard!";
    glRasterPos2f((WIDTH - glutBitmapLength(GLUT_BITMAP_TIMES_ROMAN_24, (const unsigned char*)titleText)) / 2, HEIGHT / 2 + 50);
    for (const char* c = titleText; *c != '\0'; c++) {
        glutBitmapCharacter(GLUT_BITMAP_HELVETICA_18, *c);
    }

    // Position the instructions text
    const char* instructions1 = "Press ENTER to Start";
    glRasterPos2f((WIDTH - glutBitmapLength(GLUT_BITMAP_HELVETICA_18, (const unsigned char*)instructions1)) / 2, HEIGHT / 2 - 50);
    for (const char* c = instructions1; *c != '\0'; c++) {
        glutBitmapCharacter(GLUT_BITMAP_HELVETICA_18, *c);
    }

    const char* instructions2 = "Press ESC to Exit";
    glRasterPos2f((WIDTH - glutBitmapLength(GLUT_BITMAP_HELVETICA_18, (const unsigned char*)instructions2)) / 2, HEIGHT / 2 - 100);
    for (const char* c = instructions2; *c != '\0'; c++) {
        glutBitmapCharacter(GLUT_BITMAP_HELVETICA_18, *c);
    }

    glEnable(GL_DEPTH_TEST);
    // Restore the original projection and modelview matrices
    glMatrixMode(GL_PROJECTION);
    glPopMatrix();

    glMatrixMode(GL_MODELVIEW);
    glPopMatrix();
}


void RenderHUD(int currentScore, int totalTargets, int remainingTime) {
    glDisable(GL_LIGHTING);
    glDisable(GL_DEPTH_TEST);

    // Switch to orthographic projection
    glMatrixMode(GL_PROJECTION);
    glPushMatrix();
    glLoadIdentity();
    gluOrtho2D(0.0, WIDTH, 0.0, HEIGHT);

    glMatrixMode(GL_MODELVIEW);
    glPushMatrix();
    glLoadIdentity();

    // Set text color
    glColor3f(1.0f, 1.0f, 1.0f);

    // Render Level
    std::string levelText = (currentGameState == LEVEL_1) ? "Level 1" : "Level 2";
    glRasterPos2f(WIDTH / 2 - 50, HEIGHT - 30);
    for (char c : levelText) {
        glutBitmapCharacter(GLUT_BITMAP_HELVETICA_18, c);
    }

    // Render Score
    std::string scoreText = "Score: " + std::to_string(currentScore) + " / " + std::to_string(totalTargets);
    glRasterPos2f(20, HEIGHT - 60);
    for (char c : scoreText) {
        glutBitmapCharacter(GLUT_BITMAP_HELVETICA_18, c);
    }

    // Render Timer
    std::string timerText = "Time: " + std::to_string(remainingTime) + "ms";
    glRasterPos2f(WIDTH - 150, HEIGHT - 30);
    for (char c : timerText) {
        glutBitmapCharacter(GLUT_BITMAP_HELVETICA_18, c);
    }

    // Restore original matrices
    glMatrixMode(GL_PROJECTION);
    glPopMatrix();

    glMatrixMode(GL_MODELVIEW);
    glPopMatrix();

    glEnable(GL_DEPTH_TEST);
}

void RenderGameOverScreen() {
    // Disable lighting and enable 2D textures
    glDisable(GL_LIGHTING);
    glEnable(GL_TEXTURE_2D);

    // Switch to 2D orthographic projection
    glMatrixMode(GL_PROJECTION);
    glPushMatrix();
    glLoadIdentity();
    gluOrtho2D(0.0, WIDTH, 0.0, HEIGHT); // Match the screen dimensions

    glMatrixMode(GL_MODELVIEW);
    glPushMatrix();
    glLoadIdentity();

    // Render the textured background
    tex_startScreen.Use(); // Use the start screen texture
    glBegin(GL_QUADS);
    glTexCoord2f(0.0f, 0.0f); glVertex2f(0.0f, 0.0f);
    glTexCoord2f(1.0f, 0.0f); glVertex2f(WIDTH, 0.0f);
    glTexCoord2f(1.0f, 1.0f); glVertex2f(WIDTH, HEIGHT);
    glTexCoord2f(0.0f, 1.0f); glVertex2f(0.0f, HEIGHT);
    glEnd();

    // Render the text
    glDisable(GL_TEXTURE_2D); // Disable texture for text
    glDisable(GL_DEPTH_TEST);
    glColor3f(1.0f, 0.0f, 0.0f); // Red color for "GAME OVER!"

    const char* gameOverText = "GAME OVER!";
    glRasterPos2f((WIDTH - glutBitmapLength(GLUT_BITMAP_TIMES_ROMAN_24, (const unsigned char*)gameOverText)) / 2, HEIGHT / 2 + 50);
    for (const char* c = gameOverText; *c != '\0'; c++) {
        glutBitmapCharacter(GLUT_BITMAP_TIMES_ROMAN_24, *c);
    }

    glColor3f(1.0f, 1.0f, 1.0f); // White color for instructions
    const char* restartText = "Press R to Restart";
    glRasterPos2f((WIDTH - glutBitmapLength(GLUT_BITMAP_HELVETICA_18, (const unsigned char*)restartText)) / 2, HEIGHT / 2 - 20);
    for (const char* c = restartText; *c != '\0'; c++) {
        glutBitmapCharacter(GLUT_BITMAP_HELVETICA_18, *c);
    }

    const char* exitText = "Press ESC to Exit";
    glRasterPos2f((WIDTH - glutBitmapLength(GLUT_BITMAP_HELVETICA_18, (const unsigned char*)exitText)) / 2, HEIGHT / 2 - 50);
    for (const char* c = exitText; *c != '\0'; c++) {
        glutBitmapCharacter(GLUT_BITMAP_HELVETICA_18, *c);
    }

    glEnable(GL_TEXTURE_2D); // Re-enable textures
    glEnable(GL_DEPTH_TEST);
    // Restore the original projection and modelview matrices
    glMatrixMode(GL_PROJECTION);
    glPopMatrix();

    glMatrixMode(GL_MODELVIEW);
    glPopMatrix();
}

void RenderFadeEffect() {
    if (transitionAlpha > 0.0f) {
        // Disable depth test and lighting to render the fade overlay
        glDisable(GL_DEPTH_TEST);
        glDisable(GL_LIGHTING);
        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

        // Render a full-screen quad with the fade color (black)
        glColor4f(0.0f, 0.0f, 0.0f, transitionAlpha); // Black color with alpha

        glMatrixMode(GL_PROJECTION);
        glPushMatrix();
        glLoadIdentity();
        gluOrtho2D(-1.0, 1.0, -1.0, 1.0); // Normalized device coordinates for full-screen quad

        glMatrixMode(GL_MODELVIEW);
        glPushMatrix();
        glLoadIdentity();

        glBegin(GL_QUADS);
        glVertex2f(-1.0f, -1.0f);
        glVertex2f(1.0f, -1.0f);
        glVertex2f(1.0f, 1.0f);
        glVertex2f(-1.0f, 1.0f);
        glEnd();
        
        // Restore previous matrices and OpenGL state
        glPopMatrix();
        glMatrixMode(GL_PROJECTION);
        glPopMatrix();
        glMatrixMode(GL_MODELVIEW);

        glDisable(GL_BLEND);
        glEnable(GL_DEPTH_TEST);
        glEnable(GL_LIGHTING);
    }
}
void UpdateFlickeringLights() {
    if (isMuzzleFlashActive) {
        return;
    }
    for (size_t i = 0; i < lampLightIntensity.size(); ++i) {
        // Use a hardcoded sine wave-based flicker pattern
        float timeFactor = glutGet(GLUT_ELAPSED_TIME) * 0.001f * flickerSpeeds[i];
        lampLightIntensity[i] = 0.7f + 0.3f * sin(timeFactor); // Fixed sine-based intensity
         
        // Clamp intensity between [0.2, 1.0] to avoid extreme brightness or dimness
        if (lampLightIntensity[i] < 0.1f) lampLightIntensity[i] = 0.1f;
        if (lampLightIntensity[i] > 1.0f) lampLightIntensity[i] = 1.0f;

        // Update the global ambient light proportionally
        GLfloat ambientFactor = lampLightIntensity[i] * 0.5f;
        GLfloat ambient[] = { ambientFactor, ambientFactor, ambientFactor, 1.0f };
        glLightModelfv(GL_LIGHT_MODEL_AMBIENT, ambient); // Update ambient light globally

        // Update diffuse light properties for each lamp
        GLfloat diffuse[] = { lampLightIntensity[i], lampLightIntensity[i], lampLightIntensity[i], 1.0f };
        GLfloat position[] = { lampPositionsX[i], 10.0f, lampPositionsZ[i], 1.0f };

        GLenum lightId = GL_LIGHT2 + i; // Unique light ID
        glLightfv(lightId, GL_DIFFUSE, diffuse);
        glLightfv(lightId, GL_POSITION, position);
    }
}


void RenderLamps() {
    for (size_t i = 0; i < lampPositionsX.size(); ++i) {
        GLenum lightId = GL_LIGHT1 + i; // Use unique light IDs
        glEnable(lightId); // Ensure the light is enabled

        glPushMatrix();
        glTranslatef(lampPositionsX[i], 9.0f, lampPositionsZ[i]);
        glScalef(0.1f, 0.1f, 0.1f); 
        model_light.Draw();
        glPopMatrix();
    }
}

void InitializeLamps() {
    lampPositionsX = { -50.0f, 0.0f, 50.0f }; 
    lampPositionsZ = { -50.0f, 0.0f, 50.0f };

    // Initialize light intensity and random flicker speeds
    lampLightIntensity.resize(lampPositionsX.size(), 1.0f);
    flickerSpeeds.resize(lampPositionsX.size());

    for (auto& speed : flickerSpeeds) {
        speed = 1.0f + static_cast<float>(rand() % 50) / 50.0f; // Random speeds between 1.0 and 2.0
    }

    for (size_t i = 0; i < lampPositionsX.size(); ++i) {
        GLenum lightId = GL_LIGHT2 + i; // Use GL_LIGHT1, GL_LIGHT2, etc.
        glEnable(lightId);

        GLfloat ambient[] = { 0.1f, 0.1f, 0.1f, 1.0f };
        GLfloat diffuse[] = { lampLightIntensity[i], lampLightIntensity[i], lampLightIntensity[i], 1.0f };
        GLfloat position[] = { lampPositionsX[i], 9.0f, lampPositionsZ[i], 1.0f };

        // Add attenuation for realism
        glLightf(lightId, GL_CONSTANT_ATTENUATION, 1.0f);
        glLightf(lightId, GL_LINEAR_ATTENUATION, 0.05f);
        glLightf(lightId, GL_QUADRATIC_ATTENUATION, 0.01f);

        glLightfv(lightId, GL_AMBIENT, ambient);
        glLightfv(lightId, GL_DIFFUSE, diffuse);
        glLightfv(lightId, GL_POSITION, position);
    }
}
void RenderCrates() {
    for (const auto& crate : crates) {
        glPushMatrix();

        // Set position
        glTranslatef(crate.x, crate.y -0.1f , crate.z);

        // Apply rotation
        glRotatef(crate.rotation, 0.0f, 1.0f, 0.0f);

        // Scale the crate
        glScalef(crate.scale, crate.scale, crate.scale);

        // Render the crate model
        model_crate.Draw();

        glPopMatrix();
    }
}

void RenderRubbish() {
    for (const auto& rubbish : rubbishObjects) {
        glPushMatrix();

        // Position the rubbish
        glTranslatef(rubbish.x, rubbish.y
            , rubbish.z);

        // Apply rotation for a random look
        glRotatef(rubbish.rotation, 0.0f, 1.0f, 0.0f);

        // Scale the rubbish
        glScalef(rubbish.scale, rubbish.scale , rubbish.scale );

        // Render the rubbish model
        model_rubbish.Draw();

        glPopMatrix();
    }
}

void RenderGameWinScreen() {
    glDisable(GL_LIGHTING);
    glEnable(GL_TEXTURE_2D);

    glMatrixMode(GL_PROJECTION);
    glPushMatrix();
    glLoadIdentity();
    gluOrtho2D(0.0, WIDTH, 0.0, HEIGHT);

    glMatrixMode(GL_MODELVIEW);
    glPushMatrix();
    glLoadIdentity();

    tex_startScreen.Use(); // Use the same start screen texture as background
    glBegin(GL_QUADS);
    glTexCoord2f(0.0f, 0.0f); glVertex2f(0.0f, 0.0f);
    glTexCoord2f(1.0f, 0.0f); glVertex2f(WIDTH, 0.0f);
    glTexCoord2f(1.0f, 1.0f); glVertex2f(WIDTH, HEIGHT);
    glTexCoord2f(0.0f, 1.0f); glVertex2f(0.0f, HEIGHT);
    glEnd();

    glDisable(GL_TEXTURE_2D);
    glDisable(GL_DEPTH_TEST);

    glColor3f(0.0f, 1.0f, 0.0f); // Green color for "You Win!" message

    const char* winText = "CONGRATULATIONS! YOU WIN!";
    glRasterPos2f((WIDTH - glutBitmapLength(GLUT_BITMAP_HELVETICA_18, (const unsigned char*)winText)) / 2, HEIGHT / 2 + 50);
    for (const char* c = winText; *c != '\0'; c++) {
        glutBitmapCharacter(GLUT_BITMAP_HELVETICA_18, *c);
    }

    glColor3f(1.0f, 1.0f, 1.0f); // White color for restart and exit instructions
    const char* restartText = "Press R to Restart";
    glRasterPos2f((WIDTH - glutBitmapLength(GLUT_BITMAP_HELVETICA_18, (const unsigned char*)restartText)) / 2, HEIGHT / 2 - 20);
    for (const char* c = restartText; *c != '\0'; c++) {
        glutBitmapCharacter(GLUT_BITMAP_HELVETICA_18, *c);
    }

    const char* exitText = "Press ESC to Exit";
    glRasterPos2f((WIDTH - glutBitmapLength(GLUT_BITMAP_HELVETICA_18, (const unsigned char*)exitText)) / 2, HEIGHT / 2 - 50);
    for (const char* c = exitText; *c != '\0'; c++) {
        glutBitmapCharacter(GLUT_BITMAP_HELVETICA_18, *c);
    }

    glEnable(GL_LIGHTING);
    glEnable(GL_DEPTH_TEST);


    glMatrixMode(GL_PROJECTION);
    glPopMatrix();

    glMatrixMode(GL_MODELVIEW);
    glPopMatrix();
}

void myDisplay(void) {
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    float pitch_rad = playerPitch * (PI / 180.0f);
    float yaw_rad = playerYaw * (PI / 180.0f);

    float lookX = cos(pitch_rad) * cos(yaw_rad);
    float lookY = sin(pitch_rad);
    float lookZ = cos(pitch_rad) * sin(yaw_rad);
    if (!isTransitioningToLevel2) {

    }
    if (currentGameState == START_SCREEN) {
        RenderStartScreen();
        PlayIdleMusic();
        glutSwapBuffers();
        return;
    }
    else if (currentGameState == LEVEL_1) {
        glLoadIdentity();
        StopIdleMusic();
        UpdateLighting(gameTime);

        if (isThirdPerson) {
            float camX = playerX - (cos(yaw_rad) * cameraDistance);
            float camY = playerY + cameraHeight;
            float camZ = playerZ - (sin(yaw_rad) * cameraDistance);

            gluLookAt(
                camX, camY, camZ,
                playerX, playerY + 1.0f, playerZ,
                0.0f, 1.0f, 0.0f
            );
        }
        else {
            gluLookAt(
                playerX, playerY, playerZ,
                playerX + lookX, playerY + lookY, playerZ + lookZ,
                0.0f, 1.0f, 0.0f
            );
        }

        RenderSky();
        // Render scene objects
        RenderGround();
        RenderWalls();
        RenderTrees();
        RenderBarrels();



        // RenderTargets(); // Render targets
        RenderBullseyeTargets();    // Draw House
        glPushMatrix();
        glTranslatef(0, 0, 20);
        glScalef(0.7, 0.7, 0.7);
        model_house.Draw();
        glPopMatrix();

        // Render player last for proper transparency
        RenderPlayer();
        RenderFirstPersonWeapon();
        RenderBullets();
        RenderMuzzleFlash();

        if (!isThirdPerson) {
            // Switch to orthographic projection
            glMatrixMode(GL_PROJECTION);
            glPushMatrix();
            glLoadIdentity();
            gluOrtho2D(0, WIDTH, 0, HEIGHT);

            glMatrixMode(GL_MODELVIEW);
            glPushMatrix();
            glLoadIdentity();

            // Disable depth testing and lighting
            glDisable(GL_DEPTH_TEST);
            glDisable(GL_LIGHTING);

            // Set crosshair color (white)
            glColor3f(1.0f, 1.0f, 1.0f);

            // Draw crosshair lines
            glBegin(GL_LINES);
            // Horizontal line
            glVertex2f(WIDTH / 2 - 10, HEIGHT / 2);
            glVertex2f(WIDTH / 2 + 10, HEIGHT / 2);
            // Vertical line
            glVertex2f(WIDTH / 2, HEIGHT / 2 - 10);
            glVertex2f(WIDTH / 2, HEIGHT / 2 + 10);
            glEnd();

            // Re-enable depth testing and lighting
            glEnable(GL_DEPTH_TEST);
            glEnable(GL_LIGHTING);

            // Restore the original projection and modelview matrices
            glPopMatrix(); // Pop modelview matrix
            glMatrixMode(GL_PROJECTION);
            glPopMatrix(); // Pop projection matrix
            glMatrixMode(GL_MODELVIEW);
        }

        int currentScore = 0;
        for (const auto& target : bullseyeTargets) {
            if (target.isHit) currentScore++;
        }
        int totalTargets = bullseyeTargets.size();
        int remainingTime = static_cast<int>(1080.0f - gameTime); // Assuming a total game time of 360 seconds

        RenderHUD(currentScore, totalTargets, remainingTime);

       // glutSwapBuffers();
    }

    else if (currentGameState == END_SCREEN_LOSE) {
        glLoadIdentity();
        PlayIdleMusic();
        RenderGameOverScreen();
        glutSwapBuffers();
    }

    else if (currentGameState == LEVEL_2) {
        glLoadIdentity();
        StopIdleMusic();
        if (isThirdPerson) {
            float camX = playerX - (cos(yaw_rad) * cameraDistance);
            float camY = playerY + cameraHeight;
            float camZ = playerZ - (sin(yaw_rad) * cameraDistance);

            gluLookAt(
                camX, camY, camZ,
                playerX, playerY + 1.0f, playerZ,
                0.0f, 1.0f, 0.0f
            );
        }
        else {
            gluLookAt(
                playerX, playerY, playerZ,
                playerX + lookX, playerY + lookY, playerZ + lookZ,
                0.0f, 1.0f, 0.0f
            );
        }
                UpdateFlickeringLights(); // Update flickering light intensities

        // Render scene objects
                RenderSky();
        RenderCeiling();

        RenderLamps(); // Render lamp models
        RenderCrates();
        RenderGroundL2();
        RenderWallsL2();
        RenderRubbish();

       // RenderTrees();
        RenderTargets();
        RenderBouncingTargets();
        RenderPlayer();
        RenderFirstPersonWeapon();
        RenderBullets();
        RenderMuzzleFlash();

        if (!isThirdPerson) {
            // Switch to orthographic projection
            glMatrixMode(GL_PROJECTION);
            glPushMatrix();
            glLoadIdentity();
            gluOrtho2D(0, WIDTH, 0, HEIGHT);
                
            glMatrixMode(GL_MODELVIEW);
            glPushMatrix();
            glLoadIdentity();

            // Disable depth testing and lighting
            glDisable(GL_DEPTH_TEST);
            glDisable(GL_LIGHTING);

            // Set crosshair color (white)
            glColor3f(1.0f, 1.0f, 1.0f);

            // Draw crosshair lines
            glBegin(GL_LINES);
            // Horizontal line
            glVertex2f(WIDTH / 2 - 10, HEIGHT / 2);
            glVertex2f(WIDTH / 2 + 10, HEIGHT / 2);
            // Vertical line
            glVertex2f(WIDTH / 2, HEIGHT / 2 - 10);
            glVertex2f(WIDTH / 2, HEIGHT / 2 + 10);
            glEnd();

            // Re-enable depth testing and lighting
            glEnable(GL_DEPTH_TEST);
            glEnable(GL_LIGHTING);

            // Restore the original projection and modelview matrices
            glPopMatrix(); // Pop modelview matrix
            glMatrixMode(GL_PROJECTION);
            glPopMatrix(); // Pop projection matrix
            glMatrixMode(GL_MODELVIEW);
        }

        int totalTargets =13 ;          // Total targets in Level 2
        int remainingTime = static_cast<int>(1080.0f - gameTime); // Remaining time in seconds

        RenderHUD(level2Score, totalTargets, remainingTime);

    }
    else if (currentGameState == END_SCREEN_WIN) {
        glLoadIdentity();
        PlayIdleMusic();
        RenderGameWinScreen();
        glutSwapBuffers();
}

    
        
        RenderFadeEffect();

    
    glutSwapBuffers();

}

void resetLevel1() {
    // Reset game time
    gameTime = 0.0f;

    // Reset player position and orientation
    playerX = 0.0f;
    playerY = 1.8f;
    playerZ = 5.0f;
    playerYaw = -90.0f;
    playerPitch = 0.0f;
    playerRotation = 180.0f;

    // Reset targets
    for (auto& target : bullseyeTargets) {
        target.isHit = false;
        target.isFalling = false;
        target.rotationY = 0.0f;
        target.rotationZ = 0.0f;
    }

    for (auto& target : targets) {
        target.isHit = false;
        target.rotationX = 0.0f;
        target.isRotating = true; // Reset rotation state
    }

    // Clear bullets
    bullets.clear();

    // Reset muzzle flash
    isMuzzleFlashActive = false;
    muzzleFlashTimer = 0.0f;
    recoilOffset = 0.0f;

    // Reset lighting (optional, to avoid leftover effects)
    UpdateAmbientLight(false);
}
bool CheckAABBCollision(float x1, float z1, float size1, float x2, float z2, float size2, float boundaryMultiplier = 1.5f) {
    float expandedSize1 = size1 * boundaryMultiplier;
    float expandedSize2 = size2 * boundaryMultiplier;

    return (x1 - expandedSize1 <= x2 + expandedSize2 && x1 + expandedSize1 >= x2 - expandedSize2) &&
        (z1 - expandedSize1 <= z2 + expandedSize2 && z1 + expandedSize1 >= z2 - expandedSize2);
}


void myKeyboard(unsigned char key, int x, int y) {
    float yaw_rad = playerYaw * (PI / 180.0f);

    // Calculate forward and right vectors
    float lookX = cos(yaw_rad);
    float lookZ = sin(yaw_rad);
    float rightX = -lookZ;
    float rightZ = lookX;

    // Define ground boundaries
    float groundMin = -100.0f;
    float groundMax = 100.0f;

    float newX = playerX;
    float newZ = playerZ;
    if (currentGameState == START_SCREEN) {
        if (key == 13) { // ENTER key
            currentGameState = LEVEL_1; // Transition to level 1
        }
        else if (key == 27) { // ESC key
            exit(0);
        }
        return;
    }
    else if (currentGameState == LEVEL_1 || currentGameState == LEVEL_2) {
        switch (key) {
        case 'w':
            newX = playerX + lookX * playerSpeed;
            newZ = playerZ + lookZ * playerSpeed;
            break;
        case 's':
            newX = playerX - lookX * playerSpeed;
            newZ = playerZ - lookZ * playerSpeed;
            break;
        case 'a':
            newX = playerX - rightX * playerSpeed;
            newZ = playerZ - rightZ * playerSpeed;
            break;
        case 'd':
            newX = playerX + rightX * playerSpeed;
            newZ = playerZ + rightZ * playerSpeed;
            break;
        case 'v': // Toggle view
            isThirdPerson = !isThirdPerson;
            break;
        case 27: // ESC
            exit(0);
            break;
        }

        bool collision = false;

        // Level 1: Check against trees and barrels
        if (currentGameState == LEVEL_1) {
            for (const auto& tree : trees) {
                if (CheckAABBCollision(newX, newZ, 1.0f, tree.x, tree.z, tree.scale)) {
                    collision = true;
                    break;
                }
            }
            for (const auto& barrel : barrels) {
                if (CheckAABBCollision(newX, newZ, 1.0f, barrel.x, barrel.z, barrel.scale)) {
                    collision = true;
                    break;
                }
            }
            // Check collision with the house
            float houseMinX = -5.0f, houseMaxX = 5.0f;  
            float houseMinZ = 15.0f, houseMaxZ = 25.0f; 
            if (newX >= houseMinX && newX <= houseMaxX && newZ >= houseMinZ && newZ <= houseMaxZ) {
                collision = true;
            }
        }

        // Level 2: Check against crates and rubbish
        if (currentGameState == LEVEL_2) {
            for (const auto& crate : crates) {
                if (CheckAABBCollision(newX, newZ, 1.0f, crate.x, crate.z, crate.scale, 1.8f)) {
                    collision = true;
                    break;
                }
            }
            for (const auto& rubbish : rubbishObjects) {
                if (CheckAABBCollision(newX, newZ, 1.0f, rubbish.x, rubbish.z, rubbish.scale, 1.0f)) {
                    collision = true;
                    break;
                }
            }
        }
        if (collision) {
            if (thudSound != nullptr) {
                Mix_PlayChannel(-1, thudSound, 0);
            }
        }
        // Apply movement only if no collision
        if (!collision &&
            newX >= groundMin && newX <= groundMax &&
            newZ >= groundMin && newZ <= groundMax) {
            playerX = newX;
            playerZ = newZ;
        }
    }
    else if (currentGameState == END_SCREEN_LOSE) {
        if (key == 'r') {
            resetLevel1();
            currentGameState = LEVEL_1; // Transition to level 1
        }
        else if (key == 27) { // ESC key
            exit(0);
        }
    }
    else if (currentGameState == END_SCREEN_WIN) {
        if (key == 'r') {
            resetLevel1();
            currentGameState = LEVEL_1;
        }
        else if (key == 27) { // ESC
            exit(0);
        }
    }
    glutPostRedisplay();

    
}
void LoadAssets() {
    model_house.Load("Models/house/house.3DS");
    model_tree.Load("Models/tree/Tree1.3ds");
    model_player.Load("Models/player/Soldier.3ds"); // Load the player model
    model_rifle.Load("Models/rifle/rifle.3ds");  // Load the rifle model
    tex_ground.Load("Textures/ground.bmp");
    model_barrel.Load("Models/barrel/barrel.3ds");

    model_bullet.Load("Models/bullet/Bullet.3ds");
    if (!model_target1.LoadFromFile("Models/target2/target2.obj")) {
        std::cerr << "Failed to load model!" << std::endl;
    }

    muzzle_flash.Load("Textures/muzzle_flash5.tga");
    wall1.Load("Textures/brickwall3.bmp");

    sky_morning.Load("Textures/sky_morning.bmp");
    sky_afternoon.Load("Textures/sky_afternoon.bmp");
    sky_evening.Load("Textures/evening_sky.bmp");
    sky_night.Load("Textures/sky_night.bmp");
    GLuint textures[] = { sky_morning.texture[0], sky_afternoon.texture[0],
                      sky_evening.texture[0], sky_night.texture[0] };
    for (GLuint tex : textures) {
        glBindTexture(GL_TEXTURE_2D, tex);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    }

    bullseye.Load("Textures/bullseye.bmp");
    tex_startScreen.Load("Textures/startGame.bmp");
    ground_l2.Load("Textures/groundL2.bmp");
    ceiling.Load("Textures/ceiling.bmp");
    wall2.Load("Textures/wall_l2.bmp");
    model_light.Load("Models/lamp/lamps.3ds");
    model_crate.Load("Models/crates/xcrate.3ds");
    model_rubbish.Load("Models/rubbish2/pack.3ds");
    targetTexture.Load("Textures/image.bmp");
    shootSound = Mix_LoadWAV("sounds/shoot_sound.mp3");
    if (shootSound == nullptr) {
        std::cerr << "Failed to load shooting sound! SDL_mixer Error: " << Mix_GetError() << "\n";
    }

    idleMusic = Mix_LoadMUS("sounds/idle_sound.mp3"); // Replace with your music file path
    if (idleMusic == nullptr) {
        std::cerr << "Failed to load idle music! SDL_mixer Error: " << Mix_GetError() << "\n";
    }
    else {
        std::cout << "Idle music loaded successfully.\n";
    }

    hitSound = Mix_LoadWAV("sounds/target_hit.mp3"); // Replace with your sound file path
    if (hitSound == nullptr) {
        std::cerr << "Failed to load hit sound! SDL_mixer Error: " << Mix_GetError() << "\n";
    }
    else {
        std::cout << "Hit sound loaded successfully.\n";
    }
    thudSound = Mix_LoadWAV("sounds/thud.mp3"); // Replace with your thud sound file path
    if (thudSound == nullptr) {
        std::cerr << "Failed to load thud sound! SDL_mixer Error: " << Mix_GetError() << "\n";
    }
    else {
        std::cout << "Thud sound loaded successfully.\n";
    }


}
void UpdateTargets() {
    for (auto& target : targets) {
        if (target.isRotating) {
            target.rotationX -= 1.0f; // Rotate -1 degree per frame

            // Check if the target has reached -90 degrees
            if (target.rotationX <= -90.0f) {
                target.rotationX = -90.0f;  // Stop at -90 degrees
                target.isRotating = false; // Reverse direction
            }
        }
        else {
            target.rotationX += 1.0f; // Rotate +1 degree per frame

            // Check if the target has returned upright (0 degrees)
            if (target.rotationX >= 0.0f) {
                target.rotationX = 0.0f;   // Stop at upright
                target.isRotating = true; // Reverse direction
            }
        }
    }
}
void UpdateTargetsL2() {
    for (auto& target : targets) {
        if (target.isHit) {
            if (!target.isDisappearing) {
                // Start falling animation
                target.isDisappearing = true;
                target.fallAngle = 0.0f; // Initialize fall angle
            }
            else {
                // Falling and shrinking animation
                if (target.fallAngle < 90.0f) {
                    target.fallAngle += 2.0f; // Rotate about Z-axis
                }
                else if (target.scale > 0.0f) {
                    target.scale -= 0.05f; // Shrink after falling
                }
                else {
                    target.scale = 0.0f; // Fully disappear
                }
            }
        }
        else if (target.isRotating) {
            // Regular rotation for visible targets
            target.rotationX -= 1.0f;
            if (target.rotationX <= -90.0f) {
                target.rotationX = -90.0f;
                target.isRotating = false;
            }
        }
    }
}

bool CheckCollision(const Bullet& bullet, const Target1& target) {
    // Calculate the squared distance between the bullet and the target
    float dx = bullet.x - target.x;
    float dz = bullet.z - target.z;
    float distanceSquared = dx * dx + dz * dz;

    // Calculate the squared radius of the target
    float targetRadius = target.scale * 5.0f; 
    float radiusSquared = targetRadius * targetRadius;

    // Check if the distance is less than or equal to the radius
    return distanceSquared <= radiusSquared;
}
bool CheckSphereCollision(const Bullet& bullet, const BouncingTarget& target) {
    float effectiveRadius = target.radius * target.scale; 
    float dx = bullet.x - target.x;
    float dy = bullet.y - (target.y + 1.5f); // Include the bouncing offset
    float dz = bullet.z - target.z;
    float distanceSquared = dx * dx + dy * dy + dz * dz;

    return distanceSquared <= effectiveRadius * effectiveRadius;
}
void resetLevel2() {
    // Reset game time
    gameTime = 0.0f;

    // Reset player position and orientation
    playerX = 0.0f;
    playerY = 1.8f;
    playerZ = 5.0f;
    playerYaw = -90.0f;
    playerPitch = 0.0f;
    playerRotation = 180.0f;

    // Reset bouncing targets
    for (auto& target : bouncingTargets) {
        target.isHit = false;
        target.isDisappearing = false;
        target.scale = 1.0f; // Reset scale to default
    }

    // Reset hidden targets
    for (auto& target : targets) {
        target.isHit = false;
        target.isDisappearing = false;
        target.rotationX = 0.0f; // Reset rotation
        target.scale = 0.2f;     // Reset to initial scale
        target.isRotating = true;
    }

    // Clear bullets
    bullets.clear();

    // Reset muzzle flash
    isMuzzleFlashActive = false;
    muzzleFlashTimer = 0.0f;
    recoilOffset = 0.0f;

    // Reset lamp light intensities (if required)
    for (auto& intensity : lampLightIntensity) {
        intensity = 1.0f;
    }

    // Reset the level score
    level2Score = 0;

    // Reset lighting (optional, to avoid leftover effects)
    UpdateAmbientLight(false);
}


void UpdateBullets() {
    for (auto bulletIt = bullets.begin(); bulletIt != bullets.end();) {
        // Update bullet position
        bulletIt->x += bulletIt->directionX * bulletIt->speed;
        bulletIt->y += bulletIt->directionY * bulletIt->speed;
        bulletIt->z += bulletIt->directionZ * bulletIt->speed;

        bool collisionDetected = false;

        // Check for collision with each bullseye target
        if (currentGameState == LEVEL_1) {
            for (auto& target : bullseyeTargets) {
                if (!target.isHit && CheckBullseyeCollision(*bulletIt, target)) {
                    target.isHit = true; // Mark the target as hit
                    if (hitSound != nullptr) {
                        Mix_PlayChannel(-1, hitSound, 0);
                    }
                    collisionDetected = true;
                    std::cout << "Hit bullseye at (" << target.x << ", " << target.z << ")\n";
                    break; // Bullet only hits one target
                }
            }
        }
       

        for (auto& target : bouncingTargets) {
            if (!target.isHit && CheckSphereCollision(*bulletIt, target)) {
                target.isHit = true; // Mark as hit
                target.isDisappearing = true;
                if (hitSound != nullptr) {
                    Mix_PlayChannel(-1, hitSound, 0);
                }// Start disappearance animation
                collisionDetected = true;
                level2Score++; // Increment score
                break; // Bullet only hits one target
            }
        }

        // Check collision with hidden targets
        for (auto& target : targets) {
            if (!target.isHit && CheckCollision(*bulletIt, target)) {
                target.isHit = true; // Mark as hit
                target.isDisappearing = true; 
                if (hitSound != nullptr) {
                    Mix_PlayChannel(-1, hitSound, 0);
                }// Start disappearance animation
                collisionDetected = true;
                level2Score++; // Increment score
                break; // Bullet only hits one target
            }
        }


        // Remove bullet if it hits a target
        if (collisionDetected) {
            bulletIt = bullets.erase(bulletIt);
        }
        else {
            ++bulletIt; // Move to the next bullet
        }
    }
}
void UpdateBullseyeTargets() {
    for (auto& target : bullseyeTargets) {
        if (target.isHit) {
            if (!target.isFalling) {
                // Rotate about Y-axis for self-spin
                target.rotationY += 5.0f; 
                if (target.rotationY >= 360.0f) {
                    target.rotationY = 0.0f;  // Reset Y-axis rotation
                    target.isFalling = true; // Start falling animation
                }
            }
            else {
                // Rotate about Z-axis for falling
                target.rotationZ += 2.0f; 
                if (target.rotationZ >= 90.0f) {
                    target.rotationZ = 90.0f; // Cap fall angle
                }
            }
        }
    }
}
void specialKeys(int key, int x, int y) {
    switch (key) {
    case GLUT_KEY_LEFT:
        playerYaw -= keyRotationSpeed;
        if (playerYaw < 0.0f) playerYaw += 360.0f;
        break;
    case GLUT_KEY_RIGHT:
        playerYaw += keyRotationSpeed;
        if (playerYaw > 360.0f) playerYaw -= 360.0f;
        break;
    case GLUT_KEY_UP:
        playerPitch += keyRotationSpeed;
        if (playerPitch > 89.0f) playerPitch = 89.0f;
        break;
    case GLUT_KEY_DOWN:
        playerPitch -= keyRotationSpeed;
        if (playerPitch < -89.0f) playerPitch = -89.0f;
        break;
    }
    glutPostRedisplay();
}
void myInit() {
    glClearColor(0.0, 0.0, 0.0, 0.0);

    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluPerspective(45.0, (GLdouble)WIDTH / (GLdouble)HEIGHT, 0.1, 200.0);

    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();

    // Enable necessary features
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_LIGHTING);
    glEnable(GL_LIGHT0);
    glEnable(GL_NORMALIZE);
    glEnable(GL_COLOR_MATERIAL);

    // Setup lighting
    GLfloat ambient[] = { 0.2f, 0.2f, 0.2f, 1.0f };
    GLfloat diffuse[] = { 0.8f, 0.8f, 0.8f, 1.0f };
    GLfloat position[] = { 0.0f, 10.0f, 0.0f, 1.0f };
    glLightfv(GL_LIGHT0, GL_AMBIENT, ambient);
    glLightfv(GL_LIGHT0, GL_DIFFUSE, diffuse);
    glLightfv(GL_LIGHT0, GL_POSITION, position);

    InitializeLamps(); // Add this call
    initAudio();

}
void myIdle() {
    if(currentGameState == LEVEL_1 ) UpdateLighting(gameTime); // Update lighting
    if (currentGameState == LEVEL_2 && !isTransitioningToLevel2) {
        UpdateFlickeringLights();
        UpdateBouncingTargets();
        gameTime += (timeSpeed);
        UpdateTargetsL2();

        if (level2Score == 13 && gameTime <= 1080.0f) {
            currentGameState = END_SCREEN_WIN; // All targets hit
            resetLevel2();
        }
        else if (gameTime >= 1080.0f && level2Score < 13) { // Timer expires
            currentGameState = END_SCREEN_LOSE; // Game over due to timeout
            resetLevel2();
        }
    }

    if (!isTransitioningToLevel2) {
        UpdateTargets();
        UpdateBullets();
        UpdateBullseyeTargets();

        if (isMuzzleFlashActive) {
            muzzleFlashTimer -= 0.01f;
            if (muzzleFlashTimer <= 0.0f) {
                isMuzzleFlashActive = false;
                UpdateAmbientLight(false);
            }
        }

        if(currentGameState == LEVEL_1) gameTime += (timeSpeed);

        // Check transition to Level 2
        int currentScore = 0;
        for (const auto& target : bullseyeTargets) {
            if (target.isHit) currentScore++;
        }

        if (currentGameState == LEVEL_1 && currentScore == 15) {
            isTransitioningToLevel2 = true; // Start the transition
        }
        else if (currentGameState == LEVEL_1 && gameTime >= 1080.0f) {
            currentGameState = END_SCREEN_LOSE; // Time ran out
            std::cout << "Game Over! Final Score: " << currentScore << "/15\n";
        }
    }
    else {
        
       
    }
    if (isTransitioningToLevel2) {
            transitionAlpha += transitionSpeed;
            if (transitionAlpha >= 1.0f) {
                transitionAlpha = 1.0f; // Fully black

                resetLevel1();               // Reset targets and player position
                currentGameState = LEVEL_2;  // Move to Level 2
                isTransitioningToLevel2 = false; // End the transition
            }
     }

    if (!isTransitioningToLevel2) {
        transitionAlpha -= transitionSpeed; // Decrease alpha (fade back in)
        if (transitionAlpha < 0.0f) {
            transitionAlpha = 0.0f; // Fully visible
        }
    }
    glutPostRedisplay();
}



void main(int argc, char** argv) {
    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH);
    glutInitWindowSize(WIDTH, HEIGHT);
    glutCreateWindow("Die Hard!");

    myInit();
    LoadAssets();
    glutSpecialFunc(specialKeys);        // Register special keys callback
    glutDisplayFunc(myDisplay);
    glutKeyboardFunc(myKeyboard);
    glutPassiveMotionFunc(myMouseMotion);  // Enable mouse motion callback
    glutMouseFunc(myMouse);  // Enable mouse button callback
    // Center the mouse cursor
    glutWarpPointer(WIDTH / 2, HEIGHT / 2);

    glutIdleFunc(myIdle);
    glutMainLoop();

    if (shootSound != nullptr) {
        Mix_FreeChunk(shootSound);
        shootSound = nullptr;
    }

    if (hitSound != nullptr) {
        Mix_FreeChunk(hitSound);
        hitSound = nullptr;
    }
    if (thudSound != nullptr) {
        Mix_FreeChunk(thudSound);
        thudSound = nullptr;
    }
    Mix_CloseAudio();
    SDL_Quit();
}
