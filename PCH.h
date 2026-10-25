// File: PCH.h
#pragma once

// ===========================================================
// Core Skyrim / SKSE Includes for CommonLibSSE-NG 3.x
// ===========================================================

#include <RE/Skyrim.h>
#include <REL/Relocation.h>
#include <SKSE/API.h>
#include <SKSE/SKSE.h>

// The plugin calls SHGetKnownFolderPath / GetProcAddress directly, so include the
// Windows headers here instead of relying on CommonLibSSE to pull them in.
#include <Windows.h>
#include <ShlObj.h>
#include <filesystem>

// With UNICODE defined, wingdi.h renames GetObject to GetObjectW, which breaks
// RE::*::GetObject() members. Undo the rename for the whole project.
#ifdef GetObject
#	undef GetObject
#endif

using namespace std::literals;

// If you are not using trampoline hooks, leave this commented:
// #include <SKSE/Trampoline.h>

// ===========================================================
// Standard Library Includes
// ===========================================================

#include <algorithm>
#include <array>
#include <atomic>
#include <cmath>
#include <cstdint>
#include <exception>
#include <format>
#include <memory>
#include <mutex>
#include <optional>
#include <span>
#include <string>
#include <string_view>
#include <type_traits>
#include <utility>
#include <vector>

// ===========================================================
// spdlog Logging
// ===========================================================

#include <spdlog/spdlog.h>

#ifdef NDEBUG
    #include <spdlog/sinks/basic_file_sink.h>
#else
    #include <spdlog/sinks/msvc_sink.h>
#endif

namespace logger = SKSE::log;

// ===========================================================
// Convenience Macros
// ===========================================================

#define DLLEXPORT __declspec(dllexport)

// NG VariantOffset helper
#define RELOCATION_OFFSET(SE, AE) REL::VariantOffset(SE, AE, 0).offset()

#define RELOCATION_OFFSET2(SE, AE, AE629)                                                                             \
    REL::VariantOffset(                                                                                               \
        SE,                                                                                                           \
        (REL::Module::get().version().compare(SKSE::RUNTIME_SSE_1_6_629) == std::strong_ordering::less) ? AE : AE629, \
        0)                                                                                                            \
        .offset()

// ===========================================================
// Utility
// ===========================================================
namespace util {
    using SKSE::stl::report_and_fail;
}
