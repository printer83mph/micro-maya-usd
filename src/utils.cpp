#include "utils.h"

#include <glm/gtx/transform.hpp>

float utils::getRandom()
{
    return (float) rand() / (float) RAND_MAX;
}

glm::vec3 utils::getRandomColor()
{
    return glm::vec3(getRandom(), getRandom(), getRandom());
}
