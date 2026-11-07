#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <iostream>
#include <cmath>
#include <vector>
#include <string>
#include <sstream>
#include <iomanip>
using namespace std;

#define PI 3.14159265358979323846
#define G 0.66743

struct Planet {
    string name;
    float x, y;
    float z = 0;
    float vx, vy, vz;
    float r;
    float m;          // Code mass used in physics
    string realMass;
    string realSpeed;
    string realDist;
    float cr, cg, cb; // Color components
    vector<glm::vec3> trail; 

    // orbit
    float accumulatedAngle = 0.0f; //total radians swept
    float prevAngle = 0.0f;
    bool angleInitialized = false;
    int completedOrbits = 0;
    double currentOrbitStartTime = 0.0;
    double lastOrbitDuration = 0.0;
};

struct Engine {
    GLFWwindow* window;
    int WIDTH = 1920, HEIGHT = 1080;

    //camera
    float yaw = 0.0f; // left right
    float pitch = 0.5f; // up down
    float dist = 725.0f; // front and back

    //screen projection conversion
    glm::mat4 projection; 
    glm::mat4 view;

    Engine() {
        if (!glfwInit()) {
            cerr << "glfw didn't start bruh" << endl;
            exit(EXIT_FAILURE);
        }

        window = glfwCreateWindow(WIDTH, HEIGHT, "Solar System Simulator - Vector HUD", nullptr, nullptr);
        if (!window) {
            cerr << "no window for you nerd" << endl;
            glfwTerminate();
            exit(EXIT_FAILURE);
        }

        glfwMakeContextCurrent(window);
        glewInit();
        glfwSwapInterval(1); // Wait for vsync

        int fbWidth, fbHeight;
        glfwGetFramebufferSize(window, &fbWidth, &fbHeight);
        glViewport(0, 0, fbWidth, fbHeight);
    }

    // run once per frame
    void run(const glm::vec3& target) {
        //wipe colours and depth
        glClearColor(0.05f, 0.05f, 0.08f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        glEnable(GL_DEPTH_TEST);

        //key move camera
        if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS) yaw -= 0.02;
        if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS) yaw += 0.02;
        if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS) pitch += 0.02;
        if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS) pitch -= 0.02;
        if (glfwGetKey(window, GLFW_KEY_E) == GLFW_PRESS) dist -= 5.0f;
        if (glfwGetKey(window, GLFW_KEY_Q) == GLFW_PRESS) dist += 5.0f;
        pitch = glm::clamp(pitch, -1.5f, 1.5f);

        //perspective
        projection = glm::perspective(glm::radians(45.0f), (float)WIDTH/HEIGHT, 1.0f, 5000.0f);
        glMatrixMode(GL_PROJECTION);
        glLoadMatrixf(glm::value_ptr(projection));

        glm::vec3 eye(target.x + dist*cos(pitch) *sin(yaw), 
                      target.y + dist*sin(pitch), 
                      target.z + dist*cos(pitch)*cos(yaw));
        view = glm::lookAt(eye, target, glm::vec3(0, 1, 0));
        
        glMatrixMode(GL_MODELVIEW);
        glLoadMatrixf(glm::value_ptr(view));
    }
};

#pragma region Objects
void drawCircle(float cx, float cy, float r) {
    glBegin(GL_TRIANGLE_FAN);
    glVertex2f(cx, cy);
    int segments = 100;
    for (int i = 0; i <= segments; i++) {
        float angle = 2.0f * PI * i / segments;
        float x = cx + r * cos(angle);
        float y = cy + r * sin(angle);
        glVertex2f(x, y);
    }
    glEnd();
}

// one point on a sphere
void spherePoint(const Planet& p, float theta, float phi, glm::vec3 toSun) { //toSun is the arrow from planet -> the Sun
    float x = sin(theta) * cos(phi);
    float y = cos(theta);
    float z = sin(theta) * sin(phi);

    // light handler
    float light = 1.0f; //Sun light
    if (glm::length(toSun) > 0) {
        float facing = glm::dot(glm::vec3(x,y,z), glm::normalize(toSun));
        light = 0.15f + 0.85f*fmax(facing, 0.0f);
    }

    glColor3f(p.cr * light, p.cg * light, p.cb * light);
    glVertex3f(p.x + p.r * x, p.y + p.r * y, p.z + p.r * z);
}

void drawSphere(const Planet& p, const Planet& sun) {
    int stacks = 20, slices = 30;
    glm::vec3 toSun(sun.x - p.x, sun.y - p.y, sun.z - p.z);

    for (int i = 0; i < stacks; i++) {
        float theta1 = PI * i /stacks; //ring
        float theta2 = PI * (i+1) / stacks; //ring below

        glBegin(GL_TRIANGLE_STRIP);
        for (int j=0; j<=slices; j++) {
            float phi = 2.0f * PI * j/slices;
            spherePoint(p, theta1, phi, toSun);
            spherePoint(p, theta2, phi, toSun);
        }
        glEnd();
    }
}

