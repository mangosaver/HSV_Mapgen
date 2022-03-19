#include <iostream>

#define GLFW_INCLUDE_NONE
#include "glad/glad.h"
#include <GLFW/glfw3.h>

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb_image_write.h"

std::string fileName = "C:\\Users\\Cale\\CLionProjects\\HSV_Mapper\\pollard2.png";

void error_callback(int error, const char* description) {
  fprintf(stderr, "Error: %s\n", description);
}

static void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods) {
  if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
    glfwSetWindowShouldClose(window, GLFW_TRUE);
}

void screenshot(int w, int h) {

  std::cout << "Taking screenshot" << std::endl;

  int comp = 4;

  unsigned char pixels[w * h * comp];

  glReadPixels(0, 0, w, h, GL_RGBA, GL_UNSIGNED_BYTE, pixels);

  std::cout << "Writing PNG" << std::endl;

  stbi_write_png("output.png", w, h, comp, pixels, w * comp);
}

void setWindowHints() {
//  glfwWindowHint(GLFW_VISIBLE, GLFW_FALSE);
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

  unsigned char * image = stbi_load(fileName.c_str(), &w, &h, &comp, STBI_rgb_alpha);

  if (!image) {
    std::cout << "stbi_load failure:" << std::endl;
    std::cout << stbi_failure_reason() << std::endl;
    return -2;
  }

  // # test stbi_write with the image buffer to get that working first

  stbi_write_png("test.png", w, h, comp, image, 4);

  // the image is now in memory
  std::cout << "Width: " << w << std::endl;
  std::cout << "Height: " << h << std::endl;

  // generate texture

  GLuint tex;
  glGenTextures(1, &tex);

  // TODO: make sure that all these function calls are coming from the same place
  glBindTexture(GL_TEXTURE_2D, tex);
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

  // init framebuffer
  GLuint framebufferTarget;
  glGenFramebuffers(1, &framebufferTarget);
  glBindFramebuffer(GL_FRAMEBUFFER, framebufferTarget);

  std::cout << "Created framebuffer" << std::endl;

  // upload data to shaders
  // run

  glFramebufferTexture(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, tex, 0);

  const GLenum buffers[]{ GL_COLOR_ATTACHMENT0 };

  // TODO: 3 draw buffers
  glDrawBuffers(1, buffers);

  if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
    std::cout << "Unable to create framebuffer target" << std::endl;
    return -4;
  }

  std::cout << "Framebuffer target created!" << std::endl;

  // Draw the texture to the framebuffer

  while (!glfwWindowShouldClose(window)) {
    glfwPollEvents();
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    glViewport(0, 0, 1000, 1000);
  }

  //screenshot(w, h);

  std::cout << "Cleaning up..." << std::endl;

  glDeleteFramebuffers(1, &framebufferTarget);

  glfwDestroyWindow(window);

  glfwTerminate();

  return 0;
}
