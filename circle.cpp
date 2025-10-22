#include <GL/glew.h>
#include <GL/freeglut.h>
#include <cstdio>
#include <cmath>

// --- Shaders ---
const char* vertexShaderSource = R"(
#version 330 core
layout(location = 0) in vec2 aPos;

void main() {
    gl_Position = vec4(aPos, 0.0, 1.0);
}
)";

const char* fragmentShaderSource = R"(
#version 330 core
out vec4 FragColor;
uniform vec3 hairColor;

void main() {
    FragColor = vec4(hairColor, 1.0);
}
)";

const int NUM_FIOS = 120;
GLuint shaderProgram, VAO, VBO;

struct Vec2 { float x, y; };

// --- Bezier helpers ---
int fatorial(int n){ 
    int fact = 1; 
    for(int i = n; i > 0; i--) fact *= i; 
    return fact;
} 
    
float binomial(int n, int k) { 
    return (float)fatorial(n) / (fatorial(k) * fatorial(n-k)); 
}

Vec2 bezier_point(float t, Vec2 P[], int n) {
    Vec2 result = {0.0f, 0.0f};
    for (int i = 0; i <= n; i++) {
        result.x += binomial(n, i) * pow((1 - t), (n - i)) * pow(t, i) * P[i].x;
        result.y += binomial(n, i) * pow((1 - t), (n - i)) * pow(t, i) * P[i].y;
    }
    return result;
}

Vec2 apply_wind(Vec2 pt, float t, float time) {
    float windX = -0.05f; 
    float windY = 0.0f;
    float factor = pow(t, 2.5f);
    float frequency = 2.0f;
    float amplitude_x = 0.05f * factor;
    float amplitude_y = 0.02f * factor;
    pt.x += amplitude_x * sin(frequency * time) + windX * factor;
    pt.y += amplitude_y * cos(frequency * time) + windY * factor;
    return pt;
}

Vec2 array[100];

// --- Inicialização ---
void InitShaders() {
    GLuint vertexShader = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vertexShader, 1, &vertexShaderSource, nullptr);
    glCompileShader(vertexShader);

    GLuint fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fragmentShader, 1, &fragmentShaderSource, nullptr);
    glCompileShader(fragmentShader);

    shaderProgram = glCreateProgram();
    glAttachShader(shaderProgram, vertexShader);
    glAttachShader(shaderProgram, fragmentShader);
    glLinkProgram(shaderProgram);

    glDeleteShader(vertexShader);
    glDeleteShader(fragmentShader);
}

void Init() {
    InitShaders();

    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);
    glBindVertexArray(VAO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(array), array, GL_STATIC_DRAW);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, sizeof(Vec2), (void*)0);
    glEnableVertexAttribArray(0);
    glBindVertexArray(0);
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
}

// --- Render ---
void Render() {
    glClear(GL_COLOR_BUFFER_BIT);
    glUseProgram(shaderProgram);

    GLint hairColorLoc = glGetUniformLocation(shaderProgram, "hairColor");
    float time = glutGet(GLUT_ELAPSED_TIME) / 1000.0f;

    float radius = 0.4f; // raio da cabeça
    int numPoints = 3;

    for (int j = 0; j < NUM_FIOS; j++) {
        // ângulo ao redor do círculo
        float angle = (2.0f * M_PI * j) / NUM_FIOS;
        float depth = float(j) / NUM_FIOS;

        // posição base (cabeça)
        Vec2 P[4];
        P[0] = { radius * cosf(angle), radius * sinf(angle) }; // ponto inicial na borda

        // os próximos pontos caem para baixo (direção -y)
        for (int i = 1; i <= 3; i++) {
            P[i].x = P[0].x + 0.05f * sin(angle + 0.5f); // ligeira curvatura
            P[i].y = P[0].y - 0.25f * i; // cada ponto mais para baixo
        }

        // cor do fio
        float baseR = 0.55f, baseG = 0.35f, baseB = 0.15f;
        float variation = 0.1f * sin(j * 0.4f);
        float r = baseR + variation;
        float g = baseG + variation * 0.5f;
        float b = baseB + variation * 0.2f;
        r *= 1.0f - depth * 0.4f;
        g *= 1.0f - depth * 0.4f;
        b *= 1.0f - depth * 0.4f;
        glUniform3f(hairColorLoc, r, g, b);

        // gerar pontos bezier
        for (int i = 0; i < 100; i++) {
            float t = i / float(99);
            Vec2 pt = bezier_point(t, P, numPoints);
            pt = apply_wind(pt, t, time);
            array[i] = pt;
        }

        // desenhar
        glBindVertexArray(VAO);
        glBindBuffer(GL_ARRAY_BUFFER, VBO);
        glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(array), array);
        glDrawArrays(GL_LINE_STRIP, 0, 100);
    }

    glBindVertexArray(0);
    glutSwapBuffers();
    glutPostRedisplay();
}

// --- Main ---
int main(int argc, char** argv) {
    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGBA);
    glutInitWindowSize(800, 600);
    glutInitContextVersion(3, 3);
    glutInitContextProfile(GLUT_CORE_PROFILE);
    glutCreateWindow("Cabelo em círculo ao vento");

    glewExperimental = GL_TRUE;
    if (glewInit() != GLEW_OK) {
        printf("Erro ao inicializar GLEW\n");
        return -1;
    }

    Init();
    glutDisplayFunc(Render);
    glutMainLoop();
    return 0;
}