void drawBlackHole(const Planet& bh) {
    int segments = 80;

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE);

    // accretion disk
    float rInner = bh.r * 1.15f;
    float rOuter = bh.r * 5.0f;

    glBegin(GL_QUAD_STRIP);
    for (int i = 0; i<=segments; i++) {
        float angle = 2.0f * PI * i / segments;
        float cosA = cos(angle);
        float sinA = sin(angle);

        // inner photon ring (white-orange);
        glColor4f(1.0f, 0.85f, 0.6f, 0.95f);
        glVertex3f(bh.x + rInner * cosA, bh.y, bh.z + rOuter * sinA);
    }
    glEnd();

    // light bending
    glBegin(GL_QUAD_STRIP);
    for (int i = 0; i<= segments; i++) {
        float angle = 2.0f * PI * i / segments;
        float cosA = cos(angle);
        float sinA = sin(angle);

        float arcRadius1 = bh.r * 1.25f;
        float arcRadius2 = bh.r * 1.8f;
        
        glColor4f(1.0f, 0.95f, 0.8f, 0.85f);
        glVertex3f(bh.x + arcRadius1 * cosA, bh.y + bh.r * 0.9f, bh.z + arcRadius1 * sinA);
        glColor4f(1.0f, 0.45f, 0.1f, 0.0f);
        glVertex3f(bh.x + arcRadius2 * cosA, bh.y + bh.r * 2.2f, bh.z + arcRadius2 * sinA);
    }

    glEnd();
    glDisable(GL_BLEND);

    // central event horizon core
    Planet blackCore = bh;
    blackCore.cr = 0.0f; blackCore.cg = 0.0f; blackCore.cb = 0.0f;
    drawSphere(blackCore, bh);
}
// Properly scaled vector character drawer for 1600x900 world coordinates
void drawVectorChar(char c, float x, float y, float size) {
    glPushMatrix();
    glTranslatef(x, y, 0);
    glScalef(size * 0.7f, size * 1.0f, 1.0f);
    glLineWidth(1.5f);
    glBegin(GL_LINES);

    switch (c) {
        // Numbers
        case '0': glVertex2f(0,0);glVertex2f(10,0); glVertex2f(10,0);glVertex2f(10,14); glVertex2f(10,14);glVertex2f(0,14); glVertex2f(0,14);glVertex2f(0,0); break;
        case '1': glVertex2f(5,0);glVertex2f(5,14); glVertex2f(2,11);glVertex2f(5,14); break;
        case '2': glVertex2f(0,14);glVertex2f(10,14); glVertex2f(10,14);glVertex2f(10,7); glVertex2f(10,7);glVertex2f(0,7); glVertex2f(0,7);glVertex2f(0,0); glVertex2f(0,0);glVertex2f(10,0); break;
        case '3': glVertex2f(0,14);glVertex2f(10,14); glVertex2f(10,14);glVertex2f(10,0); glVertex2f(0,7);glVertex2f(10,7); glVertex2f(0,0);glVertex2f(10,0); break;
        case '4': glVertex2f(0,14);glVertex2f(0,7); glVertex2f(0,7);glVertex2f(10,7); glVertex2f(10,14);glVertex2f(10,0); break;
        case '5': glVertex2f(10,14);glVertex2f(0,14); glVertex2f(0,14);glVertex2f(0,7); glVertex2f(0,7);glVertex2f(10,7); glVertex2f(10,7);glVertex2f(10,0); glVertex2f(10,0);glVertex2f(0,0); break;
        case '6': glVertex2f(10,14);glVertex2f(0,14); glVertex2f(0,14);glVertex2f(0,0); glVertex2f(0,0);glVertex2f(10,0); glVertex2f(10,0);glVertex2f(10,7); glVertex2f(10,7);glVertex2f(0,7); break;
        case '7': glVertex2f(0,14);glVertex2f(10,14); glVertex2f(10,14);glVertex2f(5,0); break;
        case '8': glVertex2f(0,0);glVertex2f(10,0); glVertex2f(10,0);glVertex2f(10,14); glVertex2f(10,14);glVertex2f(0,14); glVertex2f(0,14);glVertex2f(0,0); glVertex2f(0,7);glVertex2f(10,7); break;
        case '9': glVertex2f(0,7);glVertex2f(10,7); glVertex2f(10,7);glVertex2f(10,14); glVertex2f(10,14);glVertex2f(0,14); glVertex2f(0,14);glVertex2f(0,7); glVertex2f(10,14);glVertex2f(10,0); break;

        // Letters
        case 'S': case 's': glVertex2f(10,14);glVertex2f(0,14); glVertex2f(0,14);glVertex2f(0,7); glVertex2f(0,7);glVertex2f(10,7); glVertex2f(10,7);glVertex2f(10,0); glVertex2f(10,0);glVertex2f(0,0); break;
        case 'u': case 'U': glVertex2f(0,14);glVertex2f(0,0); glVertex2f(0,0);glVertex2f(10,0); glVertex2f(10,0);glVertex2f(10,14); break;
        case 'n': case 'N': glVertex2f(0,0);glVertex2f(0,14); glVertex2f(0,14);glVertex2f(10,0); glVertex2f(10,0);glVertex2f(10,14); break;
        case 'M': case 'm': glVertex2f(0,0);glVertex2f(0,14); glVertex2f(0,14);glVertex2f(5,7); glVertex2f(5,7);glVertex2f(10,14); glVertex2f(10,14);glVertex2f(10,0); break;
        case 'e': case 'E': glVertex2f(10,14);glVertex2f(0,14); glVertex2f(0,14);glVertex2f(0,0); glVertex2f(0,0);glVertex2f(10,0); glVertex2f(0,7);glVertex2f(8,7); break;
        case 'r': case 'R': glVertex2f(0,0);glVertex2f(0,14); glVertex2f(0,14);glVertex2f(10,14); glVertex2f(10,14);glVertex2f(10,7); glVertex2f(10,7);glVertex2f(0,7); glVertex2f(5,7);glVertex2f(10,0); break;
        case 'c': case 'C': glVertex2f(10,14);glVertex2f(0,14); glVertex2f(0,14);glVertex2f(0,0); glVertex2f(0,0);glVertex2f(10,0); break;
        case 'y': case 'Y': glVertex2f(0,14);glVertex2f(5,7); glVertex2f(10,14);glVertex2f(5,7); glVertex2f(5,7);glVertex2f(5,0); break;
        case 'V': case 'v': glVertex2f(0,14);glVertex2f(5,0); glVertex2f(5,0);glVertex2f(10,14); break;
        case 'a': case 'A': glVertex2f(0,0);glVertex2f(0,14); glVertex2f(0,14);glVertex2f(10,14); glVertex2f(10,14);glVertex2f(10,0); glVertex2f(0,7);glVertex2f(10,7); break;
        case 't': case 'T': glVertex2f(0,14);glVertex2f(10,14); glVertex2f(5,14);glVertex2f(5,0); break;
        case 'h': case 'H': glVertex2f(0,0);glVertex2f(0,14); glVertex2f(10,0);glVertex2f(10,14); glVertex2f(0,7);glVertex2f(10,7); break;
        case 'J': case 'j': glVertex2f(0,7);glVertex2f(5,0); glVertex2f(5,0);glVertex2f(10,0); glVertex2f(10,0);glVertex2f(10,14); break;
        case 'p': case 'P': glVertex2f(0,0);glVertex2f(0,14); glVertex2f(0,14);glVertex2f(10,14); glVertex2f(10,14);glVertex2f(10,7); glVertex2f(10,7);glVertex2f(0,7); break;
        case 'd': case 'D': glVertex2f(0,0);glVertex2f(0,14); glVertex2f(0,14);glVertex2f(8,11); glVertex2f(8,11);glVertex2f(8,3); glVertex2f(8,3);glVertex2f(0,0); break;
        case 'L': case 'l': glVertex2f(0,14);glVertex2f(0,0); glVertex2f(0,0);glVertex2f(10,0); break;
        case 'g': case 'G': glVertex2f(10,14);glVertex2f(0,14); glVertex2f(0,14);glVertex2f(0,0); glVertex2f(0,0);glVertex2f(10,0); glVertex2f(10,0);glVertex2f(10,7); glVertex2f(5,7);glVertex2f(10,7); break;
        case 'b': case 'B': glVertex2f(0,0);glVertex2f(0,14); glVertex2f(0,14);glVertex2f(10,11); glVertex2f(10,11);glVertex2f(0,7); glVertex2f(0,7);glVertex2f(10,3); glVertex2f(10,3);glVertex2f(0,0); break;
        case 'o': case 'O': glVertex2f(0,0);glVertex2f(0,14); glVertex2f(0,14);glVertex2f(10,14); glVertex2f(10,14);glVertex2f(10,0); glVertex2f(10,0);glVertex2f(0,0); break;
        case 'k': case 'K': glVertex2f(0,0);glVertex2f(0,14); glVertex2f(0,7);glVertex2f(10,14); glVertex2f(0,7);glVertex2f(10,0); break;
        case 'x': case 'X': glVertex2f(0,0);glVertex2f(10,14); glVertex2f(0,14);glVertex2f(10,0); break;
        case 'i': case 'I': glVertex2f(5,0);glVertex2f(5,14); break;
        case 'f': case 'F': glVertex2f(0,0);glVertex2f(0,14); glVertex2f(0,14);glVertex2f(10,14); glVertex2f(0,7);glVertex2f(8,7); break;
        case 'z': case 'Z': glVertex2f(0,14);glVertex2f(10,14); glVertex2f(10,14);glVertex2f(0,0); glVertex2f(0,0);glVertex2f(10,0); break;

        // Symbols
        case '.': glVertex2f(3,2);glVertex2f(6,2); break;
        case '-': glVertex2f(0,7);glVertex2f(10,7); break;
        case '+': glVertex2f(5,3);glVertex2f(5,11); glVertex2f(1,7);glVertex2f(9,7); break;
        case '/': glVertex2f(0,0);glVertex2f(10,14); break;
        case '(': glVertex2f(8,0);glVertex2f(2,7); glVertex2f(2,7);glVertex2f(8,14); break;
        case ')': glVertex2f(2,0);glVertex2f(8,7); glVertex2f(8,7);glVertex2f(2,14); break;
        case ':': glVertex2f(5,10);glVertex2f(5,11); glVertex2f(5,3);glVertex2f(5,4); break;
        case ' ': default: break;
    }

    glEnd();
    glPopMatrix();
}

