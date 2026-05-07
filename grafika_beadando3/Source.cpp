#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <GL/glu.h> 
#include <cmath>
#include <iostream>

// Az stb_image egyetlen fejlécbõl álló könyvtár képbetöltéshez
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

// Szükséges könyvtárak linkelése (Windows/Visual Studio esetén)
#pragma comment(lib, "opengl32.lib")
#pragma comment(lib, "glu32.lib")

// Globális ablakméretek
const int WINDOW_WIDTH = 800;
const int WINDOW_HEIGHT = 600;

// --- Kamera és világítás paraméterek ---
float r = 8.0f;           // A kamera távolsága az origótól (sugár)
float cameraAngle = 0.0f; // Vízszintes szög (forgás a Z-tengely körül)
float cameraZ = 0.0f;     // Kamera magassága

float lightAngle = 0.0f;   // A keringõ fényforrás aktuális szöge
bool lightEnabled = false; // Alapértelmezetten a világítás ki van kapcsolva
GLuint sunTexture;        // Itt tároljuk a GPU-n lévõ textúra azonosítóját

// Manuális kockarajzoló függvény: GL_QUADS-okat (négyszögeket) használ
void drawCube(float size) {
    float v = size / 2.0f; // A középponttól mért távolság a csúcsokig
    glBegin(GL_QUADS);

    // Minden laphoz megadjuk a normálvektort (glNormal3f) - ez elengedhetetlen a fényhatáshoz!

    // Felsõ lap (Z irány)
    glNormal3f(0.0f, 0.0f, 1.0f);
    glVertex3f(-v, -v, v); glVertex3f(v, -v, v); glVertex3f(v, v, v); glVertex3f(-v, v, v);

    // Alsó lap
    glNormal3f(0.0f, 0.0f, -1.0f);
    glVertex3f(-v, -v, -v); glVertex3f(-v, v, -v); glVertex3f(v, v, -v); glVertex3f(v, -v, -v);

    // Oldallapok (X és Y irányok)
    glNormal3f(1.0f, 0.0f, 0.0f);
    glVertex3f(v, -v, -v); glVertex3f(v, v, -v); glVertex3f(v, v, v); glVertex3f(v, -v, v);

    glNormal3f(-1.0f, 0.0f, 0.0f);
    glVertex3f(-v, -v, -v); glVertex3f(-v, -v, v); glVertex3f(-v, v, v); glVertex3f(-v, v, -v);

    glNormal3f(0.0f, 1.0f, 0.0f);
    glVertex3f(-v, v, -v); glVertex3f(-v, v, v); glVertex3f(v, v, v); glVertex3f(v, v, -v);

    glNormal3f(0.0f, -1.0f, 0.0f);
    glVertex3f(-v, -v, -v); glVertex3f(v, -v, -v); glVertex3f(v, -v, v); glVertex3f(-v, -v, v);
    glEnd();
}

// Textúra betöltése fájlból és feltöltése a videókártyára
void createSunTexture() {
    glGenTextures(1, &sunTexture);
    glBindTexture(GL_TEXTURE_2D, sunTexture);

    // Textúra mintavételezési beállítások (ha nagyobb/kisebb a felület, mint a kép)
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    int width, height, nrChannels;
    stbi_set_flip_vertically_on_load(true); // Az OpenGL alulról felfelé olvassa a képpontokat

    // Képfájl beolvasása
    unsigned char* data = stbi_load("images.jpeg", &width, &height, &nrChannels, 0);

    if (data) {
        // Formátum meghatározása (RGB vagy RGBA)
        GLenum format = (nrChannels == 4) ? GL_RGBA : GL_RGB;
        // Adatok átadása az OpenGL-nek
        glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, format, GL_UNSIGNED_BYTE, data);
    }
    else {
        std::cout << "HIBA: Nem sikerult betolteni a imgaes.jpeg texturat!" << std::endl;
    }

    stbi_image_free(data); // A RAM-ból törölhetjük, már a GPU-n van
}

void init() {
    glEnable(GL_DEPTH_TEST); // Mélységteszt: a takarásban lévõ dolgok ne látsszanak

    glEnable(GL_LIGHTING);   // Világítási rendszer aktiválása
    glEnable(GL_LIGHT0);      // 0-s számú fényforrás bekapcsolása
    glEnable(GL_NORMALIZE);  // Normálvektorok egységnyire hozása (fontos skálázáskor)

    // A fény színe (kicsit sárgás diffúz fény)
    GLfloat lightDiffuse[] = { 1.0f, 0.8f, 0.2f, 1.0f };
    glLightfv(GL_LIGHT0, GL_DIFFUSE, lightDiffuse);

    glClearColor(0.1f, 0.1f, 0.1f, 1.0f); // Sötétszürke háttérszín

    createSunTexture(); // Textúra elõkészítése
}

// Billentyûleütések kezelése
void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods) {
    if (action == GLFW_PRESS || action == GLFW_REPEAT) {
        float moveSpeed = 0.1f;
        switch (key) {
        case GLFW_KEY_LEFT:  cameraAngle -= moveSpeed; break;
        case GLFW_KEY_RIGHT: cameraAngle += moveSpeed; break;
        case GLFW_KEY_UP:    cameraZ += moveSpeed; break;
        case GLFW_KEY_DOWN:  cameraZ -= moveSpeed; break;
        case GLFW_KEY_L:
            if (action == GLFW_PRESS) lightEnabled = !lightEnabled; // Fény kapcsolgatása
            break;
        case GLFW_KEY_ESCAPE:
            glfwSetWindowShouldClose(window, GLFW_TRUE);
            break;
        }
    }
}

void display(GLFWwindow* window) {
    int width, height;
    glfwGetFramebufferSize(window, &width, &height);
    if (height == 0) height = 1;

    glViewport(0, 0, width, height);

    // Vetítési mátrix beállítása (Perspektivikus nézet)
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    float aspect = (float)width / (float)height;
    gluPerspective(55.0, aspect, 1.0, 100.0); // 55 fokos látószög

    // Modellnézeti mátrix (Tárgyak és kamera helyzete)
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT); // Képernyõ és Z-puffer törlése

    // 1. Kamera elhelyezése (Henger koordinátákból Kartéziusiba váltás)
    float camX = r * cos(cameraAngle);
    float camY = r * sin(cameraAngle);

    // gluLookAt: Honnan, Hova, Melyik irány a "FEL"
    gluLookAt(camX, camY, cameraZ,
        0.0, 0.0, 0.0,
        0.0, 0.0, 1.0); // Itt a Z-tengely mutat felfelé

    // Világítás aktuális állapotának alkalmazása
    if (lightEnabled) glEnable(GL_LIGHTING);
    else glDisable(GL_LIGHTING);

    // 2. A "NAP" (Fényforrás és gömb) kirajzolása
    glPushMatrix();
    float lightRadius = 2.0f * r; // A fény távolabb kering, mint a kamera
    float lX = lightRadius * cos(lightAngle);
    float lY = lightRadius * sin(lightAngle);

    // A fényforrás pozíciójának frissítése
    GLfloat lightPos[] = { lX, lY, 0.0f, 1.0f };
    glLightfv(GL_LIGHT0, GL_POSITION, lightPos);

    glTranslatef(lX, lY, 0.0f); // Elmozgatjuk a gömböt a fény helyére

    // Emisszió (övilágítás): a Nap akkor is sárga, ha nincs külsõ fény
    GLfloat sunEmission[] = { 1.0f, 0.8f, 0.2f, 1.0f };
    GLfloat noEmission[] = { 0.0f, 0.0f, 0.0f, 1.0f };
    glMaterialfv(GL_FRONT, GL_EMISSION, sunEmission);

    glEnable(GL_TEXTURE_2D);
    glBindTexture(GL_TEXTURE_2D, sunTexture);

    // GLU kvadrikus objektum a gömb rajzolásához
    GLUquadric* quad = gluNewQuadric();
    gluQuadricTexture(quad, GL_TRUE); // Textúra koordináták generálása a gömbre
    gluSphere(quad, 0.25, 32, 32);    // 0.25 sugarú gömb
    gluDeleteQuadric(quad);

    glDisable(GL_TEXTURE_2D);
    glMaterialfv(GL_FRONT, GL_EMISSION, noEmission); // Emisszió kikapcsolása a többieknek
    glPopMatrix();

    // 3. A három fehér kocka kirajzolása
    GLfloat cubeMaterial[] = { 1.0f, 1.0f, 1.0f, 1.0f };
    glMaterialfv(GL_FRONT, GL_AMBIENT_AND_DIFFUSE, cubeMaterial);

    // Középsõ kocka
    glPushMatrix();
    drawCube(1.0f);
    glPopMatrix();

    // Felsõ kocka
    glPushMatrix();
    glTranslatef(0.0f, 0.0f, 2.0f);
    drawCube(1.0f);
    glPopMatrix();

    // Alsó kocka
    glPushMatrix();
    glTranslatef(0.0f, 0.0f, -2.0f);
    drawCube(1.0f);
    glPopMatrix();
}

int main() {
    // Inicializálás és ablakkezelés...
    if (!glfwInit()) return -1;

    GLFWwindow* window = glfwCreateWindow(WINDOW_WIDTH, WINDOW_HEIGHT, "Kockak es Kamera", NULL, NULL);
    if (!window) {
        glfwTerminate();
        return -1;
    }

    glfwMakeContextCurrent(window);
    glewInit();
    glfwSetKeyCallback(window, key_callback);

    init();

    // Fõ renderelési ciklus
    while (!glfwWindowShouldClose(window)) {
        // A fény folyamatos keringtetése
        lightAngle += 0.01f;
        if (lightAngle > 2 * 3.14159) lightAngle -= 2 * 3.14159;

        display(window); // Rajzolás

        glfwSwapBuffers(window); // Buffer csere (double buffering)
        glfwPollEvents();        // Események (billentyûzet) lekérdezése
    }

    glfwTerminate();
    return 0;
}