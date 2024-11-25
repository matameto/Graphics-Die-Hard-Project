#include "TextureBuilder.h"
#include "Model_3DS.h"
#include "GLTexture.h"
#include <glut.h>
#include <cmath>
#include <iostream>
#include <vector>

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

float weaponBobAngle = 0.0f;
float weaponBobSpeed = 5.0f;
float weaponBobAmount = 0.05f;
bool isMoving = false;

float gameTime = 0.0f;       // Game time in seconds
float timeSpeed = 0.1f;      // Speed of time progression (adjust for faster/slower transitions)
float sunAngle = 0.0f;       // Angle of the sun (degrees)
float maxIntensity = 1.0f;   // Maximum light intensity
float minIntensity = 0.1f;   // Minimum light intensity
float lightIntensity = maxIntensity;  // Current light intensity
GLfloat lightPosition[] = { 0.0f, 10.0f, 0.0f, 1.0f }; // Initial light position


// Models
Model_3DS model_house;
Model_3DS model_tree;
Model_3DS model_player;
Model_3DS model_rifle;  // New rifle model

GLTexture tex_sky;  // Declare the sky texture globally


// Tree positions
struct TreePosition {
    float x, z;
    float scale;
    float rotation;
};

std::vector<TreePosition> trees = {
    {10.0f, -10.0f, 0.5f, 0.0f},    // Original tree
    {-15.0f, -20.0f, 0.6f, 45.0f},
    {25.0f, -15.0f, 0.4f, 90.0f},
    {-8.0f, -30.0f, 0.5f, 180.0f},
    {30.0f, -25.0f, 0.55f, 270.0f},
    {-20.0f, 15.0f, 0.45f, 120.0f},
    {15.0f, 25.0f, 0.5f, 200.0f},
    {-25.0f, -5.0f, 0.6f, 150.0f},
    {35.0f, 10.0f, 0.4f, 80.0f},
    {-30.0f, 25.0f, 0.5f, 220.0f},
    {20.0f, -30.0f, 0.55f, 30.0f},
    {-35.0f, -15.0f, 0.45f, 160.0f},
    {40.0f, 20.0f, 0.5f, 290.0f},
    {-40.0f, 10.0f, 0.6f, 100.0f},
    {25.0f, 35.0f, 0.4f, 240.0f},
    {-15.0f, 40.0f, 0.5f, 70.0f}
};

// Textures
GLTexture tex_ground;

const float M_PI = 3.14159265358979323846;

//=======================================================================
// Update Camera Direction
//=======================================================================
void UpdateCamera() {
    float lookX = cos(playerYaw * (M_PI / 180.0f));
    float lookZ = sin(playerYaw * (M_PI / 180.0f));

    gluLookAt(
        playerX, playerY, playerZ,          // Camera position
        playerX + lookX, playerY, playerZ + lookZ, // Look-at point
        0.0f, 1.0f, 0.0f                   // Up vector
    );
}


//=======================================================================
// Mouse Movement Callback
//=======================================================================
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
//=======================================================================
// Initialize Function
//=======================================================================
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


//=======================================================================
// Display Function
//=======================================================================
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
    glTranslatef(0.3f, -0.45f , -0.5f);  // Adjust these values to position the weapon

    // Apply weapon rotation based on camera look direction


    // Rotate and scale the weapon model
    glRotatef(180.0f, 0.0f, 1.0f, 0.0f);  // Align weapon model
    glRotatef(270.0f, 0.0f, 1.0f, 0.0f);   // Point weapon forward
    glScalef(1.2f, 1.2f, 1.2f);
    // Draw the weapon
    model_rifle.Draw();

    // Restore matrices
    glMatrixMode(GL_PROJECTION);
    glPopMatrix();
    glMatrixMode(GL_MODELVIEW);
    glPopMatrix();
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

void RenderSky() {
    glDisable(GL_LIGHTING);  // Disable lighting for the sky
    glDisable(GL_DEPTH_TEST); // Disable depth testing to render the sky behind everything
    glEnable(GL_TEXTURE_2D);
    glBindTexture(GL_TEXTURE_2D, tex_sky.texture[0]);

    // Draw a large sphere for the sky
    glPushMatrix();
    glTranslatef(playerX, playerY - 1.8f, playerZ); // Center the sky around the player
    glColor3f(1.0f, 1.0f, 1.0f); // Use default color

    GLUquadric* sky = gluNewQuadric();
    gluQuadricTexture(sky, GL_TRUE);
    gluSphere(sky, 100.0f, 30, 30);  // Large sphere with the sky texture
    gluDeleteQuadric(sky);

    glPopMatrix();

    glEnable(GL_LIGHTING);  // Re-enable lighting
    glEnable(GL_DEPTH_TEST); // Re-enable depth testing
}

void UpdateLighting() {
    // Update game time and sun angle
    gameTime += timeSpeed;
    if (gameTime >= 360.0f) gameTime -= 360.0f; // Reset after full day cycle
    sunAngle = gameTime; // Sun angle progresses with time

    // Update light position (rotate around player)
    lightPosition[0] = 50.0f * cos(sunAngle * M_PI / 180.0f); // X-axis
    lightPosition[1] = 50.0f * sin(sunAngle * M_PI / 180.0f); // Y-axis (height)
    lightPosition[2] = 50.0f * cos(sunAngle * M_PI / 180.0f); // Z-axis

    // Update light intensity based on time of day (sine wave for smooth transition)
    lightIntensity = minIntensity + (maxIntensity - minIntensity) * 0.5f * (1.0f + sin(sunAngle * M_PI / 180.0f));

    // Set light properties
    GLfloat diffuse[] = { lightIntensity, lightIntensity, lightIntensity, 1.0f };
    GLfloat ambient[] = { lightIntensity * 0.3f, lightIntensity * 0.3f, lightIntensity * 0.3f, 1.0f };

    glLightfv(GL_LIGHT0, GL_POSITION, lightPosition);
    glLightfv(GL_LIGHT0, GL_DIFFUSE, diffuse);
    glLightfv(GL_LIGHT0, GL_AMBIENT, ambient);
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
    RenderTrees();

    // Draw House
    glPushMatrix();
    glTranslatef(0, 0, 20);
    glScalef(0.7, 0.7, 0.7);
    model_house.Draw();
    glPopMatrix();

    // Render player last for proper transparency
    RenderPlayer();
    RenderFirstPersonWeapon();

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
void myIdle() {
    UpdateLighting();
    glutPostRedisplay();
}

//=======================================================================
// Keyboard Input Handler
//=======================================================================
//=======================================================================
// Keyboard Input Handler (with Boundaries)
//=======================================================================
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
//=======================================================================
// Load Assets Function
//=======================================================================
void LoadAssets() {
    model_house.Load("Models/house/house.3DS");
    model_tree.Load("Models/tree/Tree1.3ds");
    model_player.Load("Models/player/Soldier.3ds"); // Load the player model
    model_rifle.Load("Models/gun/rifle.3ds");  // Load the rifle model
    tex_ground.Load("Textures/ground.bmp");
    tex_sky.Load("Textures/blu-sky-3.bmp"); // Load the sky texture

}

//=======================================================================
// Special Keys Callback (Arrow Keys)
//=======================================================================
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

//=======================================================================
// Main Function
//=======================================================================
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

    // Center the mouse cursor
    glutWarpPointer(WIDTH / 2, HEIGHT / 2);
    glutIdleFunc(myIdle);
    glutMainLoop();
}
