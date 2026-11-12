#include <GLES2/gl2.h>
#include <android/log.h>
#include <jni.h>

#include <algorithm>
#include <cmath>
#include <cstddef>
#include <vector>

namespace {

constexpr char kLogTag[] = "ValleyCity3D";
constexpr float kPi = 3.14159265358979323846f;

struct Vertex {
    float x;
    float y;
    float z;
    float r;
    float g;
    float b;
};

struct Point {
    float x;
    float y;
    float z;
};

GLuint program = 0;
GLuint vertexBuffer = 0;
GLint positionLocation = -1;
GLint colorLocation = -1;
GLint matrixLocation = -1;
GLsizei vertexCount = 0;
std::vector<Vertex> vertices;
float projection[16] = {};
float viewportAspect = 1.0f;
float cameraYaw = 0.65f;
float cameraPitch = 0.62f;
float cameraDistance = 50.0f;

void logError(const char* message) {
    __android_log_print(ANDROID_LOG_ERROR, kLogTag, "%s", message);
}

void appendTriangle(const Point& a, const Point& b, const Point& c,
                    float r, float g, float blue) {
    vertices.push_back({a.x, a.y, a.z, r, g, blue});
    vertices.push_back({b.x, b.y, b.z, r, g, blue});
    vertices.push_back({c.x, c.y, c.z, r, g, blue});
}

void appendQuad(const Point& a, const Point& b, const Point& c, const Point& d,
                float r, float g, float blue) {
    appendTriangle(a, b, c, r, g, blue);
    appendTriangle(a, c, d, r, g, blue);
}

void appendBox(float x0, float x1, float z0, float z1,
               float y0, float y1, float r, float g, float blue) {
    const Point a{x0, y0, z0};
    const Point b{x1, y0, z0};
    const Point c{x1, y0, z1};
    const Point d{x0, y0, z1};
    const Point e{x0, y1, z0};
    const Point f{x1, y1, z0};
    const Point h{x1, y1, z1};
    const Point i{x0, y1, z1};

    appendQuad(a, d, c, b, r * 0.62f, g * 0.62f, blue * 0.62f);
    appendQuad(e, f, h, i, r, g, blue);
    appendQuad(a, b, f, e, r * 0.78f, g * 0.78f, blue * 0.78f);
    appendQuad(b, c, h, f, r * 0.70f, g * 0.70f, blue * 0.70f);
    appendQuad(c, d, i, h, r * 0.86f, g * 0.86f, blue * 0.86f);
    appendQuad(d, a, e, i, r * 0.74f, g * 0.74f, blue * 0.74f);
}

void appendRoof(float x0, float x1, float z0, float z1, float baseY,
                float roofY, float r, float g, float blue) {
    const float midX = (x0 + x1) * 0.5f;
    const Point frontLeft{x0, baseY, z0};
    const Point frontRight{x1, baseY, z0};
    const Point backRight{x1, baseY, z1};
    const Point backLeft{x0, baseY, z1};
    const Point ridgeFront{midX, roofY, z0};
    const Point ridgeBack{midX, roofY, z1};

    appendTriangle(frontLeft, ridgeFront, frontRight, r, g, blue);
    appendTriangle(backRight, ridgeBack, backLeft, r, g, blue);
    appendQuad(frontLeft, backLeft, ridgeBack, ridgeFront,
               r * 0.78f, g * 0.78f, blue * 0.78f);
    appendQuad(ridgeFront, ridgeBack, backRight, frontRight,
               r * 0.88f, g * 0.88f, blue * 0.88f);
}

void appendGroundQuad(float x0, float x1, float z0, float z1, float y,
                      float r, float g, float blue) {
    appendQuad({x0, y, z0}, {x1, y, z0}, {x1, y, z1}, {x0, y, z1},
               r, g, blue);
}

void appendHouse(float x, float z, float width, float depth, float height,
                 float r, float g, float blue) {
    appendBox(x, x + width, z, z + depth, 0.08f, height, r, g, blue);
    appendRoof(x - 0.25f, x + width + 0.25f, z - 0.25f, z + depth + 0.25f,
               height, height + 1.8f, 0.55f, 0.20f, 0.16f);
}

void appendBuilding(float x, float z, float width, float depth, float height,
                    float r, float g, float blue) {
    appendBox(x, x + width, z, z + depth, 0.08f, height, r, g, blue);
    appendGroundQuad(x - 0.35f, x + width + 0.35f,
                     z - 0.35f, z + depth + 0.35f, height + 0.02f,
                     r * 0.82f, g * 0.82f, blue * 0.82f);
}

void buildCityGeometry() {
    vertices.clear();
    appendGroundQuad(-70.0f, 70.0f, -70.0f, 70.0f, 0.0f,
                     0.20f, 0.48f, 0.24f);

    constexpr float roadHalfWidth = 3.2f;
    constexpr float sidewalkOuter = 5.2f;
    constexpr float sidewalkInner = 3.2f;
    constexpr float sidewalkHeight = 0.055f;

    for (const float center : {-24.0f, 0.0f, 24.0f}) {
        appendGroundQuad(-70.0f, 70.0f,
                         center - sidewalkOuter, center - sidewalkInner,
                         sidewalkHeight, 0.68f, 0.68f, 0.64f);
        appendGroundQuad(-70.0f, 70.0f,
                         center + sidewalkInner, center + sidewalkOuter,
                         sidewalkHeight, 0.68f, 0.68f, 0.64f);
        appendGroundQuad(center - sidewalkOuter, center - sidewalkInner,
                         -70.0f, 70.0f, sidewalkHeight,
                         0.68f, 0.68f, 0.64f);
        appendGroundQuad(center + sidewalkInner, center + sidewalkOuter,
                         -70.0f, 70.0f, sidewalkHeight,
                         0.68f, 0.68f, 0.64f);
    }

    for (const float center : {-24.0f, 0.0f, 24.0f}) {
        appendGroundQuad(-70.0f, 70.0f,
                         center - roadHalfWidth, center + roadHalfWidth,
                         0.075f, 0.16f, 0.17f, 0.19f);
        appendGroundQuad(center - roadHalfWidth, center + roadHalfWidth,
                         -70.0f, 70.0f, 0.075f, 0.16f, 0.17f, 0.19f);

        for (float mark = -64.0f; mark < 64.0f; mark += 8.0f) {
            appendGroundQuad(mark, mark + 3.0f, center - 0.08f, center + 0.08f,
                             0.09f, 0.94f, 0.78f, 0.24f);
            appendGroundQuad(center - 0.08f, center + 0.08f,
                             mark, mark + 3.0f, 0.09f,
                             0.94f, 0.78f, 0.24f);
        }
    }

    for (int stripe = 0; stripe < 7; ++stripe) {
        const float offset = -2.7f + stripe * 0.9f;
        appendGroundQuad(offset, offset + 0.42f, -6.1f, -3.4f, 0.095f,
                         0.92f, 0.92f, 0.88f);
        appendGroundQuad(offset, offset + 0.42f, 3.4f, 6.1f, 0.095f,
                         0.92f, 0.92f, 0.88f);
        appendGroundQuad(-6.1f, -3.4f, offset, offset + 0.42f, 0.095f,
                         0.92f, 0.92f, 0.88f);
        appendGroundQuad(3.4f, 6.1f, offset, offset + 0.42f, 0.095f,
                         0.92f, 0.92f, 0.88f);
    }

    appendHouse(-20.0f, -20.0f, 8.0f, 7.0f, 4.0f, 0.82f, 0.72f, 0.53f);
    appendHouse(-10.0f, -19.0f, 6.5f, 8.0f, 3.5f, 0.80f, 0.80f, 0.68f);
    appendBuilding(7.5f, -20.0f, 10.0f, 8.5f, 12.0f, 0.64f, 0.70f, 0.78f);
    appendHouse(10.0f, -9.0f, 7.0f, 7.0f, 4.5f, 0.82f, 0.62f, 0.45f);
    appendBuilding(-20.0f, 8.0f, 8.5f, 10.0f, 16.0f, 0.68f, 0.72f, 0.78f);
    appendHouse(-9.0f, 10.0f, 7.0f, 8.0f, 3.8f, 0.76f, 0.75f, 0.60f);
    appendHouse(8.0f, 8.0f, 8.0f, 8.0f, 4.2f, 0.77f, 0.64f, 0.50f);
    appendBuilding(8.0f, 19.0f, 10.0f, 7.0f, 10.0f, 0.72f, 0.69f, 0.61f);

    vertexCount = static_cast<GLsizei>(vertices.size());
    glGenBuffers(1, &vertexBuffer);
    glBindBuffer(GL_ARRAY_BUFFER, vertexBuffer);
    glBufferData(GL_ARRAY_BUFFER,
                 static_cast<GLsizeiptr>(vertices.size() * sizeof(Vertex)),
                 vertices.data(), GL_STATIC_DRAW);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
}

GLuint compileShader(GLenum type, const char* source) {
    const GLuint shader = glCreateShader(type);
    glShaderSource(shader, 1, &source, nullptr);
    glCompileShader(shader);

    GLint compiled = GL_FALSE;
    glGetShaderiv(shader, GL_COMPILE_STATUS, &compiled);
    if (compiled == GL_FALSE) {
        GLint logLength = 0;
        glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &logLength);
        std::vector<char> log(static_cast<size_t>(std::max(logLength, 1)));
        glGetShaderInfoLog(shader, logLength, nullptr, log.data());
        __android_log_print(ANDROID_LOG_ERROR, kLogTag,
                            "Shader compilation failed: %s", log.data());
        glDeleteShader(shader);
        return 0;
    }
    return shader;
}

GLuint createProgram() {
    constexpr char vertexSource[] =
        "attribute vec3 aPosition;\n"
        "attribute vec3 aColor;\n"
        "uniform mat4 uMvp;\n"
        "varying vec3 vColor;\n"
        "void main() {\n"
        "  gl_Position = uMvp * vec4(aPosition, 1.0);\n"
        "  vColor = aColor;\n"
        "}\n";
    constexpr char fragmentSource[] =
        "precision mediump float;\n"
        "varying vec3 vColor;\n"
        "void main() {\n"
        "  gl_FragColor = vec4(vColor, 1.0);\n"
        "}\n";

    const GLuint vertexShader = compileShader(GL_VERTEX_SHADER, vertexSource);
    const GLuint fragmentShader = compileShader(GL_FRAGMENT_SHADER, fragmentSource);
    if (vertexShader == 0 || fragmentShader == 0) {
        if (vertexShader != 0) glDeleteShader(vertexShader);
        if (fragmentShader != 0) glDeleteShader(fragmentShader);
        return 0;
    }

    const GLuint shaderProgram = glCreateProgram();
    glAttachShader(shaderProgram, vertexShader);
    glAttachShader(shaderProgram, fragmentShader);
    glBindAttribLocation(shaderProgram, 0, "aPosition");
    glBindAttribLocation(shaderProgram, 1, "aColor");
    glLinkProgram(shaderProgram);
    glDeleteShader(vertexShader);
    glDeleteShader(fragmentShader);

    GLint linked = GL_FALSE;
    glGetProgramiv(shaderProgram, GL_LINK_STATUS, &linked);
    if (linked == GL_FALSE) {
        GLint logLength = 0;
        glGetProgramiv(shaderProgram, GL_INFO_LOG_LENGTH, &logLength);
        std::vector<char> log(static_cast<size_t>(std::max(logLength, 1)));
        glGetProgramInfoLog(shaderProgram, logLength, nullptr, log.data());
        __android_log_print(ANDROID_LOG_ERROR, kLogTag,
                            "Program linking failed: %s", log.data());
        glDeleteProgram(shaderProgram);
        return 0;
    }

    return shaderProgram;
}

void multiplyMatrices(const float* left, const float* right, float* result) {
    float temporary[16] = {};
    for (int column = 0; column < 4; ++column) {
        for (int row = 0; row < 4; ++row) {
            for (int index = 0; index < 4; ++index) {
                temporary[column * 4 + row] +=
                    left[index * 4 + row] * right[column * 4 + index];
            }
        }
    }
    std::copy(std::begin(temporary), std::end(temporary), result);
}

void normalize(float& x, float& y, float& z) {
    const float length = std::sqrt(x * x + y * y + z * z);
    if (length > 0.0f) {
        x /= length;
        y /= length;
        z /= length;
    }
}

void makeViewMatrix(float* matrix) {
    const float horizontalDistance = cameraDistance * std::cos(cameraPitch);
    const float eyeX = horizontalDistance * std::sin(cameraYaw);
    const float eyeY = cameraDistance * std::sin(cameraPitch) + 1.0f;
    const float eyeZ = horizontalDistance * std::cos(cameraYaw);

    float forwardX = -eyeX;
    float forwardY = -eyeY;
    float forwardZ = -eyeZ;
    normalize(forwardX, forwardY, forwardZ);

    float sideX = -forwardZ;
    float sideY = 0.0f;
    float sideZ = forwardX;
    normalize(sideX, sideY, sideZ);

    const float upX = sideY * forwardZ - sideZ * forwardY;
    const float upY = sideZ * forwardX - sideX * forwardZ;
    const float upZ = sideX * forwardY - sideY * forwardX;

    matrix[0] = sideX;
    matrix[1] = upX;
    matrix[2] = -forwardX;
    matrix[3] = 0.0f;
    matrix[4] = sideY;
    matrix[5] = upY;
    matrix[6] = -forwardY;
    matrix[7] = 0.0f;
    matrix[8] = sideZ;
    matrix[9] = upZ;
    matrix[10] = -forwardZ;
    matrix[11] = 0.0f;
    matrix[12] = -(sideX * eyeX + sideY * eyeY + sideZ * eyeZ);
    matrix[13] = -(upX * eyeX + upY * eyeY + upZ * eyeZ);
    matrix[14] = forwardX * eyeX + forwardY * eyeY + forwardZ * eyeZ;
    matrix[15] = 1.0f;
}

void makePerspective(float* matrix, float aspect) {
    std::fill(matrix, matrix + 16, 0.0f);
    const float nearPlane = 0.2f;
    const float farPlane = 180.0f;
    const float scale = 1.0f / std::tan((60.0f * kPi / 180.0f) * 0.5f);
    matrix[0] = scale / aspect;
    matrix[5] = scale;
    matrix[10] = (farPlane + nearPlane) / (nearPlane - farPlane);
    matrix[11] = -1.0f;
    matrix[14] = (2.0f * farPlane * nearPlane) / (nearPlane - farPlane);
}

}  // namespace

extern "C" JNIEXPORT void JNICALL
Java_com_valleycity_mobile_Game3DActivity_nativeSurfaceCreated(JNIEnv*, jobject) {
    program = 0;
    vertexBuffer = 0;
    vertexCount = 0;

    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LEQUAL);
    glClearColor(0.52f, 0.74f, 0.91f, 1.0f);

    program = createProgram();
    if (program == 0) {
        logError("Unable to create the OpenGL ES 2.0 shader program");
        return;
    }

    positionLocation = glGetAttribLocation(program, "aPosition");
    colorLocation = glGetAttribLocation(program, "aColor");
    matrixLocation = glGetUniformLocation(program, "uMvp");
    buildCityGeometry();
}

extern "C" JNIEXPORT void JNICALL
Java_com_valleycity_mobile_Game3DActivity_nativeSurfaceChanged(
    JNIEnv*, jobject, jint width, jint height) {
    const int safeWidth = std::max(width, 1);
    const int safeHeight = std::max(height, 1);
    viewportAspect = static_cast<float>(safeWidth) / static_cast<float>(safeHeight);
    glViewport(0, 0, safeWidth, safeHeight);
    makePerspective(projection, viewportAspect);
}

extern "C" JNIEXPORT void JNICALL
Java_com_valleycity_mobile_Game3DActivity_nativeDrawFrame(JNIEnv*, jobject) {
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    if (program == 0 || vertexBuffer == 0 || vertexCount == 0) return;

    float view[16];
    float viewProjection[16];
    makeViewMatrix(view);
    multiplyMatrices(projection, view, viewProjection);

    glUseProgram(program);
    glUniformMatrix4fv(matrixLocation, 1, GL_FALSE, viewProjection);
    glBindBuffer(GL_ARRAY_BUFFER, vertexBuffer);
    glEnableVertexAttribArray(static_cast<GLuint>(positionLocation));
    glEnableVertexAttribArray(static_cast<GLuint>(colorLocation));
    glVertexAttribPointer(static_cast<GLuint>(positionLocation), 3, GL_FLOAT,
                          GL_FALSE, sizeof(Vertex),
                          reinterpret_cast<const void*>(offsetof(Vertex, x)));
    glVertexAttribPointer(static_cast<GLuint>(colorLocation), 3, GL_FLOAT,
                          GL_FALSE, sizeof(Vertex),
                          reinterpret_cast<const void*>(offsetof(Vertex, r)));
    glDrawArrays(GL_TRIANGLES, 0, vertexCount);
    glDisableVertexAttribArray(static_cast<GLuint>(positionLocation));
    glDisableVertexAttribArray(static_cast<GLuint>(colorLocation));
    glBindBuffer(GL_ARRAY_BUFFER, 0);
}

extern "C" JNIEXPORT void JNICALL
Java_com_valleycity_mobile_Game3DActivity_nativeOrbit(
    JNIEnv*, jobject, jfloat deltaX, jfloat deltaY) {
    cameraYaw -= deltaX * 0.008f;
    cameraPitch = std::clamp(cameraPitch + deltaY * 0.008f, 0.15f, 1.42f);
}

extern "C" JNIEXPORT void JNICALL
Java_com_valleycity_mobile_Game3DActivity_nativeZoom(
    JNIEnv*, jobject, jfloat scale) {
    if (std::isfinite(scale) && scale > 0.0f) {
        cameraDistance = std::clamp(cameraDistance / scale, 18.0f, 100.0f);
    }
}
