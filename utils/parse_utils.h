//
// Created by Cale on 6/5/2022.
//

#ifndef HSV_MAPPER_PARSE_UTILS_H
#define HSV_MAPPER_PARSE_UTILS_H

#include <string>
#include <vector>
#include <iostream>

#include "config_utils.h"

enum Component {
    HUE = 0,
    SATURATION,
    VALUE,
    RGB,
    RED,
    GREEN,
    BLUE,
    INVALID
};

std::string getOutputFileName(const std::string&, const struct ProgramConfig&);

std::vector<std::string> get_string_list_from_file(const std::string&);

Component map_string_to_flag(const std::string&);

std::vector<int> get_flags_from_comp_list(const std::string&, int&);

std::pair<int, int> parseDimsSuccess(const std::string&);

void filterDuplicates(std::vector<int>&);

std::string stripFileExt(const std::string&);

int getColorFormatFromNumComponents(int);

#endif //HSV_MAPPER_PARSE_UTILS_H
