#include <iostream>
#include <vector>
#include <algorithm>

#define GLFW_INCLUDE_NONE

#include "glad/glad.h"
#include <GLFW/glfw3.h>

#include "glm/glm.hpp"
#include "glm/gtc/matrix_transform.hpp"

#define STB_IMAGE_IMPLEMENTATION

#include "stb_image.h"

#define STB_IMAGE_WRITE_IMPLEMENTATION

#include "stb_image_write.h"

#include "VertexBuffer.h"
#include "Texture.h"
#include "consts.h"

#include "../utils/parse_utils.h"
#include "../utils/log_utils.h"
#include "../utils/load_shader.h"

#define hasElement(x, y) std::find(x.begin(), x.end(), y) != x.end()
#define doesNotHaveElement(x, y) std::find(x.begin(), x.end(), y) == x.end()
#define uniformLookup(x) glGetUniformLocation(programId, x)

#define VERBOSE false

ProgramConfig myConfig;

long long startTimeMs;

int MAX_HW_WIDTH_SUPPORTED, MAX_HW_HEIGHT_SUPPORTED;

void writeImage(const char *filename, int w, int h, int comp, const GLubyte *data) {
  if (myConfig.writeJpeg)
    stbi_write_jpg(filename, w, h, comp, data, (int) myConfig.jpegQuality);
  else
    stbi_write_png(filename, w, h, comp, data, w * comp);
  std::cout << "Wrote file " << filename << std::endl;
}

void readFramebufferSeparateImgs(const std::vector<VertexBuffer> &buffers, int w, int h, std::vector<int> comps) {
  auto *data = new GLubyte[w * h * myConfig.numChannels];

  for (int i = 0; i < buffers.size(); i++) {
    const auto &buffer = buffers[i];

    glReadPixels(buffer.getX(), buffer.getY(), w, h, getColorFormatFromNumComponents(myConfig.numChannels),
                 GL_UNSIGNED_BYTE,
                 data);

    auto err = glGetError();
    if (err != GL_NO_ERROR) {
      std::cout << "There was an error reading the framebuffer; error code is " << err << std::endl;
      delete[] data;
      return;
    }

    char filename[256];
    auto extStr = myConfig.writeJpeg ? "jpeg" : "png";

    sprintf(filename, "%s_%s.%s", myConfig.outputFile.c_str(), COMP_NAME_MAP[comps[i]], extStr);
    writeImage(filename, w, h, myConfig.numChannels, data);
  }
  delete[] data;
}

void readFramebufferCollage(unsigned int w, unsigned int h) {
  auto *data = new GLubyte[w * h * myConfig.numChannels];

  glReadPixels(0, 0, (int) w, (int) h, getColorFormatFromNumComponents(myConfig.numChannels), GL_UNSIGNED_BYTE, data);

  auto err = glGetError();
  if (err != GL_NO_ERROR) {
    std::cout << "There was an error reading the framebuffer; error code is " << err << std::endl;
    return;
  }

  char filename[256];

  auto fileExt = myConfig.writeJpeg ? "jpeg" : "png";

  if (myConfig.inputFile == myConfig.outputFile)
    sprintf(filename, "%s_map.%s", myConfig.outputFile.c_str(), fileExt);
  else
    sprintf(filename, "%s.%s", myConfig.outputFile.c_str(), fileExt);

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
  if (VERBOSE)
    std::cout << "Processing image " << file << std::endl;

  int width, height, comp;

  unsigned char *image = stbi_load(file.c_str(), &width, &height, &comp, STBI_rgb_alpha);

  if (myConfig.separateImgWrite) {
    myConfig.rows = 1;
    myConfig.cols = (int) flagsFromCompList.size();
  }

  if (VERBOSE)
    std::cout << "Image dimensions: " << width << " x " << height << std::endl;

  if (!image) {
    if (strcmp(stbi_failure_reason(), "can't fopen") == 0) {
      std::cout << "Unable to open file \"" << file << "\"" << std::endl;
      return IMAGE_NOT_FOUND;
    }
    std::cerr << "Unknown error loading image" << std::endl;
    return IMAGE_LOAD_ERR;
  }

  const auto bufferWidth = width * myConfig.cols;
  const auto bufferHeight = height * myConfig.rows;

  if (bufferWidth > MAX_HW_WIDTH_SUPPORTED || bufferHeight > MAX_HW_HEIGHT_SUPPORTED) {
    std::cerr << "Error: the size of the generated framebuffer (" << bufferWidth
              << " x " << bufferHeight << ") exceeds this machine's limit (" << MAX_HW_WIDTH_SUPPORTED
              << " x " << MAX_HW_HEIGHT_SUPPORTED << ")" << std::endl;
    return MAX_VIEWPORT_SIZE_EXCEEDED;
  }

  glViewport(0, 0, bufferWidth, bufferHeight);

  myConfig.numChannels = comp;

  Texture imageTexture, framebufferTexture;

  imageTexture.uploadData(width, height, image);
  stbi_image_free(image);

  unsigned int vao;
  glGenVertexArrays(1, &vao);
  glBindVertexArray(vao);

  GLuint framebufferTarget;
  glGenFramebuffers(1, &framebufferTarget);
  glBindFramebuffer(GL_FRAMEBUFFER, framebufferTarget);

  framebufferTexture.uploadData(bufferWidth, bufferHeight, nullptr);

  glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, framebufferTexture.id, 0);

  const GLenum buffers[]{GL_COLOR_ATTACHMENT0};
  glDrawBuffers(1, buffers);

  if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
    std::cerr << "Unable to create framebuffer target: error code is " << glGetError() << std::endl;
    return FRAMEBUFFER_CREATION_FAILURE;
  }

  auto projMatrix = glm::ortho(0.f, (float) bufferWidth, 0.f, (float) bufferHeight, 0.0f, 100.0f);
  glUniformMatrix4fv(glGetUniformLocation(programId, "MVP"), 1, GL_FALSE, &projMatrix[0][0]);

  // Generate buffers
  std::vector<VertexBuffer> vertexBuffers;
  vertexBuffers.reserve(myConfig.rows * myConfig.cols);

  for (int i = 0; i < myConfig.rows; i++) {
    for (int j = 0; j < myConfig.cols; j++) {
      auto tmp = VertexBuffer(width * j, height * i, width, height);
      vertexBuffers.push_back(tmp);
    }
  }

  glActiveTexture(GL_TEXTURE0);
  imageTexture.bind();

  // Render buffers
  for (int i = 0; i < vertexBuffers.size(); i++) {
    if (i >= flagsFromCompList.size())
      continue;

    if (VERBOSE)
      std::cout << "Rendering component: " << flagsFromCompList[i] << std::endl;
    glUniform1f(uniformLookup("blendRgbComp"), 0.f);
    glUniform1f(uniformLookup("blendOrig"), 0.f);
    if (flagsFromCompList[i] == RGB) {
      glUniform1i(uniformLookup("compIdx"), 0);
      glUniform1f(uniformLookup("blendOrig"), 1.f);
    } else if (flagsFromCompList[i] < RGB) {
      glUniform1i(uniformLookup("compIdx"), flagsFromCompList[i]);
    } else {
      glUniform1i(uniformLookup("compIdx"), flagsFromCompList[i] - RED);
      glUniform1f(uniformLookup("blendRgbComp"), 1.f);
    }

    if (myConfig.rows * myConfig.cols > myConfig.numComps && i == vertexBuffers.size() - 1) {
      glUniform1f(uniformLookup("alpha"), 0.f);
    } else {
      glUniform1f(uniformLookup("alpha"), 1.f);
    }

    vertexBuffers[i].bind();

    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, (4 * sizeof(float)), (void *) nullptr);

    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, (4 * sizeof(float)), (void *) (2 * sizeof(float)));

    glDrawArrays(GL_TRIANGLES, 0, 6);
  }

  if (myConfig.separateImgWrite)
    readFramebufferSeparateImgs(vertexBuffers, width, height, flagsFromCompList);
  else
    readFramebufferCollage(bufferWidth, bufferHeight);

  glDeleteFramebuffers(1, &framebufferTarget);
  framebufferTexture.free();
  imageTexture.free();

  return SUCCESS;
}

std::pair<ExitReason, GLFWwindow *> createOpenGlContext() {
  glfwSetErrorCallback(glErrCallback);

  if (!glfwInit()) {
    std::cerr << "Unable to initialize GLFW" << std::endl;
    return {GLFW_INIT_FAILURE, nullptr};
  }

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
    if (!myConfig.helped)
      std::cout << "\nTry hsv_mapgen --help to view usage guide\n" << std::endl;
    return ARG_PARSE_EXIT;
  }

  auto created = createOpenGlContext();

  if (created.first != SUCCESS) {
    glfwTerminate();
    return created.first;
  }

  auto window = created.second;

  auto flagsFromCompList = getFlagsFromCompList(myConfig.compList,
                                                myConfig.numComps);
  if (flagsFromCompList.empty())
    return COMP_FLAGS_UNREADABLE;

  if (doesNotHaveElement(flagsFromCompList, RGB)) {
    myConfig.numChannels = 1;
  }

  // Filters out duplicates for non-collaged images
  if (myConfig.separateImgWrite) {
    filterDuplicates(flagsFromCompList);
  } else {
    if (flagsFromCompList.size() > myConfig.rows * myConfig.cols) {
      std::cerr << "Warning: the provided dimensions will not fit the number of components (" <<
      flagsFromCompList.size() << ")" << std::endl;
    }
  }

  glEnable(GL_BLEND);
  glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

  GLint dims[2];
  glGetIntegerv(GL_MAX_VIEWPORT_DIMS, &dims[0]);
  MAX_HW_WIDTH_SUPPORTED = dims[0];
  MAX_HW_HEIGHT_SUPPORTED = dims[1];

  auto programId = createShaderProgram();

  if (!programId) {
    std::cout << "Failed to create shader program" << std::endl;
    return SHADER_COMPILATION_FAILED;
  }

  glUseProgram(programId);
  glUniform1i(glGetUniformLocation(programId, "texImage"), 0);

  stbi_set_flip_vertically_on_load(false);

  int exitReason = SUCCESS;

  if (myConfig.listFileName.empty()) {
    exitReason = processImage(myConfig.inputFile, flagsFromCompList, programId);
  } else {
    myConfig.imageList = getStringListFromFile(myConfig.listFileName);
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

  if (VERBOSE)
    std::cout << "\nProcessing everything took " << (getTimeMs() - startTimeMs) << " ms." << std::endl;

  return exitReason;
}
