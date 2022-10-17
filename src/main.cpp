#include <iostream>
#include <vector>

#define GLFW_INCLUDE_NONE

#include "../glad/glad.h"
#include <GLFW/glfw3.h>

#include "../glm/glm/glm.hpp"
#include "../glm/glm/gtc/matrix_transform.hpp"

#define STB_IMAGE_IMPLEMENTATION

#include "../include/stb_image.h"

#define STB_IMAGE_WRITE_IMPLEMENTATION

#include "../include/stb_image_write.h"

#include "VertexBuffer.h"

#include "../utils/load_shader.h"

#include "consts.h"
#include "Texture.h"

#include "../utils/parse_utils.h"
#include "../utils/config_utils.h"

#include "../utils/print_help.h"
#include "../utils/log_utils.h"

ProgramConfig myConfig;

bool IS_VERBOSE = false;

long long startTimeMs;

int maxWidth, maxHeight;

GLuint framebufferTarget;

void writeImage(const char *filename, int w, int h, int comp, const GLubyte *data) {
  if (myConfig.write_jpeg)
    stbi_write_jpg(filename, w, h, comp, data, (int) myConfig.jpeg_quality);
  else
    stbi_write_png(filename, w, h, comp, data, w * comp);
  std::cout << "Wrote file " << filename << std::endl;
}

void separateScreenshot(const std::vector<VertexBuffer> &buffers, int w, int h, std::vector<int> comps) {
  std::cout << "Taking separateScreenshot" << std::endl;

  // TODO: put this in utils
  const char *names[] = {"hue", "sat", "val", "rgb", "red", "green", "blue"};

  std::cout << "numChannels: " << myConfig.numChannels << std::endl;

  auto *data = new GLubyte[w * h * myConfig.numChannels];

  for (int i = 0; i < buffers.size(); i++) {
    auto buffer = buffers[i];

    glReadPixels(buffer.getX(), buffer.getY(), w, h, getColorFormatFromNumComponents(myConfig.numChannels),
                 GL_UNSIGNED_BYTE,
                 data);

    auto err = glGetError();
    if (err != GL_NO_ERROR) {
      std::cout << "[1] There was an error reading the framebuffer, gl err " << err << std::endl;
      delete[] data;
      return;
    }

    char filename[256];
    auto extStr = myConfig.write_jpeg ? "jpeg" : "png";

    sprintf(filename, "%s_%s.%s", myConfig.outputFile.c_str(), names[comps[i]], extStr);
    writeImage(filename, w, h, myConfig.numChannels, data);
  }
  delete[] data;
}

void screenshotSingleBuffer(unsigned int w, unsigned int h) {

  std::cout << "Taking screenshotSingleBuffer: " << w << " x " << h << std::endl;
  std::cout << "numChannels: " << myConfig.numChannels << std::endl;

  auto *data = new GLubyte[w * h * myConfig.numChannels];

  glReadPixels(0, 0, w, h, getColorFormatFromNumComponents(myConfig.numChannels), GL_UNSIGNED_BYTE, data);

  auto err = glGetError();
  if (err != GL_NO_ERROR) {
    std::cout << "[2] There was an error reading the framebuffer, err is " << err << std::endl;
  }

  // TODO: validate that the filename cannot exceed 256 characters
  char filename[256];

  auto extStr = myConfig.write_jpeg ? "jpeg" : "png";

  if (myConfig.inputFile == myConfig.outputFile)
    sprintf(filename, "%s_map.%s", myConfig.outputFile.c_str(), extStr);
  else
    sprintf(filename, "%s.%s", myConfig.outputFile.c_str(), extStr);

  writeImage(filename, w, h, myConfig.numChannels, data);

  delete[] data;
}

void setWindowHints() {
  glfwWindowHint(GLFW_VISIBLE, GLFW_FALSE);
  glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3); // OpenGL 3.3
  glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
  glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
}

int processImage(std::string &file, std::vector<int> &flagsFromCompList, GLuint programId) {

  std::cout << "Processing image: " << file << std::endl;

  // stbi image loading
  // TODO: abstract this, add support for image list
  int w, h, comp;

  unsigned char *image = stbi_load(file.c_str(), &w, &h, &comp, STBI_rgb_alpha);

  if (myConfig.separate_img_write) {
    myConfig.rows = 1;
    myConfig.cols = flagsFromCompList.size();
  }

  std::cout << "width: " << w << ", height: " << h << ", comp: " << comp << std::endl;
  std::cout << "rows: " << myConfig.rows << ", cols: " << myConfig.cols << std::endl;

  if (!image) {
    if (strcmp(stbi_failure_reason(), "can't fopen") == 0) {
      std::cout << "Unable to open file \"" << file << "\"" << std::endl;
      return IMAGE_NOT_FOUND;
    }
    std::cerr << "Unknown error loading image" << std::endl;
    return IMAGE_LOAD_ERR;
  }

  const auto fullWidth = w * myConfig.cols;
  const auto fullHeight = h * myConfig.rows;

  if (fullWidth > maxWidth || fullHeight > maxHeight) {
    std::cerr << "Error: the size of the generated framebuffer (" << fullWidth
              << " x " << fullHeight << ") exceeds this machine's limit (" << maxWidth
              << " x " << maxHeight << ")" << std::endl;
    return MAX_VIEWPORT_SIZE_EXCEEDED;
  }

  glViewport(0, 0, fullWidth, fullHeight);

  myConfig.numChannels = comp;

  Texture imageTexture, framebufferTexture;

  imageTexture.uploadData(w, h, image);
  stbi_image_free(image);

  // Required in OpenGL 3.3 // TODO: double check this
  unsigned int VAO;
  glGenVertexArrays(1, &VAO);
  glBindVertexArray(VAO);

  // TODO: create framebuffer function
  glGenFramebuffers(1, &framebufferTarget);
  glBindFramebuffer(GL_FRAMEBUFFER, framebufferTarget);

  framebufferTexture.uploadData(fullWidth, fullHeight, nullptr);

  glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, framebufferTexture.id, 0);

  const GLenum buffers[]{GL_COLOR_ATTACHMENT0};
  glDrawBuffers(1, buffers);

  if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
    std::cerr << "Unable to create framebuffer target: err " << glGetError() << std::endl;
    return FRAMEBUFFER_CREATION_FAILURE;
  }

  auto projMatrix = glm::ortho(0.f, (float) fullWidth, 0.f, (float) fullHeight, 0.0f, 100.0f);
  glUniformMatrix4fv(glGetUniformLocation(programId, "MVP"), 1, GL_FALSE, &projMatrix[0][0]);

  // Generate buffers
  std::vector<VertexBuffer> vertexBuffers; // TODO: validate that this is correct
  vertexBuffers.reserve(myConfig.rows * myConfig.cols);

  for (int i = 0; i < myConfig.rows; i++) {
    for (int j = 0; j < myConfig.cols; j++) {
      auto tmpBuffer = VertexBuffer(w * j, h * i, w, h);
      vertexBuffers.push_back(tmpBuffer);
      std::cout << "buffer added" << std::endl;
    }
  }

  glActiveTexture(GL_TEXTURE0);
  imageTexture.bind();

  std::cout << "ROWS: " << myConfig.rows << " COLS: " << myConfig.cols << std::endl;

  // Render buffers
  for (int i = 0; i < vertexBuffers.size(); i++) {
    if (i >= flagsFromCompList.size()) {
      std::cout << "i: " << i;
      std::cout << "; Out of bounds, rendering blank image..." << std::endl;
    } else {
      std::cout << "Rendering component: " << flagsFromCompList[i] << std::endl;
      glUniform1f(glGetUniformLocation(programId, "blendRgbComp"), 0.f);
      glUniform1f(glGetUniformLocation(programId, "blendOrig"), 0.f);
      if (flagsFromCompList[i] == RGB) {
        std::cout << "Rendering RGB..." << std::endl;
        glUniform1i(glGetUniformLocation(programId, "compIdx"), 0);
        glUniform1f(glGetUniformLocation(programId, "blendOrig"), 1.f);
      } else if (flagsFromCompList[i] < RGB) {
        std::cout << "flagsFromCompList 1 : " << flagsFromCompList[i] << std::endl;
        glUniform1i(glGetUniformLocation(programId, "compIdx"), flagsFromCompList[i]);
      } else {
        std::cout << "flagsFromCompList 2 : " << (flagsFromCompList[i] - RED) << std::endl;
        glUniform1i(glGetUniformLocation(programId, "compIdx"), flagsFromCompList[i] - RED);
        glUniform1f(glGetUniformLocation(programId, "blendRgbComp"), 1.f);
      }
    }
    // TODO: render blank geometry instead of a texture
    if (myConfig.rows * myConfig.cols > myConfig.numComps && i == vertexBuffers.size() - 1) {
      glUniform1f(glGetUniformLocation(programId, "alpha"), 0.f);
    } else {
      glUniform1f(glGetUniformLocation(programId, "alpha"), 1.f);
    }

    vertexBuffers[i].bind();

    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, (4 * sizeof(float)), (void *) nullptr);

    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, (4 * sizeof(float)), (void *) (2 * sizeof(float)));

    glDrawArrays(GL_TRIANGLES, 0, 6);
  }

  if (myConfig.separate_img_write)
    separateScreenshot(vertexBuffers, w, h, flagsFromCompList);
  else
    screenshotSingleBuffer(fullWidth, fullHeight);

  glDeleteFramebuffers(1, &framebufferTarget);
  framebufferTexture.free();
  imageTexture.free();

  return SUCCESS;
}

std::pair<ExitReason, GLFWwindow *> createOpenGlContext() {
  if (!glfwInit()) {
    std::cerr << "Unable to initialize GLFW" << std::endl;
    return {GLFW_INIT_FAILURE, nullptr};
  }

  glfwSetErrorCallback(glErrCallback);

  setWindowHints();
  GLFWwindow *window = glfwCreateWindow(1, 1, "", nullptr, nullptr);

  if (!window) {
    std::cerr << "Unable to create GLFW window" << std::endl;
    return {GLFW_WINDOW_INIT_FAILURE, nullptr};
  }

  glfwMakeContextCurrent(window);

  if (!gladLoadGLLoader((GLADloadproc) glfwGetProcAddress)) {
    std::cerr << "Failed to initialize GLAD" << std::endl;
    return {GLAD_INIT_FAILURE, nullptr};
  }
  return {SUCCESS, window};
}

int main(int argc, char **argv) {
  startTimeMs = getTimeMs();

  if (parseArgsFailed(argc, argv, myConfig)) {
    return ARG_PARSE_EXIT;
  }

  auto contextResult = createOpenGlContext();

  if (contextResult.first != SUCCESS)
    return contextResult.first;

  auto window = contextResult.second;

  auto flagsFromCompList = get_flags_from_comp_list(myConfig.compList,
                                                                myConfig.numComps);
  if (flagsFromCompList.empty())
    return COMP_FLAGS_UNREADABLE;

  std::cout << "numComps is " << myConfig.numComps << std::endl;

  if (std::find(flagsFromCompList.begin(), flagsFromCompList.end(), RGB) != flagsFromCompList.end()) {
    myConfig.use_8_bit_depth = false;
  } else {
    myConfig.numChannels = 1;
  }

  std::cout << "USE_8_BIT_DEPTH? " << myConfig.use_8_bit_depth << std::endl;

  // Filters out duplicates for non-collaged images
  if (myConfig.separate_img_write) {
    filterDuplicates(flagsFromCompList);
  } else {
    if (flagsFromCompList.size() > myConfig.rows * myConfig.cols) {
      // TODO: make this more clear
      std::cerr << "Error: the number of components cannot exceed the specified dimensions" << std::endl;
      return COMPONENTS_EXCEEDS_DIMENSIONS;
    }
  }

  glEnable(GL_BLEND);
  glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

  GLint dims[2];
  glGetIntegerv(GL_MAX_VIEWPORT_DIMS, &dims[0]);
  maxWidth = dims[0];
  maxHeight = dims[1];

  auto programId = LoadShaders("vertex.glsl", "frag.glsl");

  glUseProgram(programId);
  glUniform1i(glGetUniformLocation(programId, "texImage"), 0);

  stbi_set_flip_vertically_on_load(false);

  int exitReason = SUCCESS;

  if (myConfig.listFileName.empty()) {
    exitReason = processImage(myConfig.inputFile, flagsFromCompList, programId);
  } else {
    myConfig.imageList = get_string_list_from_file(myConfig.listFileName);
    for (auto file: myConfig.imageList) {
      myConfig.outputFile = getOutputFileName(file, myConfig);
      exitReason = processImage(file, flagsFromCompList, programId);
      if (exitReason != SUCCESS)
        break;
    }
  }

  glDeleteProgram(programId);

  glfwDestroyWindow(window);
  glfwTerminate();

  std::cout << "\nProcessing everything took " << (getTimeMs() - startTimeMs) << " ms." << std::endl;

  return exitReason;
}
