#include <iostream>

#define GLFW_INCLUDE_NONE
#include "glad/glad.h"
#include <GLFW/glfw3.h>

#include "glm/glm/glm.hpp"
#include "glm/glm/matrix.hpp"
#include "glm/glm/gtc/matrix_transform.hpp"
#include "glm/glm/gtc/type_ptr.hpp"

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb_image_write.h"

#include "loadShader.h"
#include "glm/glm/ext/matrix_clip_space.hpp"

#define FRAMEBUFFER_WRITE false

std::string fileName = R"(C:\Users\Cale\CLionProjects\HSV_Mapper\pollard2.png)";

void error_callback(int error, const char* description) {
  fprintf(stderr, "Error: %s\n", description);
}

static void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods) {
  if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
    glfwSetWindowShouldClose(window, GLFW_TRUE);
}

void framebufferSizeChanged(GLFWwindow * window, int width, int height) {
  std::cout << "Framebuffer size changed!" << std::endl;
  std::cout << "Width: " << width << std::endl;
  std::cout << "Height: " << height << std::endl;
  glViewport(0, 0, width, height);
}

void screenshot(int w, int h) {

  std::cout << "Taking screenshot" << std::endl;

  int comp = 4;

  unsigned char pixels[w * h * comp];

  // TODO: look into https://stackoverflow.com/questions/4029870/how-to-create-a-dynamic-array-of-integers

  auto * data = new GLubyte[w * h * comp];

  glReadPixels(0, 0, w, h, GL_RGBA, GL_UNSIGNED_BYTE, data);

  std::cout << "PIXELS: " << data[0] << std::endl;

  std::cout << "Writing PNG" << std::endl;

  stbi_write_png("output.png", w, h, comp, pixels, w * comp);

  std::cout << "PNG write complete" << std::endl;
}

void setWindowHints() {
//  glfwWindowHint(GLFW_VISIBLE, GLFW_FALSE);
  // OpenGL 3.3
  glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
  glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
  glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
}

int main() {
  if (!glfwInit()) {
    std::cout << "Unable to initialize GLFW context" << std::endl;
    return -1;
  }

  setWindowHints();

  glfwSetErrorCallback(error_callback);

  GLFWwindow* window = glfwCreateWindow(1000, 1000, "HSV Mapper", nullptr, nullptr);

  glfwSetFramebufferSizeCallback(window, framebufferSizeChanged);

  if (!window) {
    std::cout << "Unable to initialize GLFW window" << std::endl;
    return -1;
  }

  glfwMakeContextCurrent(window);

  if (!gladLoadGLLoader((GLADloadproc) glfwGetProcAddress)) {
    std::cout << "Failed to initialize GLAD" << std::endl;
    return -3;
  }

  glfwSetKeyCallback(window, key_callback);

  // stbi image loading

  int w, h;
  int comp;

  stbi_set_flip_vertically_on_load(true);
  unsigned char * image = stbi_load(fileName.c_str(), &w, &h, &comp, STBI_rgb_alpha);

  if (!image) {
    std::cout << "stbi_load failure:" << std::endl;
    std::cout << stbi_failure_reason() << std::endl;
    return -2;
  }

  // # test stbi_write with the image buffer to get that working first

  std::cout << "Comp: " << comp << std::endl;
  stbi_write_png("test.png", w, h, comp, image, comp * w);

  // the image is now in memory
  std::cout << "Width: " << w << std::endl;
  std::cout << "Height: " << h << std::endl;

  // generate texture
  GLuint textureID;
  glGenTextures(1, &textureID);

  // TODO: make sure that all these function calls are coming from the same place
  glBindTexture(GL_TEXTURE_2D, textureID);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

  // TODO: determine if mipmaps should be generated
  glGenerateMipmap(GL_TEXTURE_2D);
  glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, w, h, 0, GL_RGBA, GL_UNSIGNED_BYTE, image);

  // TODO: determine if this should be called immediately after glTexImage2D
  stbi_image_free(image);

  std::cout << "Created texture" << std::endl;

  // FIXME: framebuffer stuff (it definitely does something, as nothing will draw with this)
