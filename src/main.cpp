#include <iostream>

#define GLFW_INCLUDE_NONE

#include "../glad/glad.h"
#include <GLFW/glfw3.h>

#include <vector>
#include <set>

#include "../glm/glm/glm.hpp"
#include "../glm/glm/gtc/matrix_transform.hpp"

#define STB_IMAGE_IMPLEMENTATION

#include "stb_image.h"

#define STB_IMAGE_WRITE_IMPLEMENTATION

#include "stb_image_write.h"

#include "VertexBuffer.h"

#include "loadShader.h"

#include "consts.h"
#include "Texture.h"

#include "utils.h"

// TODO: create config struct

bool SEPARATE_IMG_WRITE = false;

int rows = 1, cols = 3;

std::string fileName;

std::string compList = "hue;sat;val";

int numComps = 3;

void error_callback(int error, const char *description) {
  fprintf(stderr, "Error: (%d) %s\n", error, description);
}

void separateScreenshot(const std::vector<VertexBuffer*>& buffers, int w, int h, std::vector<int> comps) {
  int comp = STBI_rgb_alpha;

  std::cout << "Taking separateScreenshot" << std::endl;

  // TODO: use a set for this
  const char *names[] = {"hue", "sat", "val"};

  for (int i = 0; i < buffers.size(); i++) {
    auto buffer = buffers[i];

    auto *data = new GLubyte[w * h * comp];

    glReadPixels(buffer->getX(), buffer->getY(), w, h, GL_RGBA, GL_UNSIGNED_BYTE, data);

    if (glGetError() != 0) {
      std::cout << "There was an error reading the framebuffer, gl err " << glGetError() << std::endl;
    }

    char filename[256];
    sprintf(filename, "output_%s.png", names[comps[i]]);
    std::cout << "Writing file " << filename << "..." << std::endl;
    stbi_write_png(filename, w, h, comp, data, w * comp);
    delete[] data;
  }
}

void screenshotSingleBuffer(int w, int h) {

  std::cout << "Taking screenshotSingleBuffer: " << w << " x " << h << std::endl;

  int comp = STBI_rgb_alpha;

  // TODO: look into https://stackoverflow.com/questions/4029870/how-to-create-a-dynamic-array-of-integers
  auto *data = new GLubyte[w * h * comp];

  glReadPixels(0, 0, w, h, GL_RGBA, GL_UNSIGNED_BYTE, data);

  std::cout << "Writing PNG..." << std::endl;

  if (glGetError() != 0) {
    std::cout << "There was an error reading the framebuffer, err is " << glGetError() << std::endl;
  }

  stbi_write_png("output.png", w, h, comp, data, w * comp);

  std::cout << "PNG write complete" << std::endl;

  delete[] data;
}

void setWindowHints() {
//  glfwWindowHint(GLFW_VISIBLE, GLFW_FALSE);
  glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3); // OpenGL 3.3
  glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
  glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
}

// TODO: put this in utils
void parseDims(const std::string& dims) {
  if (dims.length() < 3) {
    std::cerr << "Dimensions must be 2 numbers separated by an 'x'" << std::endl;
    return;
  }
  int w, h;
  std::string wTemp, hTemp;
  bool fillWidth = true;
  for (char dim : dims) {
    if (dim == 'x') {
      if (fillWidth) {
        fillWidth = false;
      } else {
        std::cerr << "Error parsing dimensions" << std::endl;
        return;
      }
      continue;
    }
    if (fillWidth)
      wTemp += dim;
    else
      hTemp += dim;
  }
  try {
    w = std::stoi(wTemp);
    h = std::stoi(hTemp);
  } catch (const std::invalid_argument& ia) {
    std::cerr << ia.what() << std::endl;
    return;
  }
  std::cout << "w: " << w << std::endl;
  std::cout << "h: " << h << std::endl;
  cols = w;
  rows = h;
}

bool parseArgs(int argc, char **argv) {
  for (int i = 1; i < argc; i++) {
    if (std::string(argv[i]) == "-d" || std::string(argv[i]) == "--dimensions") {
      if (i + 1 == argc) {
        std::cout << "Expected dimensions" << std::endl;
        return 3;
      }
      std::string dims = argv[i + 1];
      std::cout << "Parsing dimensions: " << dims << std::endl;
      parseDims(dims); // TODO: handle parseDims() errors here
      SEPARATE_IMG_WRITE = false;
      i++;
    } else if (std::string(argv[i]) == "-o") {
      if (i + 1 == argc) {
        std::cout << "Expected filename" << std::endl;
        return 4;
      }
      std::string outputFile = argv[i + 1];
      std::cout << "Writing to output file: " << outputFile << std::endl;
      // TODO: implement this
      i++;
    } else if (std::string(argv[i]) == "-c") {
      if (i + 1 == argc) {
        std::cout << "Expected component list" << std::endl;
        return 5;
      }
      compList = argv[i + 1];
      i++;
    } else {
      // Assume that the argument is an image
      std::string userImg = argv[i];
      fileName = userImg;
      std::cout << "Using user-specified image: " << userImg << std::endl;
    }
  }

  return false;
}

