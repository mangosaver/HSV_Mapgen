//
// Created by Cale on 9/4/2022.
//
#include <iostream>

// Prints the program's usage guide
void printHelp() {
  std::cout << "HSV Map Maker v1.0" << std::endl;

  std::cout << "\nCreates one or more grayscale maps from an image representing its\n"
            << "hue, saturation, and value. Will output separate images by default, but\n"
            << "can be configured to produce a single-image collage of all maps.\n"
            << std::endl;

  std::cout << "Usage: hsv_map -i <image file> [OPTIONS]\n"
            << std::endl;

  std::cout << "Arguments:\n"
            << "  -i <image file>\t\t\tThe file name of the image to be processed\n"
            << "  -I <text file>\t\t\tA text file with a list of images to be batch processed\n"
            << "  -o <file>\t\t\tChanges the output filename (before extension is added, for use with -i only)\n"
            << "  -c <components>\t\tSelect map components as a colon-separated list\n"
            << "\n  \t\t\t\tExample: hsv_map -i painting.png -c [normal:hue:sat:val]\n"
            << "  \t\t\t\t(Default: [hue:sat:val])\n"
            << "  \t\t\t\tValid components: hue, sat, val, orig\n"
            << "  -d, --dimensions <dims>\tCreates a single-image collage with the specified\n"
            << "  -j, --jpeg [quality]\tOutputs a JPEG with an optional quality level from 1-100\n"
            << "  \t\t\t\t(Default: 100)\n"
            << "  \t\t\t\tdimensions. Must be two numbers separated by an 'x', i.e. 1x3, 2x2, etc.\n"
            << "\n  \t\t\t\tExample: hsv_map -i painting.png -c [sat;hue;val] -d 1x3\n"
            << "  \t\t\t\t(Creates a collage of saturation, hue, and value vertically)\n"
            << std::endl;
}