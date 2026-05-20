#pragma once

#include <M5Cardputer.h>
#include <stdint.h>

// Renders the boot splash (Dwight pixel-art portrait + title) onto the
// shared canvas and blocks for `duration_ms`. Theme must already be
// initialized — colors are read from gTheme.
void splashShow(M5Canvas* canvas, uint32_t duration_ms);
