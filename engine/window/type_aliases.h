//
// Created by Kiril on 24.02.2023.
//

#ifndef DEEPLOM_TYPE_ALIASES_H
#define DEEPLOM_TYPE_ALIASES_H

#include <string>
#include <vector>

#include "enums.h"

namespace gorilla::glfw
{

class window;
class monitor;

using error_fn_t = void(const error_code, const std::string& description);
using window_pos_fn_t = void(const window&, const int xpos, const int ypos);
using window_size_fn_t = void(const window&, const int width, const int height);
using window_close_fn_t = void(const window&);
using window_refresh_fn_t = void(const window&);

using window_focus_fn_t = void(const window&, const bool focused); // TODO
using window_iconify_fn_t = void(const window&, const bool iconified);
using window_maximize_fn_t = void(const window&, const bool maximized);

using framebuffer_size_fn_t = void(const window&, const int width, const int height);
using window_content_scale_fn_t = void(const window&, const float xscale, const float yscale);
using mousebutton_fn_t = void(const window&, const mouse_btn_id, const key_action, const modifier_flags);
using cursor_pos_fn_t = void(const window&, const double xpos, const double ypos);
using cursor_enter_fn_t = void(const window&, const bool entered);
using mouse_scroll_fn_t = void(const window&, const double xoffset, const double yoffset);
using key_fn_t = void(const window&, const key_id, const int scancode, const key_action, const modifier_flags);
using char_fn_t = void(const window&, const uint32_t codepoint);
using char_mods_fn_t = void(const window&, const uint32_t codepoint, const modifier_flags);
using drop_fn_t = void(const window&, const std::vector<std::string>& paths);

using monitor_fn_t = void(const monitor&, const int); // TODO
using joystick_fn_t = void(const int, const int);


using video_mode = GLFWvidmode;
using gamma_ramp = GLFWgammaramp;
using image = GLFWimage;
using gamepad_state = GLFWgamepadstate;

}

#endif //DEEPLOM_TYPE_ALIASES_H
