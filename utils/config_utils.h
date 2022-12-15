#ifndef HSV_MAPPER_ARG_PARSE_UTILS_H
#define HSV_MAPPER_ARG_PARSE_UTILS_H

#include "parse_utils.h"
#include "print_help.h"

struct ProgramConfig {
    bool separateImgWrite = true;
    bool use8BitDepth = true;
    bool helped = false;
    bool overwriteOutputFilename = false;
    bool writeJpeg = false;
    int rows, cols;
    int numComps = 3, numChannels = 4;
    unsigned int jpegQuality = 100;
    std::vector<std::string> imageList;
    std::string compList = "hue:sat:val";
    std::string inputFile;
    std::string outputFile;
    std::string listFileName;
};

bool parseArgsFailed(int, char **, struct ProgramConfig &);

#endif //HSV_MAPPER_ARG_PARSE_UTILS_H
