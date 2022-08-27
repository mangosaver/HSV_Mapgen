//
// Created by Cale on 5/20/2022.
//

#ifndef HSV_MAPPER_TEXTURE_H
#define HSV_MAPPER_TEXTURE_H


#include "../glad/glad.h"

class Texture {
public:
    GLuint id;
    Texture();
    void bind();
    void uploadData(int width, int height, unsigned char *data);
    void free();
};

#endif //HSV_MAPPER_TEXTURE_H
