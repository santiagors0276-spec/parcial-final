#include <iostream>
#include <vector>

#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

// ============================================
// CAMARA
// ============================================
struct Camara {

    glm::vec3 pos = glm::vec3(0.f, 1.f, 8.f);
    glm::vec3 frente = glm::vec3(0.f, 0.f, -1.f);
    glm::vec3 arriba = glm::vec3(0.f, 1.f, 0.f);

    float yaw = -90.f;
    float pitch = 0.f;

    float ultX = 0.f;
    float ultY = 0.f;

    bool primerMouse = true;
};

Camara cam;

// ============================================
// SHADERS
// ============================================
const char* vertObj =
"#version 330 core\n"
"layout(location=0) in vec3 pos;\n"
"layout(location=1) in vec2 uv;\n"

"out vec2 texCoord;\n"

"uniform mat4 mvp;\n"

"void main(){\n"
"   gl_Position = mvp * vec4(pos,1.0);\n"
"   texCoord = uv;\n"
"}\0";

const char* fragObj =
"#version 330 core\n"

"in vec2 texCoord;\n"
"out vec4 colorFinal;\n"

"uniform sampler2D tex;\n"

"void main(){\n"

"   vec4 textura = texture(tex, texCoord);\n"

"   vec4 azul = vec4(0.1,0.5,1.0,1.0);\n"

"   colorFinal = mix(textura, azul, 0.5);\n"
"}\0";

const char* vertLinea =
"#version 330 core\n"

"layout(location=0) in vec3 pos;\n"

"uniform mat4 vp;\n"

"void main(){\n"
"   gl_Position = vp * vec4(pos,1.0);\n"
"}\0";

const char* fragLinea =
"#version 330 core\n"

"out vec4 colorFinal;\n"

"uniform vec3 color;\n"

"void main(){\n"
"   colorFinal = vec4(color,1.0);\n"
"}\0";

// ============================================
// CALLBACKS
// ============================================
void resize(GLFWwindow* w, int ancho, int alto) {

    glViewport(0, 0, ancho, alto);
}

void mouse(GLFWwindow* w, double x, double y) {

    if (cam.primerMouse) {

        cam.ultX = (float)x;
        cam.ultY = (float)y;

        cam.primerMouse = false;
    }

    float dx = (float)x - cam.ultX;
    float dy = cam.ultY - (float)y;

    cam.ultX = (float)x;
    cam.ultY = (float)y;

    float sens = 0.08f;

    cam.yaw += dx * sens;
    cam.pitch += dy * sens;

    cam.pitch = glm::clamp(cam.pitch, -89.f, 89.f);

    glm::vec3 dir;

    dir.x =
        cos(glm::radians(cam.yaw))
        * cos(glm::radians(cam.pitch));

    dir.y =
        sin(glm::radians(cam.pitch));

    dir.z =
        sin(glm::radians(cam.yaw))
        * cos(glm::radians(cam.pitch));

    cam.frente = glm::normalize(dir);
}

// ============================================
// SHADER PROGRAM
// ============================================
GLuint crearPrograma(const char* vs, const char* fs) {

    GLuint vert = glCreateShader(GL_VERTEX_SHADER);

    glShaderSource(vert, 1, &vs, NULL);
    glCompileShader(vert);

    GLuint frag = glCreateShader(GL_FRAGMENT_SHADER);

    glShaderSource(frag, 1, &fs, NULL);
    glCompileShader(frag);

    GLuint prog = glCreateProgram();

    glAttachShader(prog, vert);
    glAttachShader(prog, frag);

    glLinkProgram(prog);

    glDeleteShader(vert);
    glDeleteShader(frag);

    return prog;
}

// ============================================
// BEZIER
// ============================================
glm::vec3 bezier(
    glm::vec3 A,
    glm::vec3 B,
    glm::vec3 C,
    glm::vec3 D,
    float t
) {

    float u = 1.f - t;

    return
        u * u * u * A +
        3 * u * u * t * B +
        3 * u * t * t * C +
        t * t * t * D;
}

std::vector<glm::vec3> generarCurva(
    glm::vec3 A,
    glm::vec3 B,
    glm::vec3 C,
    glm::vec3 D,
    int seg
) {

    std::vector<glm::vec3> puntos;

    for (int i = 0; i <= seg; i++) {

        float t = (float)i / seg;

        puntos.push_back(
            bezier(A, B, C, D, t)
        );
    }

    return puntos;
}

// ============================================
// MAIN
// ============================================
int main() {

    glfwInit();

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);

    glfwWindowHint(
        GLFW_OPENGL_PROFILE,
        GLFW_OPENGL_CORE_PROFILE
    );

    GLFWwindow* ventana =
        glfwCreateWindow(
            1280,
            720,
            "Piramide",
            NULL,
            NULL
        );

    glfwMakeContextCurrent(ventana);

    glfwSetFramebufferSizeCallback(
        ventana,
        resize
    );

    glfwSetCursorPosCallback(
        ventana,
        mouse
    );

    glfwSetInputMode(
        ventana,
        GLFW_CURSOR,
        GLFW_CURSOR_DISABLED
    );

    gladLoadGL();

    glEnable(GL_DEPTH_TEST);

    // ========================================
    // PROGRAMAS
    // ========================================
    GLuint progObj =
        crearPrograma(
            vertObj,
            fragObj
        );

    GLuint progLinea =
        crearPrograma(
            vertLinea,
            fragLinea
        );

    // ========================================
    // PIRAMIDE
    // ========================================
    struct Vertice {

        float pos[3];
        float uv[2];
    };

    std::vector<Vertice> verts = {

        {{-1.f,-1.f,-1.f},{0.f,0.f}},
        {{ 1.f,-1.f,-1.f},{1.f,0.f}},
        {{ 1.f,-1.f, 1.f},{1.f,1.f}},
        {{-1.f,-1.f, 1.f},{0.f,1.f}},

        {{0.f,1.5f,0.f},{0.5f,0.5f}}
    };

    std::vector<GLuint> idx = {

        0,1,2,
        0,2,3,

        0,1,4,
        1,2,4,
        2,3,4,
        3,0,4
    };

    GLuint vao, vbo, ebo;

    glGenVertexArrays(1, &vao);
    glGenBuffers(1, &vbo);
    glGenBuffers(1, &ebo);

    glBindVertexArray(vao);

    glBindBuffer(GL_ARRAY_BUFFER, vbo);

    glBufferData(
        GL_ARRAY_BUFFER,
        verts.size() * sizeof(Vertice),
        verts.data(),
        GL_STATIC_DRAW
    );

    glBindBuffer(
        GL_ELEMENT_ARRAY_BUFFER,
        ebo
    );

    glBufferData(
        GL_ELEMENT_ARRAY_BUFFER,
        idx.size() * sizeof(GLuint),
        idx.data(),
        GL_STATIC_DRAW
    );

    glVertexAttribPointer(
        0,
        3,
        GL_FLOAT,
        GL_FALSE,
        sizeof(Vertice),
        (void*)0
    );

    glEnableVertexAttribArray(0);

    glVertexAttribPointer(
        1,
        2,
        GL_FLOAT,
        GL_FALSE,
        sizeof(Vertice),
        (void*)(offsetof(Vertice, uv))
    );

    glEnableVertexAttribArray(1);

    // ========================================
    // TEXTURA
    // ========================================
    GLuint textura;

    glGenTextures(1, &textura);

    glBindTexture(GL_TEXTURE_2D, textura);

    int ancho, alto, canales;

    unsigned char* img =
        stbi_load(
            "textura.jpg",
            &ancho,
            &alto,
            &canales,
            0
        );

    if (img) {

        glTexImage2D(
            GL_TEXTURE_2D,
            0,
            GL_RGB,
            ancho,
            alto,
            0,
            GL_RGB,
            GL_UNSIGNED_BYTE,
            img
        );

        glGenerateMipmap(GL_TEXTURE_2D);
    }

    stbi_image_free(img);

    // ========================================
    // CURVA NUEVA
    // ========================================
    glm::vec3 P0(-7.f, -1.f, -4.f);
    glm::vec3 P1(-2.f, 5.f, 4.f);
    glm::vec3 P2(3.f, -5.f, -3.f);
    glm::vec3 P3(7.f, 1.f, 5.f);

    auto puntos =
        generarCurva(
            P0, P1, P2, P3, 300
        );

    GLuint vaoLinea, vboLinea;

    glGenVertexArrays(1, &vaoLinea);
    glGenBuffers(1, &vboLinea);

    glBindVertexArray(vaoLinea);

    glBindBuffer(GL_ARRAY_BUFFER, vboLinea);

    glBufferData(
        GL_ARRAY_BUFFER,
        puntos.size() * sizeof(glm::vec3),
        puntos.data(),
        GL_STATIC_DRAW
    );

    glVertexAttribPointer(
        0,
        3,
        GL_FLOAT,
        GL_FALSE,
        sizeof(glm::vec3),
        (void*)0
    );

    glEnableVertexAttribArray(0);

    // ========================================
    // ANIMACION
    // ========================================
    float t = 0.f;

    // ========================================
    // LOOP
    // ========================================
    while (!glfwWindowShouldClose(ventana)) {

        const float vel = 0.04f;

        glm::vec3 lateral =
            glm::normalize(
                glm::cross(
                    cam.frente,
                    cam.arriba
                )
            );

        if (glfwGetKey(ventana, GLFW_KEY_ESCAPE)
            == GLFW_PRESS)
        {
            glfwSetWindowShouldClose(
                ventana,
                true
            );
        }

        if (glfwGetKey(ventana, GLFW_KEY_W)
            == GLFW_PRESS)
            cam.pos += cam.frente * vel;

        if (glfwGetKey(ventana, GLFW_KEY_S)
            == GLFW_PRESS)
            cam.pos -= cam.frente * vel;

        if (glfwGetKey(ventana, GLFW_KEY_A)
            == GLFW_PRESS)
            cam.pos -= lateral * vel;

        if (glfwGetKey(ventana, GLFW_KEY_D)
            == GLFW_PRESS)
            cam.pos += lateral * vel;

        // ====================================
        // ANIMACION
        // ====================================
        t += 0.0008f;

        if (t > 1.f)
            t = 0.f;

        glm::vec3 posObj =
            bezier(
                P0, P1, P2, P3, t
            );

        // ====================================
        // LIMPIAR
        // ====================================
        glClearColor(
            0.04f,
            0.04f,
            0.08f,
            1.f
        );

        glClear(
            GL_COLOR_BUFFER_BIT |
            GL_DEPTH_BUFFER_BIT
        );

        // ====================================
        // MATRICES
        // ====================================
        int w, h;

        glfwGetFramebufferSize(
            ventana,
            &w,
            &h
        );

        glm::mat4 proj =
            glm::perspective(
                glm::radians(60.f),
                (float)w / h,
                0.1f,
                100.f
            );

        glm::mat4 view =
            glm::lookAt(
                cam.pos,
                cam.pos + cam.frente,
                cam.arriba
            );

        glm::mat4 vp =
            proj * view;

        // ====================================
        // CURVA
        // ====================================
        glUseProgram(progLinea);

        glUniformMatrix4fv(
            glGetUniformLocation(
                progLinea,
                "vp"
            ),
            1,
            GL_FALSE,
            glm::value_ptr(vp)
        );

        glUniform3f(
            glGetUniformLocation(
                progLinea,
                "color"
            ),
            1.f,
            0.4f,
            0.2f
        );

        glBindVertexArray(vaoLinea);

        glDrawArrays(
            GL_LINE_STRIP,
            0,
            (GLsizei)puntos.size()
        );

        // ====================================
        // PIRAMIDE
        // ====================================
        glm::mat4 model =
            glm::translate(
                glm::mat4(1.f),
                posObj
            );

        model = glm::rotate(
            model,
            (float)glfwGetTime(),
            glm::vec3(1.f, 1.f, 0.f)
        );

        glm::mat4 mvp =
            vp * model;

        glUseProgram(progObj);

        glBindTexture(
            GL_TEXTURE_2D,
            textura
        );

        glUniformMatrix4fv(
            glGetUniformLocation(
                progObj,
                "mvp"
            ),
            1,
            GL_FALSE,
            glm::value_ptr(mvp)
        );

        glBindVertexArray(vao);

        glDrawElements(
            GL_TRIANGLES,
            (GLsizei)idx.size(),
            GL_UNSIGNED_INT,
            0
        );

        glfwSwapBuffers(ventana);

        glfwPollEvents();
    }

    glfwTerminate();

    return 0;
}