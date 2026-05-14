#pragma once

#include "matrix.hpp"

unsigned int codeKS(const Vec2 &P, float minX, float minY, float maxX, float maxY);

bool clip(Vec2 &A, Vec2 &B, float minX, float minY, float maxX, float maxY);
