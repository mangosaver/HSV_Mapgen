//
// Created by Cale on 9/3/2022.
//

#ifndef HSV_MAPPER_ARG_PARSE_UTILS_H
#define HSV_MAPPER_ARG_PARSE_UTILS_H

#include "parse_utils.h"
#include "print_help.h"

struct ProgramConfig {
    bool separate_img_write = true;
    bool use_8_bit_depth = true;
    bool write_jpeg = false;
    int rows, cols;
    int numComps = 3, numChannels = 4;
    unsigned int jpeg_quality = 100;
    std::vector<std::string> imageList;
    std::string compList = "hue:sat:val";
    std::string inputFile;
    std::string outputFile;
    std::string listFileName;
};

// TODO: put this in separate file, set a ProgramConfig struct by reference
bool parseArgsFailed(int, char **, struct ProgramConfig &);

#endif //HSV_MAPPER_ARG_PARSE_UTILS_H
