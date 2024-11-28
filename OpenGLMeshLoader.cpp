#include "TextureBuilder.h"
#include "Model_3DS.h"
#include "GLTexture.h"
#include "OBJModel.h"
#include <glut.h>
#include <cmath>
#include <iostream>
#include <vector>


enum GameState {
    START_SCREEN,
    LEVEL_1,
    LEVEL_2, 
    END_SCREEN
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
float playerSpeed = 0.1f;
float mouseSensitivity = 0.1f;
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



float gameTime = 0.0f;       // Game time in seconds
float timeSpeed = 0.1f;      // Speed of time progression (adjust for faster/slower transitions)
float sunAngle = 0.0f;       // Angle of the sun (degrees)
float maxIntensity = 1.0f;   // Maximum light intensity
float minIntensity = 0.1f;   // Minimum light intensity
float lightIntensity = maxIntensity;  // Current light intensity
GLfloat lightPosition[] = { 0.0f, 10.0f, 0.0f, 1.0f }; // Initial light position
const float M_PI = 3.14159265358979323846;


// Models
Model_3DS model_house;
Model_3DS model_tree;
Model_3DS model_player;
Model_3DS model_rifle;  // New rifle model
Model_3DS model_barrel;
OBJModel model_target1;;
Model_3DS model_bullet;

GLTexture sky_morning;  // Declare the sky texture globally
GLTexture sky_afternoon;
GLTexture sky_evening;
GLTexture sky_night;
GLTexture tex_ground;
GLTexture muzzle_flash;
GLTexture wall1;
GLTexture bullseye;
GLTexture tex_startScreen;


struct CircularTarget {
    float x, y, z;      // Position
    float radius;       // Radius of the circular target
    bool isHit;         // Whether the target is hit
    bool isFalling;     // Whether the target is falling
    float rotationY;    // Rotation about Y-axis (spin)
    float rotationZ;    // Rotation about Z-axis (falling)
};
struct Target1 {
    float x, z;       // X and Z position on the ground
    float scale;      // Scaling factor
    float rotationX;  // Rotation about the X-axis
    bool isRotating;  // Whether the target rotates
    bool isHit;       // Whether the target is hit 
};
struct BarrelPosition {
    float x, z;     // X and Z position on the ground
    float scale;    // Scaling factor
    float rotation; // Rotation angle
};
struct TreePosition {
    float x, z;
    float scale;
    float rotation;
};
struct Bullet {
    float x, y, z;  // Position
    float directionX, directionY, directionZ; // where the bullet is going
    float  speed; // Velocity
};

std::vector<Target1> targets = {
    {10.0f, -20.0f, 0.2f, 0.0f, true, false},
    {-30.0f, 15.0f, 0.2f, 0.0f, false, false},
    {35.0f, -40.0f, 0.2f, 0.0f, true, false},
    {60.0f, -70.0f, 0.2f, 0.0f, true, false},
    {-50.0f, 40.0f, 0.2f, 0.0f, false, false},
    {90.0f, 5.0f, 0.2f, 0.0f, true, false},
    {-75.0f, -60.0f, 0.2f, 0.0f, false, false},
    {50.0f, 80.0f, 0.2f, 0.0f, true, false},
    {-90.0f, -40.0f, 0.2f, 0.0f, false, false},
    {100.0f, 20.0f, 0.2f, 0.0f, true, false}
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
    // Targets on top of trees
    {15.0f, 7.0f, -30.0f, 2.5f, false, false, 0.0f, 0.0f},
    {35.0f, 10.0f, -50.0f, 1.8f, false, false, 0.0f, 0.0f},
    {-45.0f, 6.0f, 25.0f, 2.0f, false, false, 0.0f, 0.0f},
    {10.0f, 8.0f, -20.0f, 2.5f, false, false, 0.0f, 0.0f},
    {-30.0f, 9.0f, 30.0f, 1.8f, false, false, 0.0f, 0.0f},

    // Ground targets
    {-20.0f, 2.5f, 10.0f, 3.0f, false, false, 0.0f, 0.0f},
    {0.0f, 2.0f, -15.0f, 2.8f, false, false, 0.0f, 0.0f},
    {40.0f, 2.5f, 5.0f, 3.5f, false, false, 0.0f, 0.0f},
    {-35.0f, 3.0f, -25.0f, 3.0f, false, false, 0.0f, 0.0f},
    {50.0f, 3.5f, -10.0f, 3.2f, false, false, 0.0f, 0.0f},

    // Additional mix
    {-55.0f, 5.0f, -15.0f, 1.8f, false, false, 0.0f, 0.0f}, // Tree
    {60.0f, 2.8f, -30.0f, 3.3f, false, false, 0.0f, 0.0f},  // Ground
    {-20.0f, 6.5f, 40.0f, 2.2f, false, false, 0.0f, 0.0f},  // Tree
    {15.0f, 3.0f, -50.0f, 3.1f, false, false, 0.0f, 0.0f},   // Ground
    {65.0f, 7.0f, 20.0f, 2.5f, false, false, 0.0f, 0.0f},   // Tree
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
std::vector<Bullet> bullets;

void UpdateAmbientLight(bool isFlashing) {
    if (isFlashing) {
        GLfloat ambient[] = { 0.8f, 0.8f, 0.6f, 1.0f }; // Bright ambient light
        glLightModelfv(GL_LIGHT_MODEL_AMBIENT, ambient);
    }
    else {
        GLfloat ambient[] = { 0.2f, 0.2f, 0.2f, 1.0f }; // Default ambient light
        glLightModelfv(GL_LIGHT_MODEL_AMBIENT, ambient);
    }
}
void CalculateWeaponTipPosition(float& x, float& y, float& z) {
    float pitchRadians = playerPitch * (M_PI / 180.0f);
    float yawRadians = playerYaw * (M_PI / 180.0f);

    // More precise weapon tip offset in local weapon coordinates
    float weaponLength = 2.1f;  // Length of the rifle from base to tip
    float weaponOffsetX = 0.3f; // Slight X offset 
    float weaponOffsetY = -0.2f; // Slight Y offset
    float weaponOffsetZ = weaponLength;

    // Transform local offsets to world coordinates
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
    float pitchRadians = playerPitch * (M_PI / 180.0f);
    float yawRadians = playerYaw * (M_PI / 180.0f);

    // More precise weapon tip offset in local weapon coordinates
    float weaponLength = 2.1f;  // Adjust based on your rifle model's orientation
    float weaponOffsetX = 0.3f;
    float weaponOffsetY = -0.2f;
    float weaponOffsetZ = weaponLength;

    // Transform local offsets to world coordinates
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
    float pitchRadians = playerPitch * (M_PI / 180.0f);
    float yawRadians = playerYaw * (M_PI / 180.0f);

    // Get precise weapon tip position
    CalculateWeaponTipPosition(bullet.x, bullet.y, bullet.z);

    // Calculate bullet direction based on camera pitch and yaw
    bullet.directionX = cos(pitchRadians) * cos(yawRadians);
    bullet.directionY = sin(pitchRadians);
    bullet.directionZ = cos(pitchRadians) * sin(yawRadians);
    
    bullet.speed = 0.5f; // Adjust speed as needed

    bullets.push_back(bullet);

    isMuzzleFlashActive = true;
    muzzleFlashTimer = 0.1f;
    recoilOffset = 0.2f; // Apply a small recoil
    UpdateAmbientLight(true);
}
bool CheckBullseyeCollision(const Bullet& bullet, const CircularTarget& target) {
    // Compute the 2D distance between the bullet and the target's center
    float dx = bullet.x - target.x;
    float dz = bullet.z - target.z;
    float distanceSquared = dx * dx + dz * dz;

    // Check if the bullet is within the target's radius
    return distanceSquared <= target.radius * target.radius;
}
void UpdateCamera() {
    float lookX = cos(playerYaw * (M_PI / 180.0f));
    float lookZ = sin(playerYaw * (M_PI / 180.0f));

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
    glTranslatef(0.3f, -0.4f , -0.9f);  // Adjust these values to position the weapon

    // Apply weapon rotation based on camera look direction


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

    glScalef(1.2f, 1.2f, 1.2f);

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
void RenderWalls() {
    float groundMin = -100.0f;
    float groundMax = 100.0f;
    float wallHeight = 10.0f;  // Height of the walls
    float wallThickness = 1.0f; // Thickness of the walls

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
        if (target.isHit) continue; // Skip rendering hit targets

        
        glPushMatrix();

        // Position the target
        glTranslatef(target.x, 1.0f, target.z);

        // Apply rotation about the X-axis
        glRotatef(target.rotationX, 1.0f, 0.0f, 0.0f);

        // Scale the target
        glScalef(target.scale, target.scale, target.scale * 0.1);

        // Render the target model
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
        float yawDegrees = yawRadians * (180.0f / M_PI); // Convert to degrees

        // Calculate pitch (rotation around X-axis)
        float horizontalLength = sqrt(bullet.directionX * bullet.directionX + bullet.directionZ * bullet.directionZ);
        float pitchRadians = atan2(bullet.directionY, horizontalLength);
        float pitchDegrees = pitchRadians * (180.0f / M_PI); // Convert to degrees

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
        float theta1 = i * M_PI / stacks;
        float theta2 = (i + 1) * M_PI / stacks;

        glBegin(GL_QUAD_STRIP);
        for (int j = 0; j <= slices; ++j) {
            float phi = j * 2 * M_PI / slices;

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
void UpdateLighting() {
    // Update game time and sun angle
    gameTime += timeSpeed;
    if (gameTime >= 360.0f) gameTime -= 360.0f; // Reset after full day cycle
    sunAngle = gameTime; // Sun angle progresses with time

    // Compute normalized time (0 to 1) based on the sun's angle
    float normalizedTime = (sin(sunAngle * M_PI / 180.0f) + 1.0f) / 2.0f;

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

        // Enable lighting and normal for proper interaction
        glEnable(GL_LIGHTING);
        glEnable(GL_NORMALIZE);

        // Set a normal pointing upwards for the disk
        glNormal3f(0.0f, 1.0f, 0.0f);

        // Draw the circular target
        GLUquadric* quad = gluNewQuadric();
        gluQuadricTexture(quad, GL_TRUE); // Enable texturing on the disk
        gluDisk(quad, 0.0, target.radius, 32, 1); // Draw a textured disk
        gluDeleteQuadric(quad); // Clean up

        glPopMatrix();
    }

    glDisable(GL_TEXTURE_2D); // Ensure textures are disabled after use
}
void myDisplay(void) {
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glLoadIdentity();

    UpdateLighting();


    float pitch_rad = playerPitch * (M_PI / 180.0f);
    float yaw_rad = playerYaw * (M_PI / 180.0f);

    float lookX = cos(pitch_rad) * cos(yaw_rad);
    float lookY = sin(pitch_rad);
    float lookZ = cos(pitch_rad) * sin(yaw_rad);

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

	

    glutSwapBuffers();
}
void myKeyboard(unsigned char key, int x, int y) {
    float yaw_rad = playerYaw * (M_PI / 180.0f);

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

    // Check boundaries before updating position
    if (newX >= groundMin && newX <= groundMax &&
        newZ >= groundMin && newZ <= groundMax) {
        playerX = newX;
        playerZ = newZ;
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
bool CheckCollision(const Bullet & bullet, const Target1 & target) {
    // Calculate the squared distance between the bullet and the target
    float dx = bullet.x - target.x;
    float dz = bullet.z - target.z;
    float distanceSquared = dx * dx + dz * dz;

    // Calculate the squared radius of the target
    float targetRadius = target.scale * 5.0f; // Adjust the multiplier as needed
    float radiusSquared = targetRadius * targetRadius;

    // Check if the distance is less than or equal to the radius
    return distanceSquared <= radiusSquared;
}
void UpdateBullets() {
    for (auto bulletIt = bullets.begin(); bulletIt != bullets.end();) {
        // Update bullet position
        bulletIt->x += bulletIt->directionX * bulletIt->speed;
        bulletIt->y += bulletIt->directionY * bulletIt->speed;
        bulletIt->z += bulletIt->directionZ * bulletIt->speed;

        bool collisionDetected = false;

        // Check for collision with each bullseye target
        for (auto& target : bullseyeTargets) {
            if (!target.isHit && CheckBullseyeCollision(*bulletIt, target)) {
                target.isHit = true; // Mark the target as hit
                collisionDetected = true;
                std::cout << "Hit bullseye at (" << target.x << ", " << target.z << ")\n";
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
                target.rotationY += 5.0f; // Adjust rotation speed
                if (target.rotationY >= 360.0f) {
                    target.rotationY = 0.0f;  // Reset Y-axis rotation
                    target.isFalling = true; // Start falling animation
                }
            }
            else {
                // Rotate about Z-axis for falling
                target.rotationZ += 2.0f; // Adjust falling speed
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
void myIdle() {
    UpdateLighting();
    UpdateTargets(); // Ensure targets update during idle time
	UpdateBullets(); 
    if (isMuzzleFlashActive) {
        muzzleFlashTimer -= 0.01f;
        if (muzzleFlashTimer <= 0.0f) {
            isMuzzleFlashActive = false;
            UpdateAmbientLight(false); // Restore ambient light

        }
    }
    UpdateBullseyeTargets();
    glutPostRedisplay();
}
void main(int argc, char** argv) {
    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH);
    glutInitWindowSize(WIDTH, HEIGHT);
    glutCreateWindow("3D Scene");

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
}
