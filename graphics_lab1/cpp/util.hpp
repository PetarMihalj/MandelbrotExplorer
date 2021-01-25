#ifndef UTIL_LOADER_HPP
#define UTIL_LOADER_HPP

#include <GL/glew.h>
#include <GL/gl.h>
#include <sys/time.h>

bool readFile(char **lines, const char *filename);
bool compileShader(const char *path, GLuint shaderID, char *source_code);
bool linkShaders(GLuint &programID, GLuint vertexShaderID,
                 GLuint fragmentShaderID);
GLuint loadShaders(const char *vertex_file_path,
                   const char *fragment_file_path);

long long timer();
#endif
