#pragma once

#include "cwsdk.h"
#include "cube/constants.h"
#include <cmath>

namespace TogetherSpawn {
namespace Utils {

    using cube::DOTS_PER_BLOCK;

    class MathUtils {
    public:
        static constexpr double PI = 3.14159265358979323846;

        static inline float DegToRad(float degrees) {
            return degrees * static_cast<float>(PI / 180.0);
        }

        static inline float RadToDeg(float radians) {
            return radians * static_cast<float>(180.0 / PI);
        }

        static inline double Distance(const LongVector3& a, const LongVector3& b) {
            double dx = static_cast<double>(a.x - b.x);
            double dy = static_cast<double>(a.y - b.y);
            double dz = static_cast<double>(a.z - b.z);
            return std::sqrt(dx * dx + dy * dy + dz * dz);
        }

        static inline double DistanceInBlocks(const LongVector3& a, const LongVector3& b) {
            return Distance(a, b) / static_cast<double>(DOTS_PER_BLOCK);
        }

        static inline i64 WorldToBlock(i64 worldCoord) {
            return pydiv(worldCoord, DOTS_PER_BLOCK);
        }

        static inline i64 BlockToWorld(i64 blockCoord) {
            return blockCoord * DOTS_PER_BLOCK + (DOTS_PER_BLOCK / 2);
        }

        static inline LongVector3 CalculateRadialOffset(const LongVector3& center, float radiusInBlocks, float angleRadians) {
            i64 offsetDistDots = static_cast<i64>(radiusInBlocks * DOTS_PER_BLOCK);
            i64 offsetX = static_cast<i64>(std::cos(angleRadians) * offsetDistDots);
            i64 offsetY = static_cast<i64>(std::sin(angleRadians) * offsetDistDots);

            return LongVector3(center.x + offsetX, center.y + offsetY, center.z);
        }
    };

} // namespace Utils
} // namespace TogetherSpawn
