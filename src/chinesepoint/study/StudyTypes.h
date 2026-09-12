#pragma once

#include <cstdint>

namespace ChinesePoint::Study {

// Values are part of the versioned Study wire formats. Do not reorder them.
enum class CardState : uint8_t { New = 0, Learning = 1, Review = 2, Relearning = 3 };
enum class Rating : uint8_t { Again = 1, Hard = 2, Good = 3, Easy = 4 };

}  // namespace ChinesePoint::Study
