#pragma once

#include <QWidget>
#include <glm/vec3.hpp>

#include <cmath>
#include <filesystem>

static const float PI = 3.14159265358979323846f;

/// Float approximate-equality comparison
template <typename T> inline bool fequal(T a, T b, T epsilon = 0.0001) {
  if (a == b) {
    // Shortcut
    return true;
  }

  const T diff = std::abs(a - b);
  if (a * b == 0) {
    // a or b or both are zero; relative error is not meaningful here
    return diff < (epsilon * epsilon);
  }

  return diff / (std::abs(a) + std::abs(b)) < epsilon;
}

namespace utils {
float getRandom();
glm::vec3 getRandomColor();
bool verifyUsdFile(QWidget *parent, const std::filesystem::path &filePath);
} // namespace utils
