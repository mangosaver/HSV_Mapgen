//
// Created by Cale on 5/2/2022.
//

#include "VertexBuffer.h"

#include <stdio.h>

VertexBuffer::VertexBuffer(float x, float y, uint16_t width, uint16_t height) : x(x), y(y) {
  glGenBuffers(1, &id);
  glBindBuffer(GL_ARRAY_BUFFER, id); // TODO: remove bind call here
  auto w = (GLfloat) width;
  auto h = (GLfloat) height;
  printf("Creating buffer with width %f and height %f!\n", w, h);
  vertices = {
      x, y, 0, 0,
      x, y + h, 0, 1,
      x + w, y + h, 1, 1,
      x, y, 0, 0,
      x + w, y + h, 1, 1,
      x + w, y, 1, 0

  };
  glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), &vertices, GL_STATIC_DRAW);
}

void VertexBuffer::bind() const {
  glBindBuffer(GL_ARRAY_BUFFER, id);
}

VertexBuffer::~VertexBuffer() {

}

int VertexBuffer::getX() {
  return x;
}

int VertexBuffer::getY() {
  return y;
}