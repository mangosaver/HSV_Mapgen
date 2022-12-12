#include <string>
#include <iostream>

#include "parse_utils.h"
#include "config_utils.h"
#include "print_help.h"

bool parseArgsFailed(int argc, char **argv, struct ProgramConfig& config) {
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
      config.inputFile = argv[i + 1];
      i++;
    } else if (arg == "-I") {
      if (i + 1 == argc) {
        std::cout << "Expected text file name" << std::endl;
        return true;
      }
      config.listFileName = argv[i + 1];
      i++;
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
      config.cols = result.first;
      config.rows = result.second;
      config.separateImgWrite = false;
      i++;
    } else if (arg == "-j" || arg == "--jpeg" || arg == "--jpg") {
      if (i + 1 != argc) {
        // parse jpeg quality
        auto maybeJpegQualityStr = argv[i + 1];
        int maybeJpegQuality = std::atoi(maybeJpegQualityStr);
        if (maybeJpegQuality >= 1) {
          config.jpegQuality = maybeJpegQuality;
          i++;
        }
      }
      config.writeJpeg = true;
    } else if (arg == "-o") {
      if (i + 1 == argc) {
        std::cout << "Expected filename" << std::endl;
        return true;
      }
      config.outputFile = argv[i + 1];
      i++;
    } else if (arg == "-c") {
      if (i + 1 == argc) {
        std::cout << "Expected component list" << std::endl;
        return true;
      }
      config.compList = argv[i + 1];
      i++;
    } else {
      std::cout << "Unknown flag '" << arg << "'\nTry hsv_map --help to view usage guide" << std::endl;
      return true;
    }
  }

  if (config.listFileName.empty()) {
    if (config.inputFile.empty()) {
      std::cout << "Please provide an image filename with -i or an image list filename with -I" << std::endl;
      return true;
    }
    config.outputFile = getOutputFileName(config.inputFile, config);
  } else {
    // warn the user if they use both -o and -I
    if (!config.outputFile.empty()) {
      std::cout << "Warning: the specified output file name will not be used with an image list (-I flag)" << std::endl;
    }
  }

  return false;
}
