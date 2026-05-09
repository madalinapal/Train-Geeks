#pragma once

#include "utils/glm_utils.h"


namespace transform2D
{
    // Translate matrix
    inline glm::mat3 Translate(float translateX, float translateY)
    {
        // 2d translation matrix
        // [ 1  0  tx ]
        // [ 0  1  ty ]
        // [ 0  0   1 ]
        // TODO(student): Implement the translation matrix
        return glm::mat3(
            1, 0, 0,
            0, 1, 0,
            translateX, translateY, 1);

    }

    // Scale matrix
    inline glm::mat3 Scale(float scaleX, float scaleY)
    {
        // 2d scaling matrix
        // [ sx  0   0 ]
        // [ 0  sy   0 ]
        // [ 0   0   1 ]
        // TODO(student): Implement the scaling matrix
        return glm::mat3(
            scaleX, 0, 0,
            0, scaleY, 0,
            0, 0, 1);
    }

    // Rotate matrix
    inline glm::mat3 Rotate(float radians)
    {
        // 2d rotation matrix (counterclockwise)
        // [ cosθ  -sinθ   0 ]
        // [ sinθ   cosθ   0 ]
        // [  0      0     1 ]
        float c = cos(radians);
        float s = sin(radians);
        // TODO(student): Implement the rotation matrix
        return glm::mat3(
            c, s, 0,
            -s, c, 0,
            0, 0, 1
        );

    }
}   // namespace transform2D
