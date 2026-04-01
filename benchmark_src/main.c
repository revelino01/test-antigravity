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
    "vec2 map(vec3 p) {\n"
    "    vec3 w = p;\n"
    "    float m = dot(w, w);\n"
    "    vec4 trap = vec4(abs(w), m);\n"
    "    float dz = 1.0;\n"
    "    for(int i = 0; i < 4; i++) {\n"
    "        m = dot(w, w);\n"
    "        if(m > 256.0) break;\n"
    "        float r = length(w);\n"
    "        float b = 8.0 * acos(w.y / r);\n"
    "        float a = 8.0 * atan(w.x, w.z);\n"
    "        dz = 8.0 * pow(r, 7.0) * dz + 1.0;\n"
    "        r = pow(r, 8.0);\n"
    "        w = p + r * vec3(sin(b) * sin(a), cos(b), sin(b) * cos(a));\n"
    "        trap = min(trap, vec4(abs(w), m));\n"
    "    }\n"
    "    return vec2(0.25 * log(m) * sqrt(m) / dz, trap.y);\n"
    "}\n"
    "void main() {\n"
    "    vec2 p = -1.0 + 2.0 * gl_FragCoord.xy / vec2(800.0, 600.0);\n"
    "    p.x *= 800.0 / 600.0;\n"
    "    float ro_time = u_time * 0.2;\n"
    "    vec3 ro = vec3(2.5 * sin(ro_time), 0.0, 2.5 * cos(ro_time));\n"
    "    vec3 ta = vec3(0.0);\n"
    "    vec3 cw = normalize(ta - ro);\n"
    "    vec3 cu = normalize(cross(cw, vec3(0.0, 1.0, 0.0)));\n"
    "    vec3 cv = normalize(cross(cu, cw));\n"
    "    vec3 rd = normalize(p.x * cu + p.y * cv + 1.5 * cw);\n"
    "    float tmax = 20.0;\n"
    "    float t = 0.0;\n"
    "    for(int i = 0; i < 150; i++) {\n"
    "        vec3 pos = ro + rd * t;\n"
    "        vec2 h = map(pos);\n"
    "        if(h.x < 0.001 || t > tmax) break;\n"
    "        t += h.x;\n"
    "    }\n"
    "    vec3 col = vec3(0.0);\n"
    "    if(t < tmax) {\n"
    "        vec3 pos = ro + rd * t;\n"
    "        vec2 h = map(pos);\n"
    "        vec2 e = vec2(0.001, 0.0);\n"
    "        vec3 nor = normalize(vec3(\n"
    "            map(pos + e.xyy).x - map(pos - e.xyy).x,\n"
    "            map(pos + e.yxy).x - map(pos - e.yxy).x,\n"
    "            map(pos + e.yyx).x - map(pos - e.yyx).x\n"
    "        ));\n"
    "        vec3 light = normalize(vec3(1.0, 0.9, 0.3));\n"
    "        float dif = clamp(dot(nor, light), 0.0, 1.0);\n"
    "        float amb = 0.5 + 0.5 * dot(nor, vec3(0.0, 1.0, 0.0));\n"
    "        col = vec3(0.2, 0.3, 0.4) * amb + vec3(1.0, 0.9, 0.7) * dif;\n"
    "        col *= mix(vec3(1.0), vec3(0.0, 0.5, 1.0), h.y * 2.0);\n"
    "        col = mix(col, vec3(0.0), 1.0 - exp(-0.05 * t));\n"
    "    }\n"
    "    col = pow(col, vec3(0.4545));\n"
    "    gl_FragColor = vec4(col, 1.0);\n"
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
