#include <stdio.h>
#include <emscripten.h>
#include <emscripten/html5.h>
#include <GLES2/gl2.h>
#include <stdlib.h>

GLuint shaderProgram;
GLuint positionBuffer;
GLint positionAttributeLocation;
GLint timeUniformLocation;

const char *vertexShaderSource = 
    "attribute vec4 a_position;\n"
    "void main() {\n"
    "  gl_Position = a_position;\n"
    "}\n";

const char *fragmentShaderSource = 
    "precision highp float;\n"
    "uniform float u_time;\n"
    "void main() {\n"
    "    vec2 c = (gl_FragCoord.xy / vec2(800.0, 600.0)) * 4.0 - vec2(2.0, 2.0);\n"
    "    vec2 z = c;\n"
    "    float iter = 0.0;\n"
    "    const float max_iter = 100.0;\n"
    "    for(float i = 0.0; i < max_iter; i++) {\n"
    "        if(z.x*z.x + z.y*z.y > 4.0) break;\n"
    "        z = vec2(z.x*z.x - z.y*z.y, 2.0*z.x*z.y) + c;\n"
    "        iter++;\n"
    "    }\n"
    "    float color = iter / max_iter;\n"
    "    gl_FragColor = vec4(color * abs(sin(u_time)), color * 0.5, color, 1.0);\n"
    "}\n";

GLuint compileShader(GLenum type, const char *source) {
    GLuint shader = glCreateShader(type);
    glShaderSource(shader, 1, &source, NULL);
    glCompileShader(shader);
    
    GLint success;
    glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
    if (!success) {
        char infoLog[512];
        glGetShaderInfoLog(shader, 512, NULL, infoLog);
        printf("ERROR::SHADER::COMPILATION_FAILED\n%s\n", infoLog);
    }
    return shader;
}

void initWebGL() {
    EmscriptenWebGLContextAttributes attr;
    emscripten_webgl_init_context_attributes(&attr);
    attr.alpha = 0;
    attr.depth = 0;
    attr.stencil = 0;
    attr.antialias = 0;
    
    EMSCRIPTEN_WEBGL_CONTEXT_HANDLE ctx = emscripten_webgl_create_context("#canvas", &attr);
    emscripten_webgl_make_context_current(ctx);
    
    GLuint vertexShader = compileShader(GL_VERTEX_SHADER, vertexShaderSource);
    GLuint fragmentShader = compileShader(GL_FRAGMENT_SHADER, fragmentShaderSource);
    
    shaderProgram = glCreateProgram();
    glAttachShader(shaderProgram, vertexShader);
    glAttachShader(shaderProgram, fragmentShader);
    glLinkProgram(shaderProgram);
    
    positionAttributeLocation = glGetAttribLocation(shaderProgram, "a_position");
    timeUniformLocation = glGetUniformLocation(shaderProgram, "u_time");
    
    float positions[] = {
        -1.0f, -1.0f,
         1.0f, -1.0f,
        -1.0f,  1.0f,
        -1.0f,  1.0f,
         1.0f, -1.0f,
         1.0f,  1.0f,
    };
    
    glGenBuffers(1, &positionBuffer);
    glBindBuffer(GL_ARRAY_BUFFER, positionBuffer);
    glBufferData(GL_ARRAY_BUFFER, sizeof(positions), positions, GL_STATIC_DRAW);
}

int frameCount = 0;
double startTime = 0;

void renderFrame() {
    double now = emscripten_get_now();
    
    glViewport(0, 0, 800, 600);
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);
    
    glUseProgram(shaderProgram);
    glUniform1f(timeUniformLocation, now / 1000.0f);
    
    glEnableVertexAttribArray(positionAttributeLocation);
    glBindBuffer(GL_ARRAY_BUFFER, positionBuffer);
    glVertexAttribPointer(positionAttributeLocation, 2, GL_FLOAT, GL_FALSE, 0, 0);
    
    glDrawArrays(GL_TRIANGLES, 0, 6);
    
    frameCount++;
    if (now - startTime >= 1000.0) {
        double fps = frameCount * 1000.0 / (now - startTime);
        EM_ASM_({
            updateFPS($0);
        }, fps);
        frameCount = 0;
        startTime = now;
    }
}

int main() {
    printf("Initializing WebAssembly WebGL Mandelbrot Benchmark...\n");
    initWebGL();
    startTime = emscripten_get_now();
    emscripten_set_main_loop(renderFrame, 0, 1);
    return 0;
}
