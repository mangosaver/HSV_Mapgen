#ifndef HSV_MAPPER_TEXTURE_H
#define HSV_MAPPER_TEXTURE_H


#include "glad/glad.h"

class Texture {
public:
    GLuint id;
    Texture();
    void bind() const;
    void uploadData(int width, int height, unsigned char *data) const;
    void free();
};

#endif //HSV_MAPPER_TEXTURE_H
