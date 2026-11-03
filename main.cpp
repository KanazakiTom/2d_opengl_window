#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <iostream>
#include <cmath>
#include <vector>
using namespace std;

#define PI 3.14159265358979323846
#define G 0.66743

struct Planet {
    string name;
    float x, y;
    float vx, vy;
    float r;
    float m;
    float cr, cg, cb; // color components
};

struct Engine {
    GLFWwindow* window;
    int WIDTH = 1600, HEIGHT = 900;

    Engine() {
        // wake the f up GLFW
        if (!glfwInit()) {
            cerr << "glfw didn't start bruh" << endl;
            exit(EXIT_FAILURE);
        }

        // ask the operating system for a window
        window = glfwCreateWindow(WIDTH, HEIGHT, "Solar System", nullptr, nullptr);
        if (!window) {
            cerr << "no window for you nerd" << endl;
            glfwTerminate();
            exit(EXIT_FAILURE);
        }

        // point every future drawing command at this window
        glfwMakeContextCurrent(window);
        glewInit();

        glfwSwapInterval(1); // wait for vsync

        int fbWidth, fbHeight;
        glfwGetFramebufferSize(window, &fbWidth, &fbHeight);
        glViewport(0, 0, fbWidth, fbHeight);
    }

    // this runs once per frame
    void run() {
        // wipe the screen to almost-black
        glClearColor(0.05f, 0.05f, 0.08f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);

        // put (0, 0) in the middle instead of the corner
        glMatrixMode(GL_PROJECTION);
        glLoadIdentity();
        glOrtho(-WIDTH / 2.0, WIDTH / 2.0, -HEIGHT / 2.0, HEIGHT / 2.0, -1.0, 1.0);

        glMatrixMode(GL_MODELVIEW);
        glLoadIdentity();
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

int main() {
    Engine engine;

        std::vector<Planet> planets = {
            //  Name        x     y     vx     vy    r     m     cr    cg    cb
            { "Sun",       0,     0,   0.0,   0.0,  45, 2500,  1.00, 0.78, 0.25 },
            { "Mercury", 100,     0,   0.0,   4.1,   6,  0.1,  0.69, 0.69, 0.69 },
            { "Venus",   150,     0,   0.0,   3.3,  10,  0.8,  0.93, 0.79, 0.51 },
            { "Earth",   210,     0,   0.0,   2.8,  12,  1.0,  0.35, 0.62, 1.00 },
            { "Mars",    290,     0,   0.0,   2.4,   9,  0.11, 0.90, 0.38, 0.24 },
            { "Jupiter", 420,     0,   0.0,   2.0,  22, 30.0,  0.85, 0.65, 0.45 },
            { "Saturn",  540,     0,   0.0,   1.8,  17,  9.0,  0.91, 0.82, 0.56 },
            { "Uranus",  650,     0,   0.0,   1.6,  14,  1.4,  0.39, 0.75, 0.87 },
            { "Neptune", 740,     0,   0.0,   1.5,  14,  1.5,  0.25, 0.41, 0.88 },
        };

    // keep running until someone closes the window
    while (!glfwWindowShouldClose(engine.window)) {
        engine.run();

        // 1. work out every planet's acceleration
        for (auto& p : planets) {
            float ax = 0, ay = 0;

            for (auto& other : planets) {
                if (&other == &p) continue;   // a planet doesn't pull itself

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

        // 2. then ove and draw them
        for (auto& p : planets) {
            p.x += p.vx;
            p.y += p.vy;

            glColor3f(p.cr, p.cg, p.cb);
            drawCircle(p.x, p.y, p.r);
        }

        // 3. mouse hover
        double mouseX, mouseY;
        glfwGetCursorPos(engine.window, &mouseX, &mouseY);

        float worldX = static_cast<float>(mouseX) - engine.WIDTH / 2.0f;
        float worldY = static_cast<float>(engine.HEIGHT / 2.0f - mouseY);

        bool hoveredAny = false;
        for (const auto&p : planets) {
            float dx = p.x - worldX;
            float dy = p.y - worldY;
            float R2 = dx*dx + dy*dy;

            float hoverRadius = p.r + 6.0f;
            if (R2 <= hoverRadius*hoverRadius) {
                string title = "[ " + p.name + " ] " +
                               "Mass: " + to_string(p.m) + " | " + 
                               "Pos: (" + to_string((int)p.x) + ", " + to_string((int)p.y) + ") | " +
                               "Vel: (" + to_string(p.vx) + ", " + to_string(p.vy) + ")";
                glfwSetWindowTitle(engine.window, title.c_str());
                hoveredAny = true;
                break;
            }
        } 

        if(!hoveredAny) {
            glfwSetWindowTitle(engine.window, "Solar System Simulator (Hover mouse over Sun or Planets for info)");
        }

        glfwSwapBuffers(engine.window);
        glfwPollEvents();
    }

    glfwTerminate();
    return 0;
}

//x = cx+r*cos(theta), y = cy+r*sin(theta)
//x = x + vx {x ← x + vΔt}
//vx = vx + ax {v ← v + aΔt}

//dx = xother - x
//dy = yother - y
//r = \sqrt(dx^2 + dy^2 {+25})  

//ax = adx/r
//ay = ady/r