void drawText(string text, float x, float y, float size, float r, float g, float b) {
    glColor3f(r, g, b);
    float cursorX = x;
    for (char c : text) {
        drawVectorChar(c, cursorX, y, size);
        cursorX += 8.5f * size;
    }
}

void drawGrid(){
    glColor3b(0.3f, 0.3f, 0.36f);
    glBegin(GL_LINES);
    for (int i = -400; i<=400; i+=40) {
        glVertex3f(i, -250, -400); //going back 
        glVertex3f(i, -250, 400); 
        glVertex3f(-400, -250, i); //going across
        glVertex3f(400, -250, i); 
    }
    glEnd();
}

void drawRing(const Planet& p, const Planet& sun, float innerR, float outerR, float r, float g, float b) {
    int segments = 60;

    glm::vec3 toSun(sun.x - p.x, sun.y - p.y, sun.z - p.z);
    glm::vec3 sunDir = glm::length(toSun) > 0 ? glm::normalize(toSun) : glm::vec3(1,0,0);

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glBegin(GL_QUAD_STRIP);

    for (int i = 0; i <= segments; i++) {
        float angle = 2.0f * PI * i /segments;
        float cosA = cos(angle);
        float sinA = sin(angle);

        // lighting from the sun
        glm::vec3 dir(cosA, 0.0f, sinA);
        float facing = glm::dot(dir, sunDir);
        float light = 0.15f + 0.85f * fmax(facing, 0.0f);

        // innner ring vertex
        glColor4f(r * light, g * light, b * light, 0.2f);
        glVertex3f(p.x + innerR * cosA, p.y, p.z + innerR * sinA);
        
        // outer ring vertex
        glColor4f(r * light, g * light, b * light, 0.6f);
        glVertex3f(p.x + outerR * cosA, p.y, p.z + outerR * sinA);
    }
    glEnd();
    glDisable(GL_BLEND);
}
#pragma endregion Objects

#pragma region Additional Properities
void drawAxes() {
    glLineWidth(2.5f);
    glBegin(GL_LINES);
    // X Axis - Red
    glColor3f(1.0f, 0.2f, 0.2f);
    glVertex3f(-300.0f, 0.0f, 0.0f);
    glVertex3f(300.0f, 0.0f, 0.0f);

    // Y Axis - Green
    glColor3f(0.2f, 1.0f, 0.2f);
    glVertex3f(0.0f, -300.0f, 0.0f);
    glVertex3f(0.0f, 300.0f, 0.0f);

    // Z Axis - Blue
    glColor3f(0.2f, 0.5f, 1.0f);
    glVertex3f(0.0f, 0.0f, -300.0f);
    glVertex3f(0.0f, 0.0f, 300.0f);
    glEnd();
    glLineWidth(1.0f);
}

void drawPlanetAxes(const Planet& p){
    float len = p.r * 1.8f;
    glLineWidth(2.0f);
    glBegin(GL_LINES);

    glColor3f(1.0f, 0.2f, 0.2f);
    glVertex3f(p.x - len, p.y, p.z); glVertex3f(p.x + len, p.y, p.z);
    // Y Axis - Green
    glColor3f(0.2f, 1.0f, 0.2f);
    glVertex3f(p.x, p.y - len, p.z); glVertex3f(p.x, p.y + len, p.z);
    // Z Axis - Blue
    glColor3f(0.2f, 0.5f, 1.0f);
    glVertex3f(p.x, p.y, p.z - len); glVertex3f(p.x, p.y, p.z + len);
    glEnd();
    glLineWidth(1.0f);
}

void drawBoundingBubble(float radius, glm::vec3 center) {
    int stacks = 64, slices = 64;
    glColor4f(0.3f, 0.5f, 0.8f, 0.15f); // Subtle cyan grid outline
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    for (int i = 0; i <= stacks; i++) {
        float theta = PI * i / stacks;
        glBegin(GL_LINE_LOOP);
        for (int j = 0; j < slices; j++) {
            float phi = 2.0f * PI * j / slices;
            float x = center.x + radius * sin(theta) * cos(phi);
            float y = center.y + radius * cos(theta);
            float z = center.z + radius * sin(theta) * sin(phi);
            glVertex3f(x, y, z);
        }
        glEnd();
    }
    glDisable(GL_BLEND);
}

// Calculate relativistic spatial vertex displacement based on General Relativity weak-field approximation
glm::vec3 warpSpaceVertex(glm::vec3 v, const vector<Planet>& planets) {
    glm::vec3 displacement(0.0f, 0.0f, 0.0f);
    float warpingStrength = 45000.0f; // Tune this to increase/decrease overall warping intensity

    for (const auto& p : planets) {
        glm::vec3 pPos(p.x, p.y, p.z);
        glm::vec3 rVec = v - pPos;
        float distSq = glm::dot(rVec, rVec);
        float softening = 3000.0f; // Prevents division spikes near the core

        // Inverse-square/cubed gravitational well deformation formula
        float massWeight = (p.name == "Sun") ? 1.0f : (p.m / 100.0f);
        float factor = (warpingStrength * massWeight) / pow(distSq + softening, 1.3f);
        displacement -= rVec * factor;
    }

    return v + displacement;
}

// Render a 3D wireframe space lattice grid box with smooth gravitational warping
void drawSpaceBox(glm::vec3 center, float halfSize, int gridDivisions, float intensity, const vector<Planet>& planets) {
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glColor4f(0.35f, 0.65f, 1.0f, 0.3f * intensity);
    glLineWidth(1.3f * intensity);

    float step = (2.0f * halfSize) / gridDivisions;
    int lineSubdivs = 14; // Subdivide each span so lines curve smoothly instead of staying straight

    for (int i = 0; i <= gridDivisions; i++) {
        float u = -halfSize + i * step;
        for (int j = 0; j <= gridDivisions; j++) {
            float vCoord = -halfSize + j * step;

            // 1. X-aligned warped line strips
            glBegin(GL_LINE_STRIP);
            for (int s = 0; s <= lineSubdivs; s++) {
                float t = (float)s / lineSubdivs;
                float x = -halfSize + t * (2.0f * halfSize);
                glm::vec3 pt = center + glm::vec3(x, u, vCoord);
                glm::vec3 warped = warpSpaceVertex(pt, planets);
                glVertex3f(warped.x, warped.y, warped.z);
            }
            glEnd();

            // 2. Y-aligned warped line strips
            glBegin(GL_LINE_STRIP);
            for (int s = 0; s <= lineSubdivs; s++) {
                float t = (float)s / lineSubdivs;
                float y = -halfSize + t * (2.0f * halfSize);
                glm::vec3 pt = center + glm::vec3(u, y, vCoord);
                glm::vec3 warped = warpSpaceVertex(pt, planets);
                glVertex3f(warped.x, warped.y, warped.z);
            }
            glEnd();

            // 3. Z-aligned warped line strips
            glBegin(GL_LINE_STRIP);
            for (int s = 0; s <= lineSubdivs; s++) {
                float t = (float)s / lineSubdivs;
                float z = -halfSize + t * (2.0f * halfSize);
                glm::vec3 pt = center + glm::vec3(u, vCoord, z);
                glm::vec3 warped = warpSpaceVertex(pt, planets);
                glVertex3f(warped.x, warped.y, warped.z);
            }
            glEnd();
        }
    }
    glLineWidth(1.0f);
    glDisable(GL_BLEND);
}

void drawPlanetTrails(const vector<Planet>& planets) {
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glLineWidth(1.8f);

    for (const auto& p : planets) {
        if (p.trail.size() < 2) continue;

        glBegin(GL_LINE_STRIP);
        for (size_t i = 0; i < p.trail.size(); i++) {
            float alpha = (float)i / p.trail.size();
            glColor4f(p.cr, p.cg, p.cb, alpha*0.7f);
            glVertex3d(p.trail[i].x, p.trail[i].y, p.trail[i].z);
        }
        glEnd();
    } 
    glLineWidth(1.0f);
    glDisable(GL_BLEND);
}
#pragma endregion


int main() {
    Engine engine;

    // std::vector<Planet> planets = {
    //     //  Name        x     y     z     vx     vy     vz    r     m         realMass                     realSpeed      realDist          cr    cg    cb
    //     { "Sun",       0,     0,    0,   0.0,   0.0,   0.0,  45,   2500.0f,  "Mass: 333k Earth",   "Vel: 220 km/s",   "Dist: Center",     1.00, 0.78, 0.25 },
    //     { "Mercury", 100,     0,   40,   0.0,   3.5,  -1.8,   6,      0.1f,  "Mass: 0.055 Earth",  "Vel: 47.4 km/s",  "Dist: 58M km",     0.69, 0.69, 0.69 },
    //     { "Venus",   150,     0,  -70,   0.0,   2.7,   1.6,  10,      0.8f,  "Mass: 0.815 Earth",  "Vel: 35.0 km/s",  "Dist: 108M km",    0.93, 0.79, 0.51 },
    //     { "Earth",   210,     0,   90,   0.0,   2.2,  -1.4,  12,      1.0f,  "Mass: 1.0 Earth",    "Vel: 29.8 km/s",  "Dist: 150M km",    0.35, 0.62, 1.00 },
    //     { "Moon",    218,     0,   93,   0.0,   2.5,  -1.3,   3,      0.01f, "Mass: 0.012 Earth",  "Vel: 1.0 km/s",   "Dist: 384k km",    0.70, 0.70, 0.70 },
    //     { "Mars",    290,     0, -140,   0.0,   1.8,   1.3,   9,      0.11f, "Mass: 0.107 Earth",  "Vel: 24.1 km/s",  "Dist: 228M km",    0.90, 0.38, 0.24 },
    //     { "Jupiter", 420,     0,  180,   0.0,   1.5,  -0.9,  22,     30.0f,  "Mass: 317.8 Earth",  "Vel: 13.1 km/s",  "Dist: 778M km",    0.85, 0.65, 0.45 },
    //     { "Saturn",  540,     0, -220,   0.0,   1.3,   0.7,  17,      9.0f,  "Mass: 95.2 Earth",   "Vel: 9.7 km/s",   "Dist: 1.4B km",    0.91, 0.82, 0.56 },
    //     { "Uranus",  650,     0,  260,   0.0,   1.1,  -0.6,  14,      1.4f,  "Mass: 14.5 Earth",   "Vel: 6.8 km/s",   "Dist: 2.8B km",    0.39, 0.75, 0.87 },
    //     { "Neptune", 740,     0, -300,   0.0,   1.0,   0.5,  14,      1.5f,  "Mass: 17.1 Earth",   "Vel: 5.4 km/s",   "Dist: 4.5B km",    0.25, 0.41, 0.88 },
    // };
    
    std::vector<Planet> planets = {
        //  Name               x        y     z     vx     vy       vz    r     m         realMass                     realSpeed      realDist          cr    cg    cb
        { "Sun",              0.0f,     0,    0,   0.0,   11.55,   0.0,  45,   2500.0f,  "Mass: 333k Earth",   "Vel: 220 km/s",   "Dist: Center",     1.00, 0.78, 0.25 },
        { "Mercury",        100.0f,     0,   12,   0.0,   15.63,  -0.3,   6,      0.14f, "Mass: 0.055 Earth",  "Vel: 47.4 km/s",  "Dist: 58M km",     0.69, 0.69, 0.69 },
        { "Venus",          150.0f,     0,   10,   0.0,   14.88,   0.2,  10,      0.81f, "Mass: 0.815 Earth",  "Vel: 35.0 km/s",  "Dist: 108M km",    0.93, 0.79, 0.51 },
        { "Earth",          210.0f,     0,    0,   0.0,   14.37,   0.0,  12,      1.0f,  "Mass: 1.0 Earth",    "Vel: 29.8 km/s",  "Dist: 150M km",    0.35, 0.62, 1.00 },
        { "Moon",           216.5f,     0,    0,   0.0,   14.70,   0.1,   3,      0.012f,"Mass: 0.012 Earth",  "Vel: 1.0 km/s",   "Dist: 384k km",    0.70, 0.70, 0.70 },
        { "Mars",           290.0f,     0,  -15,   0.0,   13.95,   0.15,  9,      0.11f, "Mass: 0.107 Earth",  "Vel: 24.1 km/s",  "Dist: 228M km",    0.90, 0.38, 0.24 },
        { "Jupiter",        420.0f,     0,   20,   0.0,   13.54,  -0.1,  22,    317.8f, "Mass: 317.8 Earth",  "Vel: 13.1 km/s",  "Dist: 778M km",    0.85, 0.65, 0.45 },
        { "Saturn",         540.0f,     0,  -25,   0.0,   13.31,   0.12, 17,     95.2f, "Mass: 95.2 Earth",   "Vel: 9.7 km/s",   "Dist: 1.4B km",    0.91, 0.82, 0.56 },
        { "Uranus",         650.0f,     0,   15,   0.0,   13.15,  -0.08, 14,     14.5f, "Mass: 14.5 Earth",   "Vel: 6.8 km/s",   "Dist: 2.8B km",    0.39, 0.75, 0.87 },
        { "Neptune",        740.0f,     0,  -20,   0.0,   13.05,   0.1,  14,     17.1f, "Mass: 17.1 Earth",   "Vel: 5.4 km/s",   "Dist: 4.5B km",    0.25, 0.41, 0.88 },
        { "Supermassive BH", -2500.0f,  0,    0,   0.0,    0.0,    0.0,  55,  500000.0f, "Mass: 200M Earth",   "Vel: 1000 km/s",  "Dist: Galactic",   0.08, 0.08, 0.12 }
    };

    bool showProperties = false;
    bool tabWasPressed = false;
    bool mouseWasPressed = false;
    int followedPlanet = -1;

    bool optCards = true;
    bool optAxes = true;
    bool optTrails = true;
    bool optBubble = true;
    bool optSpaceBox = true;
    float spaceBoxIntensity = 2.0f; // Default brighter setting (Levels 1 to 4)

    while (!glfwWindowShouldClose(engine.window)) {
        // Sun control
        if (glfwGetKey(engine.window, GLFW_KEY_UP)) {
            planets[0].vy += 0.05f;
        }
        if (glfwGetKey(engine.window, GLFW_KEY_DOWN)) {
            planets[0].vy -= 0.05f;
        }
    
        // determine camera target pos
        glm::vec3 cameraTarget(0.0f, 0.0f, 0.0f);
        if (followedPlanet >= 0 && followedPlanet < planets.size()) {
            cameraTarget = glm::vec3(planets[followedPlanet].x, planets[followedPlanet].y, planets[followedPlanet].z);
        }

        engine.run(cameraTarget);
        drawGrid();

        bool mouseClicked = (glfwGetMouseButton(engine.window, GLFW_MOUSE_BUTTON_LEFT) == GLFW_PRESS);
        if (mouseClicked && !mouseWasPressed){
            double mx, my;
            glfwGetCursorPos(engine.window, &mx, &my);
            float bottomMouseY = engine.HEIGHT - my;

            int clickedIndex = -1;

            // Check if click is inside the top-left settings menu when TAB mode is active
            if (showProperties && mx >= 20.0f && mx <= 280.0f && bottomMouseY >= engine.HEIGHT - 157.0f && bottomMouseY <= engine.HEIGHT - 20.0f) {
                float menuY = engine.HEIGHT - 20.0f;
                float menuX = 20.0f; // Định nghĩa menuX để tránh lỗi undefined
                if (bottomMouseY >= menuY - 50.0f && bottomMouseY <= menuY - 30.0f) optCards = !optCards;
                else if (bottomMouseY >= menuY - 72.0f && bottomMouseY <= menuY - 52.0f) optAxes = !optAxes;
                else if (bottomMouseY >= menuY - 94.0f && bottomMouseY <= menuY - 74.0f) optTrails = !optTrails;
                else if (bottomMouseY >= menuY - 116.0f && bottomMouseY <= menuY - 96.0f) optBubble = !optBubble;
                else if (bottomMouseY >= menuY - 138.0f && bottomMouseY <= menuY - 118.0f) {
                    if (mx > menuX + 135.0f) {
                        spaceBoxIntensity += 1.0f; // Cycle brightness level on click
                        if (spaceBoxIntensity > 4.0f) spaceBoxIntensity = 1.0f;
                    } else {
                        optSpaceBox = !optSpaceBox;
                    }
                }
            } else {
                // 1. TAB MODE Card Selection (Đã sửa bỏ dấu { thừa)
                if (showProperties) {
                    for (size_t i = 0; i < planets.size(); i++) {
                        glm::vec3 worldPos(planets[i].x, planets[i].y, planets[i].z);
                        glm::vec3 screenPos = glm::project(worldPos, engine.view, engine.projection, glm::vec4(0, 0, engine.WIDTH, engine.HEIGHT));
                        
                        float sx = screenPos.x;
                        float sy = screenPos.y;

                        float cardW = 240.0f;
                        float cardH = 95.0f;
                        float cardX = sx + 20.0f;
                        float cardY = sy - 20.0f;

                        if (cardX + cardW > engine.WIDTH) cardX = sx - cardW - 20.0f;
                        if (cardY - cardH < 0) cardY = sy + cardH + 20.0f;
                        
                        if (mx >= cardX && mx <= cardX + cardW && bottomMouseY >= cardY - cardH && bottomMouseY <= cardY) {
                            clickedIndex = (int)i;
                            break;
                        }
                    }
                }

            // 2. 3D Planet Sphere Click Selection
                if (clickedIndex == -1) {
                    float minDist = 9999.0f;
                    for (size_t i = 0; i < planets.size(); i++) {
                        glm::vec3 worldPos(planets[i].x, planets[i].y, planets[i].z);
                        glm::vec3 screenPos = glm::project(worldPos, engine.view, engine.projection, glm::vec4(0, 0, engine.WIDTH, engine.HEIGHT));

                        float screenX = screenPos.x;
                        float screenY = engine.HEIGHT - screenPos.y;

                        float dx = (float)mx - screenX;
                        float dy = (float)my - screenY;
                        float dist2D = sqrt(dx * dx + dy * dy);
                        float clickRadius = fmax(planets[i].r * (1000.0f / engine.dist), 18.0f);

                        if (dist2D < clickRadius && dist2D < minDist) {
                            minDist = dist2D;
                            clickedIndex = (int)i;
                        }
                    }
                }
                followedPlanet = clickedIndex;
            }
        }
        mouseWasPressed = mouseClicked;

        // Toggle properties display with TAB key
        bool tabIsPressed = (glfwGetKey(engine.window, GLFW_KEY_TAB) == GLFW_PRESS);
        if (tabIsPressed && !tabWasPressed) {
            showProperties = !showProperties;
        }
        tabWasPressed = tabIsPressed;

        // 1. Acceleration physics
        for (auto& p : planets) {
            float ax = 0, ay = 0, az = 0;

            for (auto& other : planets) {
                if (&other == &p) continue;

                float dx = other.x - p.x;
                float dy = other.y - p.y;
                float dz = other.z - p.z;
                float r2 = dx * dx + dy * dy + dz * dz + 25.0f;
                float r = sqrt(r2);

                float a = G * other.m / r2;
                ax += a * dx / r;
                ay += a * dy / r;
                az += a * dz / r;
            }
            p.vx += ax;
            p.vy += ay;
            p.vz += az;
        }

        // 2. Move and draw planets, rings (if had), orbit time
        double currentTime = glfwGetTime();
        const auto& sun = planets[0];

        for (auto& p : planets) {
            p.x += p.vx;
            p.y += p.vy;
            p.z += p.vz;

            if (p.name != "Sun" && p.name != "Supermassive Black Hole") {
                float dx = p.x - sun.x;
                float dz = p.z - sun.z;
                float currentAngle = atan2(dz, dx);

                if (!p.angleInitialized) {
                    p.prevAngle = currentAngle;
                    p.currentOrbitStartTime = currentTime;
                    p.angleInitialized = true;
                } else {
                    float diff = currentAngle - p.prevAngle;
                    if (diff >  PI) diff -= 2.0f * PI;
                    if (diff < -PI) diff += 2.0f * PI;

                    p.accumulatedAngle += abs(diff);
                    p.prevAngle = currentAngle;

                    if (p.accumulatedAngle >= 2.0f * PI) {
                        p.completedOrbits++;
                        p.accumulatedAngle -= 2.0f * PI;
                        p.lastOrbitDuration = currentTime - p .currentOrbitStartTime;
                        p.currentOrbitStartTime = currentTime;
                    }   
                }    
            }

            if (showProperties && optTrails) {
                p.trail.push_back(glm::vec3(p.x,p.y,p.z));
                if(p.trail.size() > 150) {
                    p.trail.erase(p.trail.begin());
                }

            }
            
            // Draw Black Hole with Accretion Disk or normal sphere for planets
            if (p.name == "Supermassive BH") {
                drawBlackHole(p);
            } else {
                drawSphere(p, planets[0]);
                if (p.name == "Jupiter") {
                    drawRing(p, planets[0], p.r * 1.3f, p.r * 2.1f, 0.8f, 0.7f, 0.6f);
                }
            }

            if (showProperties && optAxes) {
                drawPlanetAxes(p);
            }
        }

        // Render Space Box and System Bubble centered on the Black Hole (planets[10])
        if (showProperties) {
            glm::vec3 bhCenter(planets[10].x, planets[10].y, planets[10].z);
            if (optSpaceBox) drawSpaceBox(bhCenter, 3500.0f, 10, spaceBoxIntensity, planets);
            if (optBubble) drawBoundingBubble(3500.0f, bhCenter);
            if (optTrails) drawPlanetTrails(planets);
        }

        // 3. Render HUD cards with fully visible text when TAB is active
        if (showProperties) {
            glDisable(GL_DEPTH_TEST);
            glEnable(GL_BLEND);
            glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
            
            glMatrixMode(GL_PROJECTION);
            glPushMatrix();
            glLoadIdentity();
            glOrtho(0, engine.WIDTH, 0, engine.HEIGHT, -1.0, 1.0);

            glMatrixMode(GL_MODELVIEW);
            glPushMatrix();
            glLoadIdentity();

            // --- Render Top-Left Settings Checkbox Menu ---
            float menuX = 20.0f;
            float menuY = engine.HEIGHT - 20.0f;
            float menuW = 260.0f; // Wider menu width
            float menuH = 137.0f;

            glColor4f(0.06f, 0.08f, 0.12f, 0.90f);
            glBegin(GL_QUADS);
            glVertex2f(menuX, menuY); glVertex2f(menuX + menuW, menuY);
            glVertex2f(menuX + menuW, menuY - menuH); glVertex2f(menuX, menuY - menuH);
            glEnd();

            glColor3f(0.4f, 0.6f, 0.9f);
            glLineWidth(1.5f);
            glBegin(GL_LINE_LOOP);
            glVertex2f(menuX, menuY); glVertex2f(menuX + menuW, menuY);
            glVertex2f(menuX + menuW, menuY - menuH); glVertex2f(menuX, menuY - menuH);
            glEnd();

            drawText("DISPLAY SETTINGS:", menuX + 12.0f, menuY - 20.0f, 0.6f, 1.0f, 1.0f, 1.0f);
            
            auto drawCheckbox = [&](string label, bool checked, float yOffset) {
                float boxX = menuX + 15.0f;
                float boxY = menuY - yOffset;
                glColor3f(0.3f, 0.4f, 0.5f);
                glBegin(GL_LINE_LOOP);
                glVertex2f(boxX, boxY); glVertex2f(boxX + 12, boxY);
                glVertex2f(boxX + 12, boxY - 12); glVertex2f(boxX, boxY - 12);
                glEnd();
                if (checked) {
                    glColor3f(0.3f, 1.0f, 0.6f);
                    glBegin(GL_QUADS);
                    glVertex2f(boxX + 3, boxY - 3); glVertex2f(boxX + 9, boxY - 3);
                    glVertex2f(boxX + 9, boxY - 9); glVertex2f(boxX + 3, boxY - 9);
                    glEnd();
                }
                drawText(label, boxX + 22.0f, boxY - 10.0f, 0.55f, 0.9f, 0.9f, 0.9f);
            };

            drawCheckbox("Name Cards", optCards, 40.0f);
            drawCheckbox("XYZ Axes", optAxes, 62.0f);
            drawCheckbox("Planet Trails", optTrails, 84.0f);
            drawCheckbox("System Bubble", optBubble, 106.0f);
            drawCheckbox("Space Box", optSpaceBox, 128.0f);

            // Render brightness level selector text next to Space Box
            ostringstream ossInten;
            ossInten << "[Lv." << (int)spaceBoxIntensity << "]";
            drawText(ossInten.str(), menuX + 145.0f, menuY - 128.0f, 0.5f, 1.0f, 0.85f, 0.3f);

            // --- Render Planet Cards if enabled ---
            if (optCards) {
                const auto& sun = planets[0];
                for (const auto& p : planets) {
                    glm::vec3 worldPos(p.x, p.y, p.z);
                    glm::vec3 screenPos = glm::project(worldPos, engine.view, engine.projection, glm::vec4(0, 0, engine.WIDTH, engine.HEIGHT));
                    
                    float sx = screenPos.x;
                    float sy = screenPos.y;

                    float relVx = p.vx - sun.vx;
                    float relVy = p.vy - sun.vy;
                    float relVz = p.vz - sun.vz;
                    float codeSpeed = sqrt(relVx * relVx + relVy * relVy);
                    float codeDist = sqrt((p.x - sun.x) * (p.x - sun.x) + (p.y - sun.y) * (p.y - sun.y) + (p.z - sun.z) * (p.z - sun.z));

                    float cardW = 240.0f;
                    float cardH = 125.0f;
                    float cardX = sx + 20.0f;
                    float cardY = sy - 20.0f;

                    if (cardX + cardW > engine.WIDTH) cardX = sx - cardW - 20.0f;
                    if (cardY - cardH < 0) cardY = sy + cardH + 20.0f;

                    glColor4f(0.06f, 0.08f, 0.12f, 0.90f);
                    glBegin(GL_QUADS);
                    glVertex2f(cardX, cardY); glVertex2f(cardX + cardW, cardY);
                    glVertex2f(cardX + cardW, cardY - cardH); glVertex2f(cardX, cardY - cardH);
                    glEnd();

                    glColor3f(p.cr, p.cg, p.cb);
                    glLineWidth(1.5f);
                    glBegin(GL_LINE_LOOP);
                    glVertex2f(cardX, cardY); glVertex2f(cardX + cardW, cardY);
                    glVertex2f(cardX + cardW, cardY - cardH); glVertex2f(cardX, cardY - cardH);
                    glEnd();

                    // cal orbit time
                    float fractionDone = p.accumulatedAngle / (2.0f * PI);
                    double elapsedTime = currentTime - p.currentOrbitStartTime;
                    double estRealTotalTime = (fractionDone > 0.01f) ? (elapsedTime / fractionDone) : 0.0;
                    double estRealRemTime = estRealTotalTime - elapsedTime;

                    // code time: T = 2*PI*r / v
                    float estCodePeriod = (codeSpeed > 0.001f) ? (2.0f * PI * codeDist / codeSpeed) : 0.0f;
                    float estCodeRemTime = estCodePeriod * (1.0f - fractionDone);

                    ostringstream ossXYZ, ossSpeed, ossDist, ossOrbits, ossCodeTime, ossRealTime;
                    ossXYZ << "XYZ: " << (int)p.x << ", " << (int)p.y << ", " << (int)p.z;
                    ossSpeed << "Speed:" << fixed << setprecision(1) << codeSpeed << " | " << p.realSpeed;
                    ossDist << "Distance To Sun:" << fixed << setprecision(1) << codeDist;
                    ossOrbits << "Orbits Completed: " << p.completedOrbits;

                    if (p.name == "Sun") {
                        ossCodeTime << "Estimated Period: N/A";
                        ossRealTime << "Real Rem: N/A";
                    } else {
                        ossCodeTime << "Est Code T: " << fixed << setprecision(1) << estCodeRemTime << "s rem";
                        ossRealTime << "Est Real T: " << fixed << setprecision(1) << estRealRemTime << "s rem";
                    }

                    drawText(p.name, cardX + 10.0f, cardY - 18.0f, 0.9f, 1.0f, 1.0f, 1.0f);
                    drawText(p.realMass, cardX + 10.0f, cardY - 34.0f, 0.55f, 0.8f, 0.8f, 0.8f);
                    drawText(ossXYZ.str(), cardX + 10.0f, cardY - 48.0f, 0.55f, 0.7f, 1.0f, 0.7f);
                    drawText(ossSpeed.str(), cardX + 10.0f, cardY - 62.0f, 0.55f, 0.6f, 0.9f, 1.0f);
                    drawText(ossDist.str(), cardX + 10.0f, cardY - 76.0f, 0.55f, 0.6f, 0.9f, 1.0f);
                    drawText(ossOrbits.str(), cardX + 10.0f, cardY - 90.0f, 0.55f, 1.0f, 0.85f, 0.3f);
                    drawText(ossCodeTime.str(), cardX + 10.0f, cardY - 104.0f, 0.55f, 0.4f, 1.0f, 0.9f);
                    drawText(ossRealTime.str(), cardX + 10.0f, cardY - 118.0f, 0.55f, 1.0f, 0.5f, 0.8f);
                }
            }

            glMatrixMode(GL_PROJECTION);
            glPopMatrix();
            glMatrixMode(GL_MODELVIEW);
            glPopMatrix();

            glDisable(GL_BLEND);
            glEnable(GL_DEPTH_TEST);
        }

        // Update window title with following status
        string followName = (followedPlanet >= 0 && followedPlanet < (int)planets.size()) ? planets[followedPlanet].name : "Free Center (0,0,0)";
        string windowTitle = "Solar System Simulator | Following: " + followName + " | [UP/DOWN] Move Sun | [TAB] Toggle HUD";
        glfwSetWindowTitle(engine.window, windowTitle.c_str());

        glfwSwapBuffers(engine.window);
        glfwPollEvents();
    }

    glfwTerminate();
    return 0;
}

// eye=dist⋅(cospsiny,sinp,cospcosy)
// x=rsinθcosϕ
// y=rcosθ
// z=rsinθsinϕ

//brightness=max(0, cosα)
//vec(n)⋅vec(l) = nxlx + nyly + nzlz = cosα
//light = 0.15 + 0.85 max (0, cosα)