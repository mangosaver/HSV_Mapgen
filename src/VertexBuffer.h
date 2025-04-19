#ifndef HSV_MAPPER_VERTEXBUFFER_H
#define HSV_MAPPER_VERTEXBUFFER_H

#include <array>

#include "glad/glad.h"

class VertexBuffer {
private:
  GLuint id;
  std::array<GLfloat, 24> vertices;
  uint16_t x, y;
public:
  VertexBuffer(uint16_t, uint16_t, uint16_t, uint16_t);
  ~VertexBuffer();
  void bind() const;

  int getX() const;
  int getY() const;
};


#endif //HSV_MAPPER_VERTEXBUFFER_H
