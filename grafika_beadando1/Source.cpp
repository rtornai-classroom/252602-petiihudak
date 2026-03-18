#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <iostream>
#include <cmath>
#include <algorithm>

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

        vec3 color = yellow; // Sárga háttér (Kötelezõ 1.)
        float radius = 50.0;

        // Kör rajzolása (Kötelezõ 2.)
        float dist = distance(uv, u_circlePos);
        if (dist < radius) {
            float t = dist / radius;
            if (u_intersecting) {
                // METSZÉSKOR: Színcsere történik (Közép zöld, szél piros)
                color = mix(green, red, t); 
            } else {
                // ALAPÁLLAPOT: Piros centrum, zöld határvonal (Feladat 2. pont)
                color = mix(red, green, t); 
            }
        }

        // Kék szakasz (Kötelezõ 4.)
        if (uv.x >= 200.0 && uv.x <= 400.0 && abs(uv.y - u_lineY) <= 1.5) {
            color = blue;
        }

        FragColor = vec4(color, 1.0);
    }
)";

// Állapotváltozók
float circleX = 300.0f;
float circleY = 300.0f; // Fixen középen marad az Y tengelyen
float lineY = 300.0f;   // A kék szakasz magassága
float dx = 4.0f;        // Vízszintes sebesség

// Felhasználói interakció (Szabadon választható 1.)
void processInput(GLFWwindow* window) {
    if (glfwGetKey(window, GLFW_KEY_UP) == GLFW_PRESS) lineY += 5.0f;
    if (glfwGetKey(window, GLFW_KEY_DOWN) == GLFW_PRESS) lineY -= 5.0f;

    // Határok betartása
    if (lineY < 2) lineY = 2;
    if (lineY > 598) lineY = 598;
}

bool checkIntersection() {
    float closestX = std::max(200.0f, std::min(circleX, 400.0f));
    float distSq = pow(circleX - closestX, 2) + pow(circleY - lineY, 2);
    return distSq <= pow(50.0f, 2);
}

void printInstructions() {
 
    std::cout << "SZAMITOGEPES GRAFIKA - 1. BEADANDO FELADAT" << std::endl;
 
    std::cout << "IRANYITAS:" << std::endl;
    std::cout << "  [FEL NYIL]   - Kek szakasz mozgatasa FELFELE" << std::endl;
    std::cout << "  [LE NYIL]    - Kek szakasz mozgatasa LEFELE" << std::endl;
 
    std::cout << "A PROGRAM MUKODESE:" << std::endl;
    std::cout << "1. A kor automatikusan mozog vizszintesen az X tengely menten." << std::endl;
    std::cout << "2. A kepernyo szelet erintve a kor visszapattan." << std::endl;
    std::cout << "3. Ha a kor es a kek szakasz osszeer (METSZIK EGYMAST):" << std::endl;
    std::cout << "   -> A kor szinei megvaltoznak (kozep zold lesz)." << std::endl;
    std::cout << "4. Ha nem ernek ossze:" << std::endl;
    std::cout << "   -> A kor kozepe piros marad." << std::endl;
 
}

int main() {
    // Tájékoztató kiírása a konzolra
    printInstructions();

    if (!glfwInit()) return -1;

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

    // Teljes ablakot lefedõ négyszög
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

    while (!glfwWindowShouldClose(window)) {
        processInput(window);

        // Vizszintes mozgás és pattogás (Kötelezõ 3.)
        circleX += dx;
        if (circleX <= 50.0f || circleX >= 550.0f) {
            dx *= -1.0f;
        }

        glClear(GL_COLOR_BUFFER_BIT);
        glUniform2f(circleLoc, circleX, circleY);
        glUniform1f(lineLoc, lineY);
        glUniform1i(intersectLoc, checkIntersection());

        glBindVertexArray(VAO);
        glDrawArrays(GL_TRIANGLES, 0, 6);

        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    glfwTerminate();
    return 0;
}