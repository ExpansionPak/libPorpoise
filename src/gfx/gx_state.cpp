#include "gx_state.hpp"

namespace porpoise::gfx::gx {

// Function-local static avoids static init order fiasco (e.g. unordered_map
// constructed before CRT heap ready when linked with PikminDemo)
GXState& g_gxState() {
    static GXState s;
    return s;
}

} // namespace porpoise::gfx::gx

