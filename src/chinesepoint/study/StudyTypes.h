#pragma once

#include <cstdint>

namespace ChinesePoint::Study {

// Values are part of the versioned Study wire formats. Do not reorder them.
enum class CardState : uint8_t { New = 0, Learning = 1, Review = 2, Relearning = 3 };

}  // namespace ChinesePoint::Study
