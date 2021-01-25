#include "util.hpp"
#include <string>
#include <fstream>
#include <iostream>
#include <streambuf>
#include <vector>
#include <stdlib.h>

long long timer() {
    // sa stackoverflowa
    struct timeval tp;
    gettimeofday(&tp, NULL);
    return (long long)tp.tv_sec * 1000L + tp.tv_usec / 1000;
}

bool readFile(char **lines, const char *filename) {

    FILE *f = fopen(filename, "rb");
    if (f == NULL)
        return false;
    fseek(f, 0, SEEK_END);
    int pos = ftell(f);
    fseek(f, 0, SEEK_SET);

    char *buf = (char *)malloc(pos + 1);
    if (buf == NULL) {
        fclose(f);
        return false;
    }

    bool success = pos > 0 ? 1 == fread(buf, pos, 1, f) : true;
    buf[pos] = '\0';

    fclose(f);

    if (!success) {
        free(buf);
        return false;
    }

    *lines = buf;

    return true;
}

bool compileShader(const char *path, GLuint shaderID, char *source_code) {
    std::cout << "Compiling shader : " << path << std::endl << std::flush;
    char const *src = source_code;
    glShaderSource(shaderID, 1, &src, NULL);
    glCompileShader(shaderID);

    // Check Shader Compilation Status
    GLint result = GL_FALSE;
    int infoLogLength;
    glGetShaderiv(shaderID, GL_COMPILE_STATUS, &result);
    glGetShaderiv(shaderID, GL_INFO_LOG_LENGTH, &infoLogLength);
    if (infoLogLength > 0) {
        std::vector<char> shaderErrorMessage(infoLogLength + 1);
        glGetShaderInfoLog(shaderID, infoLogLength, NULL,
                           &shaderErrorMessage[0]);
        std::cout << (char *)(&shaderErrorMessage[0]) << std::endl;
    }

    return result == GL_TRUE ? true : false;
}

bool linkShaders(GLuint &programID, GLuint vertexShaderID,
                 GLuint fragmentShaderID) {

    // Link the program
    std::cout << "Linking program" << std::endl << std::flush;

    programID = glCreateProgram();
    glAttachShader(programID, vertexShaderID);
    glAttachShader(programID, fragmentShaderID);
    glLinkProgram(programID);

    // Check the program
    GLint result = GL_FALSE;
    int infoLogLength;
    glGetProgramiv(programID, GL_LINK_STATUS, &result);
    glGetProgramiv(programID, GL_INFO_LOG_LENGTH, &infoLogLength);
    if (infoLogLength > 0) {
        std::vector<char> programErrorMessage(infoLogLength + 1);
        glGetProgramInfoLog(programID, infoLogLength, NULL,
                            &programErrorMessage[0]);
        std::cout << (char *)(&programErrorMessage[0]) << std::endl;
    }

    glDetachShader(programID, vertexShaderID);
    glDetachShader(programID, fragmentShaderID);

    return result == GL_TRUE ? true : false;
}

GLuint loadShaders(const char *vertex_file_path,
                   const char *fragment_file_path) {

    std::cout << "Creating vertex shader... " << std::endl << std::flush;
    GLuint vertexShaderID = glCreateShader(GL_VERTEX_SHADER);
    std::cout << "Going to read... " << std::endl << std::flush;
    char *vertexShaderCode = NULL;
    if (!readFile(&vertexShaderCode, vertex_file_path))
        return 0;

    std::cout << "Creating fragment shader... " << std::endl << std::flush;
    GLuint fragmentShaderID = glCreateShader(GL_FRAGMENT_SHADER);
    std::cout << "Going to read... " << std::endl << std::flush;
    char *fragmentShaderCode = NULL;
    if (!readFile(&fragmentShaderCode, fragment_file_path)) {
        free(vertexShaderCode);
        return 0;
    }

    bool compiled;
    compiled =
        compileShader(vertex_file_path, vertexShaderID, vertexShaderCode);
    compiled = compileShader(fragment_file_path, fragmentShaderID,
                             fragmentShaderCode) &&
               compiled;

    free(vertexShaderCode);
    free(fragmentShaderCode);

    GLuint programID;

    if (compiled) {
        compiled = linkShaders(programID, vertexShaderID, fragmentShaderID);
    }

    glDeleteShader(vertexShaderID);
    glDeleteShader(fragmentShaderID);

    return !compiled ? 0 : programID;
}