//  GLuint framebufferTarget;
//  glGenFramebuffers(1, &framebufferTarget);
//  glBindFramebuffer(GL_FRAMEBUFFER, framebufferTarget);
//
//  std::cout << "Created framebuffer" << std::endl;
//
//  glFramebufferTexture(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, textureID, 0);
//
//  const GLenum buffers[]{ GL_COLOR_ATTACHMENT0 };
//
//  // TODO: 3 draw buffers
//  glDrawBuffers(1, buffers);
//
//  if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
//    std::cout << "Unable to create framebuffer target" << std::endl;
//    return -4;
//  }
//
//  std::cout << "Framebuffer target created!" << std::endl;

  // FIXME: this is just for testing
  GLuint programId = LoadShaders("vertex.glsl", "frag.glsl");

  GLuint vertexArrId;
  glGenVertexArrays(1, &vertexArrId);
  glBindVertexArray(vertexArrId);

  // An array of 3 vectors which represents 3 vertices
  static const GLfloat g_vertex_buffer_data[] = {
      -1.0f, -1.0f,
      1.0f, -1.0f,
      0.0f,  1.0f,
  };

  float vertices[] = {
      // positions    // texture coords
      .5f * 2.f,  .5f * 2.f,    1.0f, 1.0f,   // top right
      .5f * 2.f, -.5f * 2.f,     1.0f, 0.0f,   // bottom right
      -.5f * 2.f, -0.5f * 2.f,   0.0f, 0.0f,   // bottom left
      -0.5f * 2.f,  0.5f * 2.f,    0.0f, 1.0f    // top left
  };

  unsigned int indices[] = {
      0, 1, 3, // first triangle
      1, 2, 3  // second triangle
  };

  bool screenshotted = false;

  // TODO: look into EBO
  GLuint VBO, VAO, EBO; // Generate 1 buffer
  glGenVertexArrays(1, &VAO);
  glGenBuffers(1, &VBO);
  glGenBuffers(1, &EBO);
  glBindBuffer(GL_ARRAY_BUFFER, VBO); // Bind the buffer
  glBufferData(GL_ARRAY_BUFFER, sizeof(g_vertex_buffer_data), g_vertex_buffer_data, GL_STATIC_DRAW);

  glBindVertexArray(VAO);

  glBindBuffer(GL_ARRAY_BUFFER, VBO);
  glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
  glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);

  // position attribute
  glEnableVertexAttribArray(0);
  glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*) 0);
  // color attribute
  glEnableVertexAttribArray(1);
  glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*) (2 * sizeof(float)));

  // vsync, clear color
  glfwSwapInterval(1);
  glClearColor(0, 0, 0, 1);
  glfwSetWindowSize(window, w, h);

  glViewport(0, 0, w, h);

//  glOrtho(0, w, 0, h, -1, 1);

  // TODO: set up matrices
  // view, projection
  // view matrix = identity matrix
  glm::mat4 view = glm::mat4(0.0f);

  // projection matrix = orthographic
  glm::mat4 projection = glm::ortho(0.f, (float) w, (float) h, 0.f, -1.f, 1.f);
  glUniformMatrix4fv(glGetUniformLocation(programId, "projection"), 1, GL_FALSE, glm::value_ptr(projection));

//  glUniformMatrix4fv(glGetUniformLocation(programId, "view"), 1, GL_FALSE, &View[0][0]);

  glUseProgram(programId);
  glUniform1i(glGetUniformLocation(programId, "texImage"), 0);

  std::cout << "Starting loop..." << std::endl;

  while (!glfwWindowShouldClose(window)) {
    glClear(GL_COLOR_BUFFER_BIT);
    glfwPollEvents();

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, textureID);

    glUseProgram(programId);
    glBindVertexArray(VAO);
//    glDrawArrays(GL_TRIANGLES, 0, 4);
    glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, nullptr);

    // glDrawArrays(GL_TRIANGLES, 0, 6);

    if (!screenshotted && (screenshotted = true))
      screenshot(100, 100);
    glfwSwapBuffers(window);
  }

  std::cout << "Cleaning up..." << std::endl;

  // FIXME: be sure to re-enable this
//  glDeleteFramebuffers(1, &framebufferTarget);
  glDeleteBuffers(1, &VBO);
  glDeleteProgram(programId);

  glfwDestroyWindow(window);

  glfwTerminate();

  return 0;
}
