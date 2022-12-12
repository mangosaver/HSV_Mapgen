#include <chrono>
#include <unordered_set>
#include <iostream>
#include <fstream>
#include <string>

#include "parse_utils.h"
#include "config_utils.h"
#include "log_utils.h"
#include "../glad/glad.h"

std::string getOutputFileName(const std::string &inFile, const struct ProgramConfig &myConfig) {
  std::string tmp = stripFileExt(inFile);
  std::cout << "Stripped to " << tmp << std::endl;
  if (tmp.empty()) { // handles cases where the input file is ".png" or similar
    return getTimestampStr();
  } else if (!myConfig.separateImgWrite) {
    tmp += "_" + getTimestampStr();
  }
  return tmp;
}

std::vector<std::string> getStringListFromFile(const std::string &file) {
  std::ifstream inStream(file);

  if (!inStream.is_open()) {
    std::cout << "Unable to open file \"" << file << "\"" << std::endl;
    return {};
  }

  std::vector<std::string> out;

  for (std::string line; getline(inStream, line);) {
    std::cout << "Adding " << line << "..." << std::endl;
    out.push_back(line);
  }

  return out;
}

Component mapStringToFlag(const std::string &str) {
  if (str == "hue" || str == "h")
    return HUE;
  if (str == "sat" || str == "saturation" || str == "s")
    return SATURATION;
  if (str == "val" || str == "value" || str == "lightness" || str == "v")
    return VALUE;
  if (str == "orig" || str == "normal" || str == "rgb" || str == "o" || str == "n")
    return RGB;
  if (str == "red" || str == "r")
    return RED;
  if (str == "green" || str == "g")
    return GREEN;
  if (str == "blue" || str == "b")
    return BLUE;
  return INVALID;
}

std::vector<int> getFlagsFromCompList(const std::string &compList, int &numComps) {
  std::string current;
  std::vector<std::string> strs;

  for (char c: compList) {
    if (std::isalpha(c)) {
      current += c;
    } else if (c == ',' || c == ';' || c == ':') {
      strs.push_back(current);
      current = "";
    } else {
      std::cerr << "Illegal character '" << c << "' in component list" << std::endl;
      return {};
    }
  }

  if (!current.empty())
    strs.push_back(current);

  std::vector<int> out;
  numComps = strs.size();

  for (auto &str: strs) {
    if (mapStringToFlag(str) == INVALID) {
      std::cerr << "Invalid component string '" << str << "'" << std::endl;
      return {};
    }
    out.push_back(mapStringToFlag(str));
  }

  return out;
}

std::pair<int, int> parseDimsSuccess(const std::string &dims) {
  if (dims.length() < 3)
    return {-1, -1};

  int w, h;
  std::string wTemp, hTemp;
  bool fillWidth = true;
  for (char dim: dims) {
    if (dim == 'x') {
      if (fillWidth) {
        fillWidth = false;
      } else {
        return {-1, -1};
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
  } catch (const std::invalid_argument &ia) {
    std::cerr << ia.what() << std::endl;
    return {-1, -1};
  }
  return {w, h};
}

void filterDuplicates(std::vector<int> &toFilter) {
  std::unordered_set<int> filtered(toFilter.begin(), toFilter.end());
  toFilter.assign(filtered.begin(), filtered.end());
}

std::string stripFileExt(const std::string &input) {
  std::string out;

  if (input.ends_with(".png") || input.ends_with(".jpg")) {
    out = input.substr(0, input.size() - 4);
  } else if (input.ends_with(".jpeg")) {
    out = input.substr(0, input.size() - 5);
  }

  return out;
}

int getColorFormatFromNumComponents(int numComponents) {
  switch (numComponents) {
    case 4:
      return GL_RGBA;
    case 3:
      return GL_RGB;
    case 2:
      return GL_RG;
    default:
      return GL_RED;
  }
}