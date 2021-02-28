#pragma once

#include <renderer/point.h>
#include <stdint.h>

namespace renderer {
    class MouseUtils {
        private:
            uint8_t* mouse_pointer;

        public:
            MouseUtils();
            uint8_t* get_mouse_pointer();
            renderer::point_t mouse_position;
            renderer::point_t mouse_position_old;
    };

    extern MouseUtils* global_mouse_utils;
}