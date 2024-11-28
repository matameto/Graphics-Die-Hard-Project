#include "TextureBuilder.h"
#include "Model_3DS.h"
#include "GLTexture.h"
#include "OBJModel.h"
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
Model_3DS model_barrel;
OBJModel model_target1;;
Model_3DS model_bullet;

GLTexture tex_sky;  // Declare the sky texture globally

struct Target1 {
    float x, z;       // X and Z position on the ground
    float scale;      // Scaling factor
    float rotationX;  // Rotation about the X-axis
    bool isRotating;  // Whether the target rotates
    bool isHit;       // Whether the target is hit 
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

// Barrels - Level 1
struct BarrelPosition {
    float x, z;     // X and Z position on the ground
    float scale;    // Scaling factor
    float rotation; // Rotation angle
};

// Define a vector with barrel positions
std::vector<BarrelPosition> barrels = {
    {15.0f, -15.0f, 0.001f, 0.0f},
    {-25.0f, 10.0f, 0.001f, 45.0f},
    {40.0f, -50.0f, 0.001f, 90.0f},
    {70.0f, -90.0f, 0.001f, 180.0f},
    {-60.0f, 50.0f, 0.001f, 270.0f},
    {100.0f, 15.0f, 0.001f, 30.0f}
};


// Tree positions - Level 1
struct TreePosition {
    float x, z;
    float scale;
    float rotation;
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


struct Bullet {
	float x, y, z;  // Position
	float directionX, directionY, directionZ; // where the bullet is going
    float  speed; // Velocity
};

std::vector<Bullet> bullets;


// Textures
GLTexture tex_ground;

const float M_PI = 3.14159265358979323846;

//=======================================================================
// shoot function creates a new bullet every time it is called
//=======================================================================

void Shoot() {
    Bullet bullet;
	// offset the bullet so it comes out of the gun
    float pitchRadians = playerPitch * (M_PI / 180.0f);
    float yawRadians = playerYaw * (M_PI / 180.0f);
	

    bullet.x = playerX + cos(pitchRadians) * cos(yawRadians); ;
    bullet.y = playerY + sin(pitchRadians);
    bullet.z = playerZ + cos(pitchRadians) * sin(yawRadians);
    

    bullet.directionX = cos(pitchRadians) * cos(yawRadians); // Horizontal component with pitch
    bullet.directionY = sin(pitchRadians);                  // Vertical component with pitch
    bullet.directionZ = cos(pitchRadians) * sin(yawRadians); // Horizontal component with yaw
    bullet.speed = 0.5f; // Adjust speed as needed we can increase it a bit

    bullets.push_back(bullet);
}

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
// shooting call back 
void myMouse(int button, int state, int x, int y) {
	if (button == GLUT_LEFT_BUTTON && state == GLUT_DOWN) {
		Shoot();
	}
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

        // Apply rotations to align the bullet with its direction
        
        glRotatef(pitchDegrees, 1.0f, 0.0f, 0.0f); // Pitch rotation (around X-axis)
		glRotatef(-yawDegrees, 0.0f, 1.0f, 0.0f); // Yaw rotation (around Y-axis) keep it negative or the bullet tip will point towards the player
       
        glScalef(0.001f, 0.001f, 0.001f); // use diffrent scale for debugging


        // Draw the bullet model
        model_bullet.Draw();

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
    RenderBarrels();
    RenderTargets(); // Render targets

    // Draw House
    glPushMatrix();
    glTranslatef(0, 0, 20);
    glScalef(0.7, 0.7, 0.7);
    model_house.Draw();
    glPopMatrix();

    // Render player last for proper transparency
    RenderPlayer();
    RenderFirstPersonWeapon();
    RenderBullets();

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
    model_rifle.Load("Models/rifle/rifle.3ds");  // Load the rifle model
    tex_ground.Load("Textures/ground.bmp");
    tex_sky.Load("Textures/blu-sky-3.bmp"); // Load the sky texture
    model_barrel.Load("Models/barrel/barrel.3ds");
	
	model_bullet.Load("Models/bullet/Bullet.3ds");
    if (!model_target1.LoadFromFile("Models/target2/target2.obj")) {
        std::cerr << "Failed to load model!" << std::endl;
        // Handle error
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

        // Check for collision with each target
        for (auto& target : targets) {
            if (!target.isHit && CheckCollision(*bulletIt, target)) {
                target.isHit = true; // Mark the target as hit
                collisionDetected = true;
                std::cout << "Hit target at (" << target.x << ", " << target.z << ")\n";
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

/// 
///  ========================
/// 
void myIdle() {
    UpdateLighting();
    UpdateTargets(); // Ensure targets update during idle time
	UpdateBullets(); // Update bullet positions
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
	glutMouseFunc(myMouse);  // Enable mouse button callback
    // Center the mouse cursor
    glutWarpPointer(WIDTH / 2, HEIGHT / 2);
    glutIdleFunc(myIdle);
    glutMainLoop();
}
