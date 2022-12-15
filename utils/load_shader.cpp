#include <iostream>

#include "../glad/glad.h"

#include "load_shader.h"

#include "../shaders/generated.h"

#define VERBOSE false

GLuint loadShader(int shaderType, const char* shaderSrc) {
  auto shaderId = glCreateShader(shaderType);

  glShaderSource(shaderId, 1, &shaderSrc, nullptr);
  glCompileShader(shaderId);

  auto status = GL_FALSE;
  glGetShaderiv(shaderId, GL_COMPILE_STATUS, &status);

  if (status == GL_FALSE) {
    std::cout << "A fatal error occurred when compiling the shader" << std::endl;
    return 0;
  }

  return shaderId;
}

GLuint createShaderProgram() {
  auto compiledVertexShaderId = loadShader(GL_VERTEX_SHADER, vertSrc);
  auto compiledFragmentShaderId = loadShader(GL_FRAGMENT_SHADER, fragSrc);

  auto program = glCreateProgram();

  glAttachShader(program, compiledVertexShaderId);
  glAttachShader(program, compiledFragmentShaderId);

  glLinkProgram(program);

  auto status = GL_FALSE;
  glGetProgramiv(program, GL_LINK_STATUS, &status);

  if (status == GL_FALSE) {
    std::cout << "A fatal error occurred when compiling the shader" << std::endl;
    return 0;
  }

  glDetachShader(program, compiledVertexShaderId);
  glDetachShader(program, compiledFragmentShaderId);

  glDeleteShader(compiledFragmentShaderId);
  glDeleteShader(compiledVertexShaderId);

  if (VERBOSE)
    printf("Shader program created successfully\n");

  return program;
}