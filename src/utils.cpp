//
// Created by Cale on 6/5/2022.
//

#include "utils.h"

int map_string_to_flag(const std::string& str) {
  if (str == "hue" || str == "h")
    return 0;
  if (str == "sat" || str == "saturation" || str == "h")
    return 1;
  if (str == "val" || str == "value" || str == "lightness")
    return 2;
  return -1;
}

std::vector<int> get_flags_from_comp_list(const std::string& compList, int& numComps) {
  std::cout << "Components list: " << compList << std::endl;

  std::string current;
  std::vector<std::string> strs;

  for (char c : compList) {
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

  for (int i = 0; i < strs.size(); i++) {
    out.push_back(map_string_to_flag(strs[i]));
    if (out[i] == -1) {
      std::cerr << "Invalid component string '" << strs[i] << "'" << std::endl;
      return {};
    }
  }

  return out;
}