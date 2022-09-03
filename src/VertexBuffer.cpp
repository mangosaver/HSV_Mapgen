//
// Created by Cale on 5/2/2022.
//

#include <iostream>

#include "VertexBuffer.h"

VertexBuffer::VertexBuffer(uint16_t x, uint16_t y, uint16_t width, uint16_t height) : x(x), y(y) {
  glGenBuffers(1, &id);
  glBindBuffer(GL_ARRAY_BUFFER, id); // TODO: remove bind call here
  auto w = (GLfloat) width;
  auto h = (GLfloat) height;
  auto x2 = (GLfloat) x;
  auto y2 = (GLfloat) y;
  vertices = {
      x2, y2, 0, 0,
      x2, y2 + h, 0, 1,
      x2 + w, y2 + h, 1, 1,
      x2, y2, 0, 0,
      x2 + w, y2 + h, 1, 1,
      x2 + w, y2, 1, 0
  };
  glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), &vertices, GL_STATIC_DRAW);
}

void VertexBuffer::bind() const {
  glBindBuffer(GL_ARRAY_BUFFER, id);
}

VertexBuffer::~VertexBuffer() = default;

int VertexBuffer::getX() const {
  return x;
}

int VertexBuffer::getY() const {
  return y;
}