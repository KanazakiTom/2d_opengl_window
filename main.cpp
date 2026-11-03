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
    void run() {
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
        glm::mat4 projection = glm::perspective(glm::radians(45.0f), (float)WIDTH/HEIGHT, 1.0f, 5000.0f);
        glMatrixMode(GL_PROJECTION);
        glLoadMatrixf(glm::value_ptr(projection));

        glm::vec3 eye(dist*cos(pitch) *sin(yaw), dist*sin(pitch), dist*cos(pitch)*cos(yaw));
        glm::mat4 view = glm::lookAt(eye, glm::vec3(0, 0, 0), glm::vec3(0, 1, 0));
        
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

int main() {
    Engine engine;

    std::vector<Planet> planets = {
        //  Name        x     y     vx     vy    r     m         realMass                     realSpeed      realDist          cr    cg    cb
        { "Sun",       0,     0,   0.0,   0.0,  45,   2500.0f,  "Mass: 333k Earth",   "Vel: 220 km/s",   "Dist: Center",     1.00, 0.78, 0.25 },
        { "Mercury", 100,     0,   0.0,   4.1,   6,      0.1f,  "Mass: 0.055 Earth",  "Vel: 47.4 km/s",  "Dist: 58M km",     0.69, 0.69, 0.69 },
        { "Venus",   150,     0,   0.0,   3.3,  10,      0.8f,  "Mass: 0.815 Earth",  "Vel: 35.0 km/s",  "Dist: 108M km",    0.93, 0.79, 0.51 },
        { "Earth",   210,     0,   0.0,   2.8,  12,      1.0f,  "Mass: 1.0 Earth",    "Vel: 29.8 km/s",  "Dist: 150M km",    0.35, 0.62, 1.00 },
        { "Mars",    290,     0,   0.0,   2.4,   9,      0.11f, "Mass: 0.107 Earth",  "Vel: 24.1 km/s",  "Dist: 228M km",    0.90, 0.38, 0.24 },
        { "Jupiter", 420,     0,   0.0,   2.0,  22,     30.0f,  "Mass: 317.8 Earth",  "Vel: 13.1 km/s",  "Dist: 778M km",    0.85, 0.65, 0.45 },
        { "Saturn",  540,     0,   0.0,   1.8,  17,      9.0f,  "Mass: 95.2 Earth",   "Vel: 9.7 km/s",   "Dist: 1.4B km",    0.91, 0.82, 0.56 },
        { "Uranus",  650,     0,   0.0,   1.6,  14,      1.4f,  "Mass: 14.5 Earth",   "Vel: 6.8 km/s",   "Dist: 2.8B km",    0.39, 0.75, 0.87 },
        { "Neptune", 740,     0,   0.0,   1.5,  14,      1.5f,  "Mass: 17.1 Earth",   "Vel: 5.4 km/s",   "Dist: 4.5B km",    0.25, 0.41, 0.88 },
    };

    bool showProperties = false;
    bool tabWasPressed = false;

    while (!glfwWindowShouldClose(engine.window)) {
        engine.run();

        drawGrid();

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

        // 2. Move and draw planets
        for (auto& p : planets) {
            p.x += p.vx;
            p.y += p.vy;

            glColor3f(p.cr, p.cg, p.cb);
            drawCircle(p.x, p.y, p.r);
        }

        // 3. Render HUD cards with fully visible text when TAB is active
        if (showProperties) {
            glEnable(GL_BLEND);

            
            glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

            const auto& sun = planets[0];

            for (const auto& p : planets) {
                float relVx = p.vx - sun.vx;
                float relVy = p.vy - sun.vy;
                float codeSpeed = sqrt(relVx * relVx + relVy * relVy);
                float codeDist = sqrt((p.x - sun.x) * (p.x - sun.x) + (p.y - sun.y) * (p.y - sun.y));

                float cardX = p.x + p.r + 15.0f;
                float cardY = p.y - 10.0f;
                float cardW = 210.0f;
                float cardH = 75.0f;

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

                // Draw crisp, properly scaled vector text inside the card
                drawText(p.name, cardX + 10.0f, cardY - 18.0f, 0.9f, 1.0f, 1.0f, 1.0f);
                drawText(p.realMass, cardX + 10.0f, cardY - 34.0f, 0.6f, 0.8f, 0.8f, 0.8f);
                
                ostringstream ossSpeed, ossDist;
                ossSpeed << "Spd:" << fixed << setprecision(1) << codeSpeed << " | " << p.realSpeed;
                ossDist << "Dist:" << fixed << setprecision(1) << codeDist;
                
                drawText(ossSpeed.str(), cardX + 10.0f, cardY - 48.0f, 0.55f, 0.6f, 0.9f, 1.0f);
                drawText(ossDist.str(), cardX + 10.0f, cardY - 62.0f, 0.55f, 0.6f, 0.9f, 1.0f);
            }
            glDisable(GL_BLEND);
            
            glfwSetWindowTitle(engine.window, "Solar System Simulator [HUD ON - Press TAB to hide]");
        } else {
            glfwSetWindowTitle(engine.window, "Solar System Simulator [Press TAB to toggle HUD properties]");
        }

        glfwSwapBuffers(engine.window);
        glfwPollEvents();
    }

    glfwTerminate();
    return 0;
}

// eye=dist⋅(cospsiny,sinp,cospcosy)