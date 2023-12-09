#include "GLUtils.h"
#include <algorithm>
#include <fstream>
#include <sstream>
#include <vector>

GLUtils::GLUtils() {}


GLUtils::~GLUtils() {}

void GLUtils::loadShader(char* shaderSource, GLenum shaderType, GLuint &programId) {
    GLuint shaderId = glCreateShader(shaderType);

    GLint result = GL_FALSE; // compilation result
    int infoLogLength; // length of info log

    std::ifstream shaderFile(shaderSource);
    std::string shaderStr;
    const char* shader;

    if(!shaderFile.is_open()) {
        std::string error = "Error: could not read file ";
        throw std::exception(error.append(shaderSource).c_str());
    }

    // Read shader
    std::string buffer;
    while(std::getline(shaderFile, buffer)) {
        shaderStr += buffer + "\n";
    }

    shader = shaderStr.c_str();

    // Compile shader
    printf("Compiling shader\n");
    glShaderSource(shaderId,        // Shader handle
        1,               // Number of files
        &shader,  // Shader source code
        NULL);           // NULL terminated string
    glCompileShader(shaderId);

    // Check shader
    glGetShaderiv(shaderId, GL_COMPILE_STATUS, &result);
    glGetShaderiv(shaderId, GL_INFO_LOG_LENGTH, &infoLogLength);
    std::vector<char> errorMessage(std::max(infoLogLength, int(1)));
    glGetShaderInfoLog(shaderId, infoLogLength, NULL, &errorMessage[0]);
    fprintf(stdout, "%s\n", &errorMessage[0]);

    // Link the program
    fprintf(stdout, "Linking program\n");
    glAttachShader(programId, shaderId);
    glLinkProgram(programId);

    // Check the program
    glGetProgramiv(programId, GL_LINK_STATUS, &result);
    glGetProgramiv(programId, GL_INFO_LOG_LENGTH, &infoLogLength);
    std::vector<char> programErrorMessage(std::max(infoLogLength, int(1)));
    glGetProgramInfoLog(programId, infoLogLength, NULL, &programErrorMessage[0]);
    fprintf(stdout, "%s\n", &programErrorMessage[0]);

    glDeleteShader(shaderId);

    shaderFile.close();
}
