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

// --- Configuração ---
const int NUM_FIOS = 50;

GLuint shaderProgram, VAO, VBO;

//Funções para Bezier
int fatorial(int n){ 
    int fact = 1; 
    for(int i = n; i > 0; i--) { 
        fact = i * fact; 
    } 
    return fact;
} 
    
float binomial(int n, int k) { 
    return (float)fatorial(n) / (fatorial(k) * fatorial(n-k)); 
}

struct Vec2 { float x, y; };

Vec2 controlPoints[4] = {
    {400.0f, 100.0f},  
    {420.0f, 300.0f},  
    {380.0f, 500.0f}, 
    {410.0f, 580.0f}   
};

int numPoints = 3; 

Vec2 bezier_point(float t, Vec2 P[], int n) {
    Vec2 result = {0.0f, 0.0f};
    for (int i = 0; i <= n; i++) {
        result.x += binomial(n, i) * pow((1 - t), (n - i)) * pow(t, i) * P[i].x;
        result.y += binomial(n, i) * pow((1 - t), (n - i)) * pow(t, i) * P[i].y;
    }

    // Normalização para coordenadas de clip space (-1 a 1)
    result.x = result.x / 800.0f * 2.0f - 1.0f;
    result.y = -(result.y / 600.0f * 2.0f - 1.0f);
    return result;
}

Vec2 array[100];

Vec2 apply_wind(Vec2 pt, float t, float time) {
    // Direção do vento
    float windX = -0.05f; // vento para trás
    float windY = 0.5f;   // ligeiro movimento para cima

    // Intensidade proporcional à ponta
    float factor = pow(t, 2.5f);

    // Movimento oscilante + vento constante
    float frequency = 2.0f;
    float amplitude_x = 0.05f * factor;
    float amplitude_y = 0.02f * factor;

    pt.x += amplitude_x * sin(frequency * time) + windX * factor;
    pt.y += amplitude_y * cos(frequency * time) + windY * factor;

    return pt;
}

// --- Inicialização ---
void Init() {
    //Criação e compilação de shaders 
    GLuint vertexShader = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vertexShader, 1, &vertexShaderSource, nullptr);
    glCompileShader(vertexShader);

    GLint success;
    GLchar infoLog[512];
    glGetShaderiv(vertexShader, GL_COMPILE_STATUS, &success);
    if (!success) {
        glGetShaderInfoLog(vertexShader, 512, NULL, infoLog);
        printf("Erro ao compilar Vertex Shader:\n%s\n", infoLog);
    }

    GLuint fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fragmentShader, 1, &fragmentShaderSource, nullptr);
    glCompileShader(fragmentShader);

    glGetShaderiv(fragmentShader, GL_COMPILE_STATUS, &success);
    if (!success) {
        glGetShaderInfoLog(fragmentShader, 512, NULL, infoLog);
        printf("Erro ao compilar Fragment Shader:\n%s\n", infoLog);
    }

    shaderProgram = glCreateProgram();
    glAttachShader(shaderProgram, vertexShader);
    glAttachShader(shaderProgram, fragmentShader);
    glLinkProgram(shaderProgram);

    glGetProgramiv(shaderProgram, GL_LINK_STATUS, &success);
    if (!success) {
        glGetProgramInfoLog(shaderProgram, 512, NULL, infoLog);
        printf("Erro ao linkar o Shader Program:\n%s\n", infoLog);
    }

    glDeleteShader(vertexShader);
    glDeleteShader(fragmentShader);

    //VAO e VBO
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

// --- Renderização ---
void Render() {
    glClear(GL_COLOR_BUFFER_BIT);

    float time = glutGet(GLUT_ELAPSED_TIME) / 1000.0f;
    glUseProgram(shaderProgram);

    GLint hairColorLoc = glGetUniformLocation(shaderProgram, "hairColor");

    for (int j = 0; j < NUM_FIOS; j++) {

        float depth = float(j) / NUM_FIOS;
        float scale = 1.0f - depth * 0.2f;
        float offsetX = (depth - 0.5f) * 0.3f;
        float phase = depth * 3.14f;

        //Cor do cabelo
        float baseR = 0.55f, baseG = 0.35f, baseB = 0.15f; // castanho claro
        float variation = 0.08f * ((j % 5) - 2); // pequenas variações
        float r = baseR + variation;
        float g = baseG + variation * 0.5f;
        float b = baseB + variation * 0.2f;
        // fios mais atrás mais escuros
        r *= 1.0f - depth * 0.4f;
        g *= 1.0f - depth * 0.4f;
        b *= 1.0f - depth * 0.4f;

        glUniform3f(hairColorLoc, r, g, b);

        // Calcular pontos do fio
        for (int i = 0; i < 100; i++) {
            float t = i / float(99);
            Vec2 pt = bezier_point(t, controlPoints, numPoints);
            pt.x = pt.x * scale + offsetX;
            pt.y = pt.y * scale;
            array[i] = apply_wind(pt, t, time + phase);
        }

        // Atualiza e desenha
        glBindVertexArray(VAO);
        glBindBuffer(GL_ARRAY_BUFFER, VBO);
        glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(array), array);
        glDrawArrays(GL_LINE_STRIP, 0, 100);
    }

    glBindVertexArray(0);
    glutSwapBuffers();
    glutPostRedisplay();
}


int main(int argc, char** argv) {
    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGBA);
    glutInitWindowSize(800, 600);

    glutInitContextVersion(3, 3);
    glutInitContextProfile(GLUT_CORE_PROFILE);
    glutCreateWindow("Cabelo ao vento com Shaders");

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
