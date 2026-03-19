#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <iostream>
#include <cmath>
#include <algorithm>

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

// --- SHADEREK ---
const char* vertexShaderSource = R"(
    #version 330 core
    layout (location = 0) in vec2 aPos;
    void main() {
        gl_Position = vec4(aPos, 0.0, 1.0);
    }
)";

const char* fragmentShaderSource = R"(
    #version 330 core
    out vec4 FragColor;
    uniform vec2 u_circlePos;
    uniform float u_lineY;
    uniform bool u_intersecting;

    void main() {
        vec2 uv = gl_FragCoord.xy;
        vec3 yellow = vec3(1.0, 1.0, 0.0);
        vec3 blue = vec3(0.0, 0.0, 1.0);
        vec3 red = vec3(1.0, 0.0, 0.0);
        vec3 green = vec3(0.0, 1.0, 0.0);

        vec3 color = yellow; // Sárga háttér (Kötelező 1.)
        float radius = 50.0;

        // Kör rajzolása (Kötelező 2.)
        float dist = distance(uv, u_circlePos);
        if (dist < radius) {
            float t = dist / radius;
            if (u_intersecting) {
                // METSZÉSKOR: Színcsere
                color = mix(green, red, t); 
            } else {
                // ALAPÁLLAPOT: Piros centrum, zöld határvonal (Feladat 2. pont)
                color = mix(red, green, t); 
            }
        }

        // Kék szakasz (Kötelező 4.) - 1/3 szélesség = 200 pixel, 3 pixel vastag
        if (uv.x >= 200.0 && uv.x <= 400.0 && abs(uv.y - u_lineY) <= 1.5) {
            color = blue;
        }

        FragColor = vec4(color, 1.0);
    }
)";

// Állapotváltozók
float circleX = 300.0f;
float circleY = 300.0f;
float lineY = 300.0f;
float dx = 4.0f;        // Kezdeti sebesség (vízszintes)
float dy = 0.0f;        // Kezdeti függőleges sebesség

// Felhasználói interakció
void processInput(GLFWwindow* window) {
    // Kék szakasz mozgatása (Szabadon választható 1.)
    if (glfwGetKey(window, GLFW_KEY_UP) == GLFW_PRESS) lineY += 5.0f;
    if (glfwGetKey(window, GLFW_KEY_DOWN) == GLFW_PRESS) lineY -= 5.0f;

    // --- SZABADON VÁLASZTHATÓ 3. FELADAT ---
    // 'S' billentyűre 25 fokos szögben, 10 egységnyi sebességgel indul el
    if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS) {
        float alpha = 25.0f * (M_PI / 180.0f); // fokból radián
        float speed = 10.0f;                   // 10 pixel hosszú vektor
        dx = speed * cos(alpha);
        dy = speed * sin(alpha);
    }

    // Határok a szakasznak
    if (lineY < 2) lineY = 2;
    if (lineY > 598) lineY = 598;
}

// Metszés vizsgálata (Szabadon választható 2.-höz)
bool checkIntersection() {
    // Legközelebbi pont a szakaszon (X: 200-400 tartományban)
    float closestX = std::max(200.0f, std::min(circleX, 400.0f));
    float distSq = pow(circleX - closestX, 2) + pow(circleY - lineY, 2);
    return distSq <= pow(50.0f, 2); // Sugarat (50) nézzük
}

void printInstructions() {
    std::cout << "SZAMITOGEPES GRAFIKA - 1. BEADANDO" << std::endl;
    std::cout << "----------------------------------" << std::endl;
    std::cout << "[FEL/LE NYIL] - Kek szakasz mozgatasa" << std::endl;
    std::cout << "[S]           - INDITAS: 25 fokos szogben mozgatas)" << std::endl;
    std::cout << "A kor szine megvaltozik, ha metszi a kek szakaszt." << std::endl;
}

int main() {
    printInstructions();

    if (!glfwInit()) return -1;

    // 600x600-as ablak (Kötelező 1.)
    GLFWwindow* window = glfwCreateWindow(600, 600, "Grafika Beadando 1", NULL, NULL);
    if (!window) { glfwTerminate(); return -1; }
    glfwMakeContextCurrent(window);
    glewInit();

    // Shader összeállítás
    unsigned int vs = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vs, 1, &vertexShaderSource, NULL);
    glCompileShader(vs);
    unsigned int fs = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fs, 1, &fragmentShaderSource, NULL);
    glCompileShader(fs);
    unsigned int prog = glCreateProgram();
    glAttachShader(prog, vs); glAttachShader(prog, fs);
    glLinkProgram(prog);
    glUseProgram(prog);

    // Teljes képernyőt lefedő téglalap (2 db háromszög)
    float vertices[] = { -1,-1, 1,-1, -1,1, 1,-1, 1,1, -1,1 };
    unsigned int VAO, VBO;
    glGenVertexArrays(1, &VAO); glGenBuffers(1, &VBO);
    glBindVertexArray(VAO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);

    int circleLoc = glGetUniformLocation(prog, "u_circlePos");
    int lineLoc = glGetUniformLocation(prog, "u_lineY");
    int intersectLoc = glGetUniformLocation(prog, "u_intersecting");

    // Render ciklus
    while (!glfwWindowShouldClose(window)) {
        processInput(window);

        // Mozgatás
        circleX += dx;
        circleY += dy;

        // VISSZAPATTANÁS (Kötelező 3. és Szabadon választható 3. kombinálva)
        // X irányú falak
        if (circleX <= 50.0f) {
            circleX = 50.0f;
            dx *= -1.0f;
        }
        else if (circleX >= 550.0f) {
            circleX = 550.0f;
            dx *= -1.0f;
        }

        // Y irányú falak (Szabadon választható 3. rész: "képernyő széleit pontosan érintve")
        if (circleY <= 50.0f) {
            circleY = 50.0f;
            dy *= -1.0f;
        }
        else if (circleY >= 550.0f) {
            circleY = 550.0f;
            dy *= -1.0f;
        }

        glClear(GL_COLOR_BUFFER_BIT);

        // Uniformok küldése
        glUniform2f(circleLoc, circleX, circleY);
        glUniform1f(lineLoc, lineY);
        // Szabadon választható 2.: Színcsere ha nincs metszésben
        glUniform1i(intersectLoc, checkIntersection());

        glBindVertexArray(VAO);
        glDrawArrays(GL_TRIANGLES, 0, 6);

        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    glfwTerminate();
    return 0;
}
