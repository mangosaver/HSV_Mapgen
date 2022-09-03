#include <iostream>
#include <vector>
#include <chrono>

#define GLFW_INCLUDE_NONE

#include "../glad/glad.h"
#include <GLFW/glfw3.h>

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

#include "printHelp.h"

// TODO: create config struct

struct config {
    bool separate_img_write;
    bool single_channel;
    int rows, cols;
    std::string inputFile;
    std::string outputFile;
    std::string compList;
};

bool WRITE_JPEG = false;
int jpegQuality = 100;

long long startTimeMs;

int maxWidth, maxHeight;

bool SEPARATE_IMG_WRITE = true;
bool USE_8_BIT_DEPTH = true; // for optimizing image size

int rows = 1, cols = 3;

std::vector<std::string> imageList;

std::string listFileName;
std::string fileName;
std::string outputFile;

std::string compList = "hue:sat:val";

int numChannels = 4;

int numComps = 0;

GLuint framebufferTarget;

std::string getOutputFileName(const std::string& inFile) {
  std::string tmp = stripFileExt(inFile);
  std::cout << "Stripped to " << tmp << std::endl;
  if (tmp.empty()) { // handles cases where the input file is ".png" or similar
    return getTimestampStr();
  } else if (!SEPARATE_IMG_WRITE) {
    tmp += "_" + getTimestampStr();
  }
  return tmp;
}

void error_callback(int error, const char *description) {
  fprintf(stderr, "Error: (%d) %s\n", error, description);
}

void writeImage(const char *filename, int w, int h, int comp, const GLubyte *data) {
  if (WRITE_JPEG)
    stbi_write_jpg(filename, w, h, comp, data, jpegQuality);
  else
    stbi_write_png(filename, w, h, comp, data, w * comp);
  std::cout << "Wrote file " << filename << std::endl;
}

void separateScreenshot(const std::vector<VertexBuffer> &buffers, int w, int h, std::vector<int> comps) {
  std::cout << "Taking separateScreenshot" << std::endl;

  // TODO: put this in utils
  const char *names[] = {"hue", "sat", "val", "rgb", "red", "green", "blue"};

  auto *data = new GLubyte[w * h * numChannels];

  for (int i = 0; i < buffers.size(); i++) {
    auto buffer = buffers[i];

    if (USE_8_BIT_DEPTH) {
      glReadPixels(buffer.getX(), buffer.getY(), w, h, GL_RED, GL_UNSIGNED_BYTE, data);
    } else {
      glReadPixels(buffer.getX(), buffer.getY(), w, h, getColorFormatFromNumComponents(numChannels), GL_UNSIGNED_BYTE,
                   data);
    }

    auto err = glGetError();
    if (err != GL_NO_ERROR) {
      std::cout << "[1] There was an error reading the framebuffer, gl err " << err << std::endl;
      delete[] data;
      return;
    }

    char filename[256];
    auto extStr = WRITE_JPEG ? "jpeg" : "png";

    sprintf(filename, "%s_%s.%s", outputFile.c_str(), names[comps[i]], extStr);
    if (USE_8_BIT_DEPTH) {
      writeImage(filename, w, h, 1, data);
    } else {
      writeImage(filename, w, h, numChannels, data);
    }
  }
  delete[] data;
}

void screenshotSingleBuffer(unsigned int w, unsigned int h) {

  std::cout << "Taking screenshotSingleBuffer: " << w << " x " << h << std::endl;

  auto *data = new GLubyte[w * h * numChannels];

  if (USE_8_BIT_DEPTH) {
    glReadPixels(0, 0, w, h, GL_RED, GL_UNSIGNED_BYTE, data);
  } else {
    glReadPixels(0, 0, w, h, getColorFormatFromNumComponents(numChannels), GL_UNSIGNED_BYTE, data);
  }

  auto err = glGetError();
  if (err != GL_NO_ERROR) {
    std::cout << "[2] There was an error reading the framebuffer, err is " << err << std::endl;
  }

  // TODO: validate that the filename cannot exceed 256 characters
  char filename[256];

  auto extStr = WRITE_JPEG ? "jpeg" : "png";

  if (fileName == outputFile)
    sprintf(filename, "%s_map.%s", outputFile.c_str(), extStr);
  else
    sprintf(filename, "%s.%s", outputFile.c_str(), extStr);

  writeImage(filename, w, h, numChannels, data);

  delete[] data;
}

void setWindowHints() {
  glfwWindowHint(GLFW_VISIBLE, GLFW_FALSE);
  glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3); // OpenGL 3.3
  glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
  glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
}

// TODO: put this in separate file, set a config struct by reference
bool parseArgsFailed(int argc, char **argv) {
  for (int i = 1; i < argc; i++) {
    const auto arg = std::string(argv[i]);
    if (arg == "-h" || arg == "--help") {
      printHelp();
      return true;
    } else if (arg == "-i" || arg == "--input") {
      if (i + 1 == argc) {
        std::cout << "Expected image name" << std::endl;
        return true;
      }
      fileName = argv[i + 1];
      i++;
    } else if (arg == "-I") {
      if (i + 1 == argc) {
        std::cout << "Expected text file name" << std::endl;
        return true;
      }
      listFileName = argv[i + 1];
      i++;
      std::cout << "Reading from " << listFileName << std::endl;
    } else if (arg == "-d" || arg == "--dimensions") {
      if (i + 1 == argc) {
        std::cout << "Expected dimensions (i.e. 1x3, 2x2) following '" << arg << "'" << std::endl;
        return true;
      }
      std::string dims = argv[i + 1];
      auto result = parseDimsSuccess(dims);
      if (result.first < 1 || result.second < 1) {
        std::cerr << "Dimensions must be 2 positive, non-zero values separated by an 'x'" << std::endl;
        return true;
      }
      cols = result.first;
      rows = result.second;
      SEPARATE_IMG_WRITE = false;
      i++;
    } else if (arg == "-j" || arg == "--jpeg" || arg == "--jpg") {
      if (i + 1 != argc) {
        // parse jpeg quality
        auto maybeJpegQualityStr = argv[i + 1];
        int maybeJpegQuality = std::atoi(maybeJpegQualityStr);
        if (maybeJpegQuality >= 1) {
          jpegQuality = maybeJpegQuality;
          i++;
        }
      }
      WRITE_JPEG = true;
    } else if (arg == "-o") {
      if (i + 1 == argc) {
        std::cout << "Expected filename" << std::endl;
        return true;
      }
      outputFile = argv[i + 1];
      i++;
    } else if (arg == "-c") {
      if (i + 1 == argc) {
        std::cout << "Expected component list" << std::endl;
        return true;
      }
      compList = argv[i + 1];
      i++;
    } else {
      std::cout << "Unknown flag '" << arg << "'\nTry hsv_map --help to view usage guide" << std::endl;
      return true;
    }
  }

  if (listFileName.empty()) {
    if (fileName.empty()) {
      std::cout << "Please provide an image filename with -i or an image list filename with -I" << std::endl;
      return true;
    }
    outputFile = getOutputFileName(fileName);
  } else {
    // warn the user if they use both -o and -I
    if (!outputFile.empty()) {
      std::cout << "Warning: the specified output file name will not be used with an image list (-I flag)" << std::endl;
    }
  }

  return false;
}

int processImage(std::string& file, std::vector<int>& flagsFromCompList, GLuint programId) {

  std::cout << "Processing image: " << file << std::endl;

  // stbi image loading
  // TODO: abstract this, add support for image list
  int w, h, comp;

  unsigned char *image = stbi_load(file.c_str(), &w, &h, &comp, STBI_rgb_alpha);

  if (!image) {
    if (strcmp(stbi_failure_reason(), "can't fopen") == 0) {
      std::cout << "Unable to open file \"" << fileName << "\"" << std::endl;
      return IMAGE_NOT_FOUND;
    }
    std::cerr << "Unknown error loading image" << std::endl;
    return IMAGE_LOAD_ERR;
  }

  const auto fullWidth = w * cols;
  const auto fullHeight = h * rows;

  if (fullWidth > maxWidth || fullHeight > maxHeight) {
    std::cerr << "Error: the size of the generated framebuffer (" << fullWidth
              << " x " << fullHeight << ") exceeds this machine's limit (" << maxWidth
              << " x " << maxHeight << ")" << std::endl;
    return MAX_VIEWPORT_SIZE_EXCEEDED;
  }

  glViewport(0, 0, fullWidth, fullHeight);

  numChannels = comp;

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

  if (SEPARATE_IMG_WRITE) {
    rows = 1;
    cols = flagsFromCompList.size();
  }

  // Generate buffers
  std::vector<VertexBuffer> vertexBuffers; // TODO: validate that this is correct
  vertexBuffers.reserve(rows * cols);

  for (int i = 0; i < rows; i++) {
    for (int j = 0; j < cols; j++) {
      auto tmpBuffer = VertexBuffer(w * j, h * i, w, h);
      vertexBuffers.push_back(tmpBuffer);
    }
  }

  glActiveTexture(GL_TEXTURE0);
  imageTexture.bind();

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
    if (rows * cols > numComps && i == vertexBuffers.size() - 1) {
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

  if (SEPARATE_IMG_WRITE)
    separateScreenshot(vertexBuffers, w, h, flagsFromCompList);
  else
    screenshotSingleBuffer(fullWidth, fullHeight);

  glDeleteFramebuffers(1, &framebufferTarget);
  framebufferTexture.free();
  imageTexture.free();

  return SUCCESS;
}

int main(int argc, char **argv) {
  startTimeMs = getTimeMs();

  if (parseArgsFailed(argc, argv)) {
    return ARG_PARSE_EXIT;
  }

  if (!glfwInit()) {
    std::cerr << "Unable to initialize GLFW" << std::endl;
    return GLFW_INIT_FAILURE;
  }

  glfwSetErrorCallback(error_callback);

  setWindowHints();
  GLFWwindow *window = glfwCreateWindow(1, 1, "", nullptr, nullptr);

  if (!window) {
    std::cerr << "Unable to create GLFW window" << std::endl;
    return GLFW_WINDOW_INIT_FAILURE;
  }

  glfwMakeContextCurrent(window);

  if (!gladLoadGLLoader((GLADloadproc) glfwGetProcAddress)) {
    std::cerr << "Failed to initialize GLAD" << std::endl;
    return GLAD_INIT_FAILURE;
  }

  std::vector<int> flagsFromCompList = get_flags_from_comp_list(compList, numComps);// = get_flags_from_comp_list();
  if (flagsFromCompList.empty()) {
    return COMP_FLAGS_UNREADABLE;
  }

  std::cout << "numComps is " << numComps << std::endl;

  if (std::find(flagsFromCompList.begin(), flagsFromCompList.end(), RGB) != flagsFromCompList.end()) {
    USE_8_BIT_DEPTH = false;
  }

  std::cout << "USE_8_BIT_DEPTH? " << USE_8_BIT_DEPTH << std::endl;

  // Filters out duplicates for non-collaged images
  if (SEPARATE_IMG_WRITE) {
    filterDuplicates(flagsFromCompList);
  } else {
    if (flagsFromCompList.size() > rows * cols) {
      // TODO: make this more clear
      std::cerr << "Error: the number of components must be <= the specified dimensions" << std::endl;
      return COMPONENTS_EXCEEDS_DIMENSIONS;
    }
  }

  glEnable(GL_BLEND);
  glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

  GLint dims[2];
  glGetIntegerv(GL_MAX_VIEWPORT_DIMS, &dims[0]);
  maxWidth = dims[0];
  maxHeight = dims[1];

  // FIXME: this is just for testing
  GLuint programId = LoadShaders("vertex.glsl", "frag.glsl");

  glUseProgram(programId);
  glUniform1i(glGetUniformLocation(programId, "texImage"), 0);

  stbi_set_flip_vertically_on_load(false);

  int exitReason = SUCCESS;

  if (listFileName.empty()) {
    exitReason = processImage(fileName, flagsFromCompList, programId);
  } else {
    imageList = get_string_list_from_file(listFileName);
    for (auto file: imageList) {
      outputFile = getOutputFileName(file);
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
