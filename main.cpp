#include <iostream>

#define GLFW_INCLUDE_NONE

#include "glad/glad.h"
#include <GLFW/glfw3.h>

#include <chrono>
#include <vector>

#include "glm/glm/glm.hpp"
#include "glm/glm/gtc/matrix_transform.hpp"

#define STB_IMAGE_IMPLEMENTATION

#include "stb_image.h"

#define STB_IMAGE_WRITE_IMPLEMENTATION

#include "stb_image_write.h"

#include "VertexBuffer.h"

#include "loadShader.h"

#define SEPARATE_IMG_WRITE true

std::string fileName = R"(C:\Users\Cale\CLionProjects\HSV_Mapper\pollard2.png)";

std::string compList = "[hue, val, val]";

short WHITESPACE = 0, ALPHA = 1, COMMA = 2;

int map_string_to_flag(std::string str) {
  if (str == "hue" || str == "h")
    return 0;
  if (str == "sat" || str == "saturation")
    return 1;
  if (str == "val" || str == "value" || str == "lightness")
    return 2;
  return -1;
}

int* get_flags_from_comp_list() {
  if (compList[0] != '[' || compList.back() != ']') {
    std::cerr << "Components list must be enclosed in brackets, i.e. '[hue, sat, val]'" << std::endl;
    return NULL;
  }

  std::string current = "";
  std::vector<std::string> strs;

  short expecting = ALPHA;
  for (int i = 1; i < compList.size() - 1; i++) {
    char c = compList[i];

    if (c == ' ')
      continue;

    if (std::isalpha(c)) {
      current += c;
    } else if (c == ',') {
      strs.push_back(current);
      current = "";
    } else {
      std::cerr << "Illegal character '" << c << "' in component list" << std::endl;
      return NULL;
    }
  }

  if (!current.empty())
    strs.push_back(current);

  int* out = new int[strs.size()];

  for (int i = 0; i < strs.size(); i++) {
    out[i] = map_string_to_flag(strs[i]);
    if (out[i] == -1)
      std::cerr << "Invalid string '" << strs[i] << "'" << std::endl;
  }

  return out;
}

void error_callback(int error, const char *description) {
  fprintf(stderr, "Error: %s\n", description);
}

void framebufferSizeChanged(GLFWwindow *window, int width, int height) {
  std::cout << "Framebuffer size changed!" << std::endl;
  std::cout << "Width: " << width << std::endl;
  std::cout << "Height: " << height << std::endl;
  glViewport(0, 0, width, height);
}

void separateScreenshot(int w, int h) {
  int comp = STBI_rgb_alpha;

  const char * names[] = {"hue", "sat", "val"};

  for (int i = 0; i < 3; i++) {
    auto *data = new GLubyte[w * h * comp];

    glReadPixels(w * i, 0, w, h, GL_RGBA, GL_UNSIGNED_BYTE, data);

    if (glGetError() != 0) {
      std::cout << "There was an error reading the framebuffer" << std::endl;
    }

    char filename[256];
    sprintf(filename, "output_%s.png", names[i]);
    stbi_write_png(filename, w, h, comp, data, w * comp);
    delete[] data;
  }
}

void screenshot(int w, int h) {

  std::cout << "Taking screenshot: " << w << " x " << h << std::endl;

  int comp = STBI_rgb_alpha;

  // TODO: look into https://stackoverflow.com/questions/4029870/how-to-create-a-dynamic-array-of-integers

  auto *data = new GLubyte[w * h * comp];

  glReadPixels(0, 0, w, h, GL_RGBA, GL_UNSIGNED_BYTE, data);

  std::cout << "Writing PNG..." << std::endl;

  if (glGetError() != 0) {
    std::cout << "There was an error reading the framebuffer" << std::endl;
  }

  stbi_write_png("output.png", w, h, comp, data, w * comp);

  std::cout << "PNG write complete" << std::endl;

  delete[] data;
}

void setWindowHints() {
  glfwWindowHint(GLFW_VISIBLE, GLFW_FALSE);
  // OpenGL 3.3
  glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
  glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
  glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
}

int main(int argc, char **argv) {
  if (!glfwInit()) {
    std::cout << "Unable to initialize GLFW context" << std::endl;
    return -1;
  }

  // Loading an image from command line
  if (argc == 2) {
    std::string userImg = argv[1];
    fileName = userImg;
    std::cout << "Using user-specified image: " << userImg << std::endl;
  }

  setWindowHints();

  glfwSetErrorCallback(error_callback);

  GLFWwindow *window = glfwCreateWindow(100, 100, "HSV Mapper", nullptr, nullptr);

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


  auto x = get_flags_from_comp_list();
  std::cout << x[0] << " " << x[1] << " " << x[2] << std::endl;

  glEnable(GL_BLEND);
  glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

  // stbi image loading
  int w, h;
  int comp;

  const int rows = 2, cols = 2;

  stbi_set_flip_vertically_on_load(false);
  unsigned char *image = stbi_load(fileName.c_str(), &w, &h, &comp, STBI_rgb_alpha);

  if (!image) {
    std::cout << "stbi_load failure:" << std::endl;
    std::cout << stbi_failure_reason() << std::endl;
    if (strcmp(stbi_failure_reason(), "can't fopen") == 0) {
      std::cout << "Unable to open file \"" << fileName << "\"" << std::endl;
      return -10;
    }
    return -2;
  }

  std::cout << "Comp: " << comp << std::endl;
  //stbi_write_png("test.png", w, h, STBI_rgb_alpha, image, STBI_rgb_alpha * w);

  // the image is now in memory
  std::cout << "Width: " << w << std::endl;
  std::cout << "Height: " << h << std::endl;

  GLuint wcImage;
  glGenTextures(1, &wcImage);
  glGenerateMipmap(GL_TEXTURE_2D);
  glBindTexture(GL_TEXTURE_2D, wcImage);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
  glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, w, h, 0, GL_RGBA, GL_UNSIGNED_BYTE, image);

  // generate texture
  GLuint fbTexture;
  glGenTextures(1, &fbTexture);

  // TODO: make sure that all these function calls are coming from the same place
  glBindTexture(GL_TEXTURE_2D, fbTexture);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

  // TODO: determine if mipmaps should be generated
  glGenerateMipmap(GL_TEXTURE_2D);
  glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, w * cols, h * rows, 0, GL_BGRA, GL_UNSIGNED_BYTE, nullptr);

  // TODO: determine if this should be called immediately after glTexImage2D
  stbi_image_free(image);

  // Texture has been created

  glfwSwapInterval(1);
//  glfwSetWindowSize(window, w * numImages, h);

  glViewport(0, 0, w * cols, h * rows);

  // FIXME: framebuffer stuff (it definitely does something, as nothing will draw with this)
  GLuint framebufferTarget;

  glGenFramebuffers(1, &framebufferTarget);
  glBindFramebuffer(GL_FRAMEBUFFER, framebufferTarget);

  std::cout << "Created framebuffer" << std::endl;

  glFramebufferTexture(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, fbTexture, 0);

  const GLenum buffers[]{GL_COLOR_ATTACHMENT0};
  glDrawBuffers(1, buffers);

  if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
    std::cout << "Unable to create framebuffer target" << std::endl;
    return -4;
  }
  std::cout << "Framebuffer target created!" << std::endl;

  // FIXME: this is just for testing
  GLuint programId = LoadShaders("vertex.glsl", "frag.glsl");

  GLuint vertexArrId;
  glGenVertexArrays(1, &vertexArrId);
  glBindVertexArray(vertexArrId);

  glUseProgram(programId);
  glUniform1i(glGetUniformLocation(programId, "texImage"), 0);

  GLuint MatrixID = glGetUniformLocation(programId, "MVP");

  glm::mat4 Projection = glm::ortho(0.f, (float) w * cols, 0.f, (float) h * rows, 0.0f, 100.0f);

  // TODO: auto-generate buffers

  std::vector<VertexBuffer*> vertexBuffers; // TODO: validate that this is correct

  for (int i = 0; i < rows; i++) {
    for (int j = 0; j < cols; j++) {
      auto tmpBuffer = new VertexBuffer(w * j, h * i, w, h);
      vertexBuffers.push_back(tmpBuffer);
    }
  }

  glUniformMatrix4fv(MatrixID, 1, GL_FALSE, &Projection[0][0]);

  std::cout << "Starting loop..." << std::endl;

  auto startTime = std::chrono::system_clock::now();

  glfwPollEvents();

  glActiveTexture(GL_TEXTURE0);
  glBindTexture(GL_TEXTURE_2D, fbTexture);
  glBindTexture(GL_TEXTURE_2D, wcImage);

  glUseProgram(programId);

  for (int i = 0; i < vertexBuffers.size(); i++) {
    glUniform1i(glGetUniformLocation(programId, "compIdx"), i);
    // TODO: render blank geometry instead of a texture
    glUniform1f(glGetUniformLocation(programId, "alpha"), i == vertexBuffers.size() - 1 ? 0 : 1);
    vertexBuffers.at(i)->bind();

    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, (4 * sizeof(float)), (void *) nullptr);

    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, (4 * sizeof(float)), (void *) (2 * sizeof(float)));

    glDrawArrays(GL_TRIANGLES, 0, 6);
  }

  if (SEPARATE_IMG_WRITE)
    separateScreenshot(w, h);
  else
    screenshot(w * cols, h * rows);

  std::cout << "Cleaning up..." << std::endl;

  glBindFramebuffer(GL_FRAMEBUFFER, 0);

  glDeleteFramebuffers(1, &framebufferTarget);
  glDeleteTextures(1, &fbTexture);
  glDeleteProgram(programId);

  glfwDestroyWindow(window);

  glfwTerminate();

  return 0;
}