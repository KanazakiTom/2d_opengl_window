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
    float vx, vy;
    float r;
    float m;          // Code mass used in physics
    string realMass;
    string realSpeed;
    string realDist;
    float cr, cg, cb; // Color components
};

struct Engine {
    GLFWwindow* window;
    int WIDTH = 1920, HEIGHT = 1080;

    //camera
    float yaw = 0.0f; // left right
    float pitch = 0.0f; // up down
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

// 3D XYZ Coordinate Axes (Red = X, Green = Y, Blue = Z)
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

void drawPLanetAxes(const Planet& p){
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

void drawRing(const Planet& p, float innerR, float outerR, float r, float g, float b) {
    int segments = 60;
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glBegin(GL_QUAD_STRIP);

    for (int i = 0; i <= segments; i++) {
        float angle = 2.0f * PI * i /segments;
        float cosA = cos(angle);
        float sinA = sin(angle);

        // innner ring vertex
        glColor4f(r, g, b, 0.2f);
        glVertex3f(p.x + innerR * cosA, p.y, p.z + innerR * sinA);
        
        // outer ring vertex
        glColor4f(r, g, b, 0.6f);
        glVertex3f(p.x + outerR * cosA, p.y, p.z + outerR * sinA);
    }
    glEnd();
    glDisable(GL_BLEND);
}

int main() {
    Engine engine;

    std::vector<Planet> planets = {
        //  Name        x     y     z     vx     vy    r        m           realMass            realSpeed         realDist          cr    cg    cb
        { "Sun",       0,     0,    0,   0.0,   0.0,  45,   2500.0f,  "Mass: 333k Earth",   "Vel: 220 km/s",   "Dist: Center",     1.00, 0.78, 0.25 },
        { "Mercury", 100,     0,    0,   0.0,   4.1,   6,      0.1f,  "Mass: 0.055 Earth",  "Vel: 47.9 km/s",  "Dist: 58M km",     0.69, 0.69, 0.69 },
        { "Venus",   150,     0,    0,   0.0,   3.3,  10,      0.8f,  "Mass: 0.815 Earth",  "Vel: 35.0 km/s",  "Dist: 108M km",    0.93, 0.79, 0.51 },
        { "Earth",   210,     0,    0,   0.0,   2.8,  12,      1.0f,  "Mass: 1.0 Earth",    "Vel: 29.8 km/s",  "Dist: 150M km",    0.35, 0.62, 1.00 },
        { "Mars",    290,     0,    0,   0.0,   2.4,   9,      0.11f, "Mass: 0.107 Earth",  "Vel: 24.1 km/s",  "Dist: 228M km",    0.90, 0.38, 0.24 },
        { "Jupiter", 420,     0,    0,   0.0,   2.0,  22,     30.0f,  "Mass: 317.8 Earth",  "Vel: 13.1 km/s",  "Dist: 778M km",    0.85, 0.65, 0.45 },
        { "Saturn",  540,     0,    0,   0.0,   1.8,  17,      9.0f,  "Mass: 95.2 Earth",   "Vel: 9.7 km/s",   "Dist: 1.4B km",    0.91, 0.82, 0.56 },
        { "Uranus",  650,     0,    0,   0.0,   1.6,  14,      1.4f,  "Mass: 14.5 Earth",   "Vel: 6.8 km/s",   "Dist: 2.8B km",    0.39, 0.75, 0.87 },
        { "Neptune", 740,     0,    0,   0.0,   1.5,  14,      1.5f,  "Mass: 17.1 Earth",   "Vel: 5.4 km/s",   "Dist: 4.5B km",    0.25, 0.41, 0.88 },
    };

    bool showProperties = false;
    bool tabWasPressed = false;
    bool mouseWasPressed = false;
    int followedPlanet = -1;

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

            //float minDist = 9999.0f;
            
            // 1. TAB MODE 
            if (showProperties) {
                const auto& sun = planets[0];
                for (size_t i = 0; i < planets.size(); i++) {
                    glm::vec3 worldPos(planets[i].x, planets[i].y, planets[i].z);
                    glm::vec3 screenPos = glm::project(worldPos, engine.view, engine.projection, glm::vec4(0, 0, engine.WIDTH, engine.HEIGHT));

                    float screenX = screenPos.x;
                    float screenY = screenPos.y;

                    float cardW = 240.0f;
                    float cardH = 95.0f;
                    float cardX = screenX + 20.0f;
                    float cardY = screenY - 20.0f;

                    if (cardX + cardW > engine.WIDTH) cardX= screenX - cardW - 20.0f;
                    if (cardY - cardH < 0) cardY = screenY + cardH + 20.0f;
                    
                    if (mx >= cardX && mx <=cardX + cardW && bottomMouseY >= cardY - cardH && bottomMouseY <= cardY) {
                        clickedIndex = (int)i;
                        break;;
                    }
                }
            }

            //2. no card clicked, check 3D planet 
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
        mouseWasPressed = mouseClicked;

        // Toggle properties display with TAB key
        bool tabIsPressed = (glfwGetKey(engine.window, GLFW_KEY_TAB) == GLFW_PRESS);
        if (tabIsPressed && !tabWasPressed) {
            showProperties = !showProperties;
        }
        tabWasPressed = tabIsPressed;

        // 1. Acceleration physics
        for (auto& p : planets) {
            float ax = 0, ay = 0;

            for (auto& other : planets) {
                if (&other == &p) continue;

                float dx = other.x - p.x;
                float dy = other.y - p.y;
                float r2 = dx * dx + dy * dy + 25.0f;
                float r = sqrt(r2);

                float a = G * other.m / r2;
                ax += a * dx / r;
                ay += a * dy / r;
            }
            p.vx += ax;
            p.vy += ay;
        }

        // 2. Move and draw planets, rings (if had)
        for (auto& p : planets) {
            p.x += p.vx;
            p.y += p.vy;

            drawSphere(p, planets[0]);

            // render Jupiter's ring
            if (p.name == "Jupiter") {
                drawRing(p, p.r * 1.3f, p.r * 2.1f, 0.8f, 0.7f, 0.6f);
            }

            if (showProperties) {
                drawPLanetAxes(p);
            }
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

            const auto& sun = planets[0];

            for (const auto& p : planets) {
                // Project 3D world coordinates directly to 2D window screen coordinates
                glm::vec3 worldPos(p.x, p.y, p.z);
                glm::vec3 screenPos = glm::project(worldPos, engine.view, engine.projection, glm::vec4(0, 0, engine.WIDTH, engine.HEIGHT));
                
                float sx = screenPos.x;
                float sy = screenPos.y;

                float relVx = p.vx - sun.vx;
                float relVy = p.vy - sun.vy;
                float codeSpeed = sqrt(relVx * relVx + relVy * relVy);
                float codeDist = sqrt((p.x - sun.x) * (p.x - sun.x) + (p.y - sun.y) * (p.y - sun.y) + (p.z - sun.z) * (p.z - sun.z));

                float cardW = 240.0f;
                float cardH = 95.0f;
                float cardX = sx + 20.0f;
                float cardY = sy - 20.0f;

                if (cardX + cardW > engine.WIDTH) cardX = sx - cardW - 20.0f;
                if (cardY - cardH < 0) cardY = sy + cardH + 20.0f;

                // Background card box
                glColor4f(0.06f, 0.08f, 0.12f, 0.90f);
                glBegin(GL_QUADS);
                glVertex2f(cardX, cardY);
                glVertex2f(cardX + cardW, cardY);
                glVertex2f(cardX + cardW, cardY - cardH);
                glVertex2f(cardX, cardY - cardH);
                glEnd();

                // Card border
                glColor3f(p.cr, p.cg, p.cb);
                glLineWidth(1.5f);
                glBegin(GL_LINE_LOOP);
                glVertex2f(cardX, cardY);
                glVertex2f(cardX + cardW, cardY);
                glVertex2f(cardX + cardW, cardY - cardH);
                glVertex2f(cardX, cardY - cardH);
                glEnd();

                // Format strings including XYZ coordinates
                ostringstream ossXYZ, ossSpeed, ossDist;
                ossXYZ << "XYZ: " << (int)p.x << ", " << (int)p.y << ", " << (int)p.z;
                ossSpeed << "Spd:" << fixed << setprecision(1) << codeSpeed << " | " << p.realSpeed;
                ossDist << "Dist:" << fixed << setprecision(1) << codeDist;

                // Draw crisp upright vector text inside the card
                drawText(p.name, cardX + 10.0f, cardY - 18.0f, 0.9f, 1.0f, 1.0f, 1.0f);
                drawText(p.realMass, cardX + 10.0f, cardY - 34.0f, 0.55f, 0.8f, 0.8f, 0.8f);
                drawText(ossXYZ.str(), cardX + 10.0f, cardY - 48.0f, 0.55f, 0.7f, 1.0f, 0.7f);
                drawText(ossSpeed.str(), cardX + 10.0f, cardY - 62.0f, 0.55f, 0.6f, 0.9f, 1.0f);
                drawText(ossDist.str(), cardX + 10.0f, cardY - 76.0f, 0.55f, 0.6f, 0.9f, 1.0f);
            }
            // Restore 3D matrices & depth test
            glMatrixMode(GL_PROJECTION);
            glPopMatrix();
            glMatrixMode(GL_MODELVIEW);
            glPopMatrix();

            glDisable(GL_BLEND);
            glEnable(GL_DEPTH_TEST);
        } else {
            glfwSetWindowTitle(engine.window, "Solar System Simulator [Press TAB to toggle HUD properties]");
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