//
// Created by Cale on 6/5/2022.
//

#ifndef HSV_MAPPER_UTILS_H
#define HSV_MAPPER_UTILS_H

#include <string>
#include <vector>
#include <iostream>

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

long long getTimeMs();

std::vector<std::string> get_string_list_from_file(const std::string&);

Component map_string_to_flag(const std::string&);

std::vector<int> get_flags_from_comp_list(const std::string&, int&);

std::pair<int, int> parseDimsSuccess(const std::string&);

void filterDuplicates(std::vector<int>&);

std::string stripFileExt(const std::string&);

std::string getTimestampStr();

int getColorFormatFromNumComponents(int);

#endif //HSV_MAPPER_UTILS_H
