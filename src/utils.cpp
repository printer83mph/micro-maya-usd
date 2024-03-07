#include "utils.h"

float utils::getRandom() { return (float)rand() / (float)RAND_MAX; }

glm::vec3 utils::getRandomColor() {
  return glm::vec3(getRandom(), getRandom(), getRandom());
}
