#pragma once

/// @file icetea.h
/// @brief Umbrella header for the icetea library (ICU4X C++ wrapper).

#include "icetea/exports.h"
#include "icetea/types.h"
#include "icetea/icu4x_bridge.h"

namespace icetea {

/// Return the library version string.
[[nodiscard]] ICETEA_API const char* version() noexcept;

}  // namespace icetea
