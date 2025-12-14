#pragma once

// Force fmt to use runtime formatting only; avoid consteval/compile-time paths that
// break under clang-cl when fmt/compile.h is pulled in by the engine headers.
// Return fmt::string_view to bypass FMT_COMPILE_STRING type that triggers constexpr checks.
#ifndef FMT_COMPILE
#    define FMT_COMPILE(s) fmt::string_view(s)
#endif
#ifndef FMT_HEADER_ONLY
#    define FMT_HEADER_ONLY 1
#endif
#ifndef FMT_USE_CONSTEXPR20
#    define FMT_USE_CONSTEXPR20 0
#endif
#ifndef FMT_USE_CONSTEVAL
#    define FMT_USE_CONSTEVAL 0
#endif
#ifndef FMT_HAS_CONSTEVAL
#    define FMT_HAS_CONSTEVAL 0
#endif
#ifndef FMT_CONSTEVAL
#    define FMT_CONSTEVAL
#endif

#include <axmol.h>

namespace DevTools {
    // Core UI control
    void open();
    void close();
    void toggle();
    bool isOpen();
    
    // Input handling
    void handleKeyPress(int keyCode);
    void update(float dt);
}
