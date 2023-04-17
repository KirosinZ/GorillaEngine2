//
// Created by Kiril on 23.02.2023.
//

#ifndef DEEPLOM_ENUMS_H
#define DEEPLOM_ENUMS_H

#include "library_object.h"

namespace gorilla::glfw
{

enum class key_action
{
	release = GLFW_RELEASE,
	press = GLFW_PRESS,
	repeat = GLFW_REPEAT,
};

enum class joystick_hat_state
{
	centered = GLFW_HAT_CENTERED,
	up = GLFW_HAT_UP,
	right = GLFW_HAT_RIGHT,
	down = GLFW_HAT_DOWN,
	left = GLFW_HAT_LEFT,
	right_up = GLFW_HAT_RIGHT_UP,
	right_down = GLFW_HAT_RIGHT_DOWN,
	left_up = GLFW_HAT_LEFT_UP,
	left_down = GLFW_HAT_LEFT_DOWN,
};

enum class key_id
{
	unknown = GLFW_KEY_UNKNOWN,

	space = GLFW_KEY_SPACE,
	apostrophe = GLFW_KEY_APOSTROPHE,
	comma = GLFW_KEY_COMMA,
	minus = GLFW_KEY_MINUS,
	period = GLFW_KEY_PERIOD,
	slash = GLFW_KEY_SLASH,
	digit_0 = GLFW_KEY_0,
	digit_1 = GLFW_KEY_1,
	digit_2 = GLFW_KEY_2,
	digit_3 = GLFW_KEY_3,
	digit_4 = GLFW_KEY_4,
	digit_5 = GLFW_KEY_5,
	digit_6 = GLFW_KEY_6,
	digit_7 = GLFW_KEY_7,
	digit_8 = GLFW_KEY_8,
	digit_9 = GLFW_KEY_9,
	semicolon = GLFW_KEY_SEMICOLON,
	equal = GLFW_KEY_EQUAL,
	A = GLFW_KEY_A,
	B = GLFW_KEY_B,
	C = GLFW_KEY_C,
	D = GLFW_KEY_D,
	E = GLFW_KEY_E,
	F = GLFW_KEY_F,
	G = GLFW_KEY_G,
	H = GLFW_KEY_H,
	I = GLFW_KEY_I,
	J = GLFW_KEY_J,
	K = GLFW_KEY_K,
	L = GLFW_KEY_L,
	M = GLFW_KEY_M,
	N = GLFW_KEY_N,
	O = GLFW_KEY_O,
	P = GLFW_KEY_P,
	Q = GLFW_KEY_Q,
	R = GLFW_KEY_R,
	S = GLFW_KEY_S,
	T = GLFW_KEY_T,
	U = GLFW_KEY_U,
	V = GLFW_KEY_V,
	W = GLFW_KEY_W,
	X = GLFW_KEY_X,
	Y = GLFW_KEY_Y,
	Z = GLFW_KEY_Z,
	left_bracket = GLFW_KEY_LEFT_BRACKET,
	backslash = GLFW_KEY_BACKSLASH,
	right_bracket = GLFW_KEY_RIGHT_BRACKET,
	grave_accent = GLFW_KEY_GRAVE_ACCENT,
	world_1 = GLFW_KEY_WORLD_1,
	world_2 = GLFW_KEY_WORLD_2,

	escape = GLFW_KEY_ESCAPE,
	enter = GLFW_KEY_ENTER,
	tab = GLFW_KEY_TAB,
	backspace = GLFW_KEY_BACKSPACE,
	insert = GLFW_KEY_INSERT,
	delete_ = GLFW_KEY_DELETE,
	right = GLFW_KEY_RIGHT,
	left = GLFW_KEY_LEFT,
	down = GLFW_KEY_DOWN,
	up = GLFW_KEY_UP,
	page_up = GLFW_KEY_PAGE_UP,
	page_down = GLFW_KEY_PAGE_DOWN,
	home = GLFW_KEY_HOME,
	end = GLFW_KEY_END,
	capslock = GLFW_KEY_CAPS_LOCK,
	scroll_lock = GLFW_KEY_SCROLL_LOCK,
	numlock = GLFW_KEY_NUM_LOCK,
	printscreen = GLFW_KEY_PRINT_SCREEN,
	pause = GLFW_KEY_PAUSE,
	f1 = GLFW_KEY_F1,
	f2 = GLFW_KEY_F2,
	f3 = GLFW_KEY_F3,
	f4 = GLFW_KEY_F4,
	f5 = GLFW_KEY_F5,
	f6 = GLFW_KEY_F6,
	f7 = GLFW_KEY_F7,
	f8 = GLFW_KEY_F8,
	f9 = GLFW_KEY_F9,
	f10 = GLFW_KEY_F10,
	f11 = GLFW_KEY_F11,
	f12 = GLFW_KEY_F12,
	f13 = GLFW_KEY_F13,
	f14 = GLFW_KEY_F14,
	f15 = GLFW_KEY_F15,
	f16 = GLFW_KEY_F16,
	f17 = GLFW_KEY_F17,
	f18 = GLFW_KEY_F18,
	f19 = GLFW_KEY_F19,
	f20 = GLFW_KEY_F20,
	f21 = GLFW_KEY_F21,
	f22 = GLFW_KEY_F22,
	f23 = GLFW_KEY_F23,
	f24 = GLFW_KEY_F24,
	f25 = GLFW_KEY_F25,
	kp0 = GLFW_KEY_KP_0,
	kp1 = GLFW_KEY_KP_1,
	kp2 = GLFW_KEY_KP_2,
	kp3 = GLFW_KEY_KP_3,
	kp4 = GLFW_KEY_KP_4,
	kp5 = GLFW_KEY_KP_5,
	kp6 = GLFW_KEY_KP_6,
	kp7 = GLFW_KEY_KP_7,
	kp8 = GLFW_KEY_KP_8,
	kp9 = GLFW_KEY_KP_9,
	kp_decimal = GLFW_KEY_KP_DECIMAL,
	kp_divide = GLFW_KEY_KP_DIVIDE,
	kp_multiply = GLFW_KEY_KP_MULTIPLY,
	kp_subtract = GLFW_KEY_KP_SUBTRACT,
	kp_add = GLFW_KEY_KP_ADD,
	kp_enter = GLFW_KEY_KP_ENTER,
	kp_equal = GLFW_KEY_KP_EQUAL,
	lshift = GLFW_KEY_LEFT_SHIFT,
	lctrl = GLFW_KEY_LEFT_CONTROL,
	lalt = GLFW_KEY_LEFT_ALT,
	lsup = GLFW_KEY_LEFT_SUPER,
	rshift = GLFW_KEY_RIGHT_SHIFT,
	rctrl = GLFW_KEY_RIGHT_CONTROL,
	ralt = GLFW_KEY_RIGHT_ALT,
	rsup = GLFW_KEY_RIGHT_SUPER,
	menu = GLFW_KEY_MENU,

	last = GLFW_KEY_LAST,
};

enum class modifier_flags
{
	shift = GLFW_MOD_SHIFT,
	ctrl = GLFW_MOD_CONTROL,
	alt = GLFW_MOD_ALT,
	sup = GLFW_MOD_SUPER,
	capslock = GLFW_MOD_CAPS_LOCK,
	numlock = GLFW_MOD_NUM_LOCK,
};

enum class mouse_btn_id
{
	btn_1 = GLFW_MOUSE_BUTTON_1,
	btn_2 = GLFW_MOUSE_BUTTON_2,
	btn_3 = GLFW_MOUSE_BUTTON_3,
	btn_4 = GLFW_MOUSE_BUTTON_4,
	btn_5 = GLFW_MOUSE_BUTTON_5,
	btn_6 = GLFW_MOUSE_BUTTON_6,
	btn_7 = GLFW_MOUSE_BUTTON_7,
	btn_8 = GLFW_MOUSE_BUTTON_8,
	btn_last = GLFW_MOUSE_BUTTON_LAST,
	lmb = GLFW_MOUSE_BUTTON_LEFT,
	rmb = GLFW_MOUSE_BUTTON_RIGHT,
	mmb = GLFW_MOUSE_BUTTON_MIDDLE,
};

enum class joystick_id
{
	_1 = GLFW_JOYSTICK_1,
	_2 = GLFW_JOYSTICK_2,
	_3 = GLFW_JOYSTICK_3,
	_4 = GLFW_JOYSTICK_4,
	_5 = GLFW_JOYSTICK_5,
	_6 = GLFW_JOYSTICK_6,
	_7 = GLFW_JOYSTICK_7,
	_8 = GLFW_JOYSTICK_8,
	_9 = GLFW_JOYSTICK_9,
	_10 = GLFW_JOYSTICK_10,
	_11 = GLFW_JOYSTICK_11,
	_12 = GLFW_JOYSTICK_12,
	_13 = GLFW_JOYSTICK_13,
	_14 = GLFW_JOYSTICK_14,
	_15 = GLFW_JOYSTICK_15,
	_16 = GLFW_JOYSTICK_16,
	last = GLFW_JOYSTICK_LAST,
};

enum class gamepad_btn_id
{
	A = GLFW_GAMEPAD_BUTTON_A,
	B = GLFW_GAMEPAD_BUTTON_B,
	X = GLFW_GAMEPAD_BUTTON_X,
	Y = GLFW_GAMEPAD_BUTTON_Y,
	LB = GLFW_GAMEPAD_BUTTON_LEFT_BUMPER,
	RB = GLFW_GAMEPAD_BUTTON_RIGHT_BUMPER,
	back = GLFW_GAMEPAD_BUTTON_BACK,
	start = GLFW_GAMEPAD_BUTTON_START,
	guide = GLFW_GAMEPAD_BUTTON_GUIDE,
	lthumb = GLFW_GAMEPAD_BUTTON_LEFT_THUMB,
	rthumb = GLFW_GAMEPAD_BUTTON_RIGHT_THUMB,
	dpad_up = GLFW_GAMEPAD_BUTTON_DPAD_UP,
	dpad_right = GLFW_GAMEPAD_BUTTON_DPAD_RIGHT,
	dpad_down = GLFW_GAMEPAD_BUTTON_DPAD_DOWN,
	dpad_left = GLFW_GAMEPAD_BUTTON_DPAD_LEFT,
	last = GLFW_GAMEPAD_BUTTON_LAST,
};

enum class gamepad_axis
{
	left_x = GLFW_GAMEPAD_AXIS_LEFT_X,
	left_y = GLFW_GAMEPAD_AXIS_LEFT_Y,
	right_x = GLFW_GAMEPAD_AXIS_RIGHT_X,
	right_y = GLFW_GAMEPAD_AXIS_RIGHT_Y,
	left_trigger = GLFW_GAMEPAD_AXIS_LEFT_TRIGGER,
	right_trigger = GLFW_GAMEPAD_AXIS_RIGHT_TRIGGER,
	last = GLFW_GAMEPAD_AXIS_LAST,
};

enum class error_code
{
	no_error = GLFW_NO_ERROR,

	not_inintialized = GLFW_NOT_INITIALIZED,
	no_current_context = GLFW_NO_CURRENT_CONTEXT,
	invalid_enum = GLFW_INVALID_ENUM,
	invalid_value = GLFW_INVALID_VALUE,
	out_of_memory = GLFW_OUT_OF_MEMORY,
	api_unavailable = GLFW_API_UNAVAILABLE,
	version_unavailable = GLFW_VERSION_UNAVAILABLE,
	platform_error = GLFW_PLATFORM_ERROR,
	format_unavailable = GLFW_FORMAT_UNAVAILABLE,
	no_window_context = GLFW_NO_WINDOW_CONTEXT,
};

//enum class window_attribute
//{
//	focused = GLFW_FOCUSED,
//	iconified = GLFW_ICONIFIED,
//	resizable = GLFW_RESIZABLE,
//	visible = GLFW_VISIBLE,
//	decorated = GLFW_DECORATED,
//	auto_iconify = GLFW_AUTO_ICONIFY,
//	floating = GLFW_FLOATING,
//	maximized = GLFW_MAXIMIZED,
//	center_cursor = GLFW_CENTER_CURSOR,
//	transparent_framebuffer = GLFW_TRANSPARENT_FRAMEBUFFER,
//	hovered = GLFW_HOVERED,
//	focus_on_show = GLFW_FOCUS_ON_SHOW,
//
//	red_bits = GLFW_RED_BITS,
//	green_bits =  GLFW_GREEN_BITS,
//	blue_bits = GLFW_BLUE_BITS,
//	alpha_bits = GLFW_ALPHA_BITS,
//	depth_bits = GLFW_DEPTH_BITS,
//	stencil_bits = GLFW_STENCIL_BITS,
//	accum_red_bits = GLFW_ACCUM_RED_BITS,
//	accum_green_bits = GLFW_ACCUM_GREEN_BITS,
//	accum_blue_bits = GLFW_ACCUM_BLUE_BITS,
//	accum_alpha_bits = GLFW_ACCUM_ALPHA_BITS,
//	aux_buffers = GLFW_AUX_BUFFERS,
//	stereo = GLFW_STEREO,
//	samples = GLFW_SAMPLES,
//	srgb_capable = GLFW_SRGB_CAPABLE,
//	refresh_rate = GLFW_REFRESH_RATE,
//	doublebuffer = GLFW_DOUBLEBUFFER,
//
//	client_api = GLFW_CLIENT_API,
//	context_version_major = GLFW_CONTEXT_VERSION_MAJOR,
//	context_version_minor = GLFW_CONTEXT_VERSION_MINOR,
//	context_revision = GLFW_CONTEXT_REVISION,
//	context_robustness = GLFW_CONTEXT_ROBUSTNESS,
//	opengl_fwd_compat = GLFW_OPENGL_FORWARD_COMPAT,
//	opengl_debug_context = GLFW_OPENGL_DEBUG_CONTEXT,
//	opengl_profile = GLFW_OPENGL_PROFILE,
//	context_release_behaviour = GLFW_CONTEXT_RELEASE_BEHAVIOR,
//	context_no_error = GLFW_CONTEXT_NO_ERROR,
//	context_creation_api = GLFW_CONTEXT_CREATION_API,
//	scale_to_monitor = GLFW_SCALE_TO_MONITOR,
//	cocoa_retina_framebuffer = GLFW_COCOA_RETINA_FRAMEBUFFER,
//	cocoa_frame_name = GLFW_COCOA_FRAME_NAME,
//	cocoa_graphics_switching = GLFW_COCOA_GRAPHICS_SWITCHING,
//	x11_class_name = GLFW_X11_CLASS_NAME,
//	x11_instance_name = GLFW_X11_INSTANCE_NAME,
//
//
//};

enum class cursor_shape
{
	arrow = GLFW_ARROW_CURSOR,
	ibeam = GLFW_IBEAM_CURSOR,
	crosshair = GLFW_CROSSHAIR_CURSOR,
	hand = GLFW_HAND_CURSOR,
	hresize = GLFW_HRESIZE_CURSOR,
	vresize = GLFW_VRESIZE_CURSOR
};

template<typename Enum>
inline std::underlying_type_t<Enum> enum_cast(const Enum value)
{
	return std::underlying_type_t<Enum>(value);
}

template<typename Enum>
inline Enum enum_cast(const std::underlying_type_t<Enum> value)
{
	return Enum(value);
}

}

#endif //DEEPLOM_ENUMS_H