int main(int argc, char **argv) {
  if (parseArgs(argc, argv)) {
    return -5;
  }

  if (!glfwInit()) {
    std::cerr << "Unable to initialize GLFW context" << std::endl;
    return GLFW_INIT_FAILURE;
  }

  glfwSetErrorCallback(error_callback);

  setWindowHints();
  GLFWwindow *window = glfwCreateWindow(1, 1, "", nullptr, nullptr);

  if (!window) {
    std::cout << "Unable to initialize GLFW window" << std::endl;
    return GLFW_WINDOW_INIT_FAILURE;
  }

  glfwMakeContextCurrent(window);

  if (!gladLoadGLLoader((GLADloadproc) glfwGetProcAddress)) {
    std::cout << "Failed to initialize GLAD" << std::endl;
    return GLAD_INIT_FAILURE;
  }

  std::vector<int> flagsFromCompList = get_flags_from_comp_list(compList, numComps);// = get_flags_from_comp_list();
  if (flagsFromCompList.empty()) {
    return COMP_FLAGS_UNREADABLE;
  }

  if (SEPARATE_IMG_WRITE) {
    std::set<int> filtered;
    for (auto v : flagsFromCompList)
      filtered.insert(v);
    std::cout << "Filtered size: " << filtered.size() << std::endl;
    flagsFromCompList.clear();
    std::cout << std::endl;
    for (auto v : filtered) {
      std::cout << v << " ";
      flagsFromCompList.push_back(v);
    }
    std::cout << std::endl;
  }

  glEnable(GL_BLEND);
  glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

  // stbi image loading
  int w, h;
  int comp;

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

  glViewport(0, 0, w * cols, h * rows);

  std::cout << "Comp: " << comp << std::endl;

  // the image is now in memory
  std::cout << "Width: " << w << std::endl;
  std::cout << "Height: " << h << std::endl;

  Texture wcTex, fbTex;

  wcTex.uploadData(w, h, image);
  stbi_image_free(image);

  fbTex.uploadData(w * cols, h * rows, nullptr);

  GLuint framebufferTarget;
  glGenFramebuffers(1, &framebufferTarget);
  glBindFramebuffer(GL_FRAMEBUFFER, framebufferTarget);

  std::cout << "Created framebuffer" << std::endl;

  glFramebufferTexture(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, fbTex.id, 0);

  const GLenum buffers[]{GL_COLOR_ATTACHMENT0};
  glDrawBuffers(1, buffers);

  if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
    std::cout << "Unable to create framebuffer target" << std::endl;
    return FRAMEBUFFER_CREATION_FAILURE;
  }
  std::cout << "Framebuffer target created!" << std::endl;

  // FIXME: this is just for testing
  GLuint programId = LoadShaders("vertex.glsl", "frag.glsl");

  glUseProgram(programId);
  glUniform1i(glGetUniformLocation(programId, "texImage"), 0);

  auto projMatrix = glm::ortho(0.f, (float) w * cols, 0.f, (float) h * rows, 0.0f, 100.0f);

  glUniformMatrix4fv(glGetUniformLocation(programId, "MVP"), 1, GL_FALSE, &projMatrix[0][0]);

  // Generate buffers
  std::vector<VertexBuffer *> vertexBuffers; // TODO: validate that this is correct

  if (SEPARATE_IMG_WRITE) {
    rows = 1;
    cols = flagsFromCompList.size();
  }

  for (int i = 0; i < rows; i++) {
    for (int j = 0; j < cols; j++) {
      auto tmpBuffer = new VertexBuffer(w * j, h * i, w, h);
      vertexBuffers.push_back(tmpBuffer);
    }
  }

  std::cout << "Starting rendering... buffers: " << vertexBuffers.size() << std::endl;

  glActiveTexture(GL_TEXTURE0);

  wcTex.bind();

  glUseProgram(programId);

  // Render buffers
  for (int i = 0; i < vertexBuffers.size(); i++) {
    if (i >= flagsFromCompList.size()) {
      std::cout << "i: " << i;
      std::cout << "; Out of bounds, rendering blank image..." << std::endl;
    } else {
      std::cout << "Rendering component: " << flagsFromCompList[i] << std::endl;
    }
    glUniform1i(glGetUniformLocation(programId, "compIdx"), flagsFromCompList[i]);
    // TODO: render blank geometry instead of a texture
    if (rows * cols > numComps && i == vertexBuffers.size() - 1)
      glUniform1f(glGetUniformLocation(programId, "alpha"), 0.f);
    else
      glUniform1f(glGetUniformLocation(programId, "alpha"), 1.f);

    vertexBuffers.at(i)->bind();

    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, (4 * sizeof(float)), (void *) nullptr);

    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, (4 * sizeof(float)), (void *) (2 * sizeof(float)));

    glDrawArrays(GL_TRIANGLES, 0, 6);
  }

  if (SEPARATE_IMG_WRITE)
    separateScreenshot(vertexBuffers, w, h, flagsFromCompList);
  else
    screenshotSingleBuffer(w * cols, h * rows);

  std::cout << "Cleaning up..." << std::endl;

  glDeleteFramebuffers(1, &framebufferTarget);
  fbTex.free();
  wcTex.free();
  glDeleteProgram(programId);

  glfwDestroyWindow(window);

  glfwTerminate();

  return 0;
}
