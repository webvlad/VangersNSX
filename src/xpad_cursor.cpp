#include "xpad_cursor.h"

#include <algorithm>
#include <cmath>

#include "xjoystick.h"
#include "xtcore.h"

extern int xgrScreenSizeX;
extern int xgrScreenSizeY;
extern int g_xpad_in_race; // road.cpp: 1 while the race (game quant) is running
extern int Pause;          // road.cpp: pause screen is open

namespace {
constexpr float kDeadZone = 0.20f;
constexpr float kScreensPerSecond = 0.85f; // full tilt: 85% of screen width per second
constexpr Uint32 kActiveWindowMs = 3000;
constexpr Uint32 kMaxStepMs = 50;

enum { kMoved = 1, kDown = 2, kUp = 4 };

XPadCursorState g_state;
} // namespace

void XPadStickVector(Sint16 ax, Sint16 ay, float dead_zone, float &out_x, float &out_y) {
	float fx = std::max(-1.0f, ax / 32767.0f);
	float fy = std::max(-1.0f, ay / 32767.0f);
	float mag = std::sqrt(fx * fx + fy * fy);
	if (mag <= dead_zone) {
		out_x = out_y = 0.0f;
		return;
	}
	if (mag > 1.0f)
		mag = 1.0f;
	float scaled = (mag - dead_zone) / (1.0f - dead_zone); // 0..1 after the dead zone
	float gain = scaled * scaled;							// squared response curve
	float norm = std::sqrt(fx * fx + fy * fy);
	out_x = fx / norm * gain;
	out_y = fy / norm * gain;
}

int XPadCursorStep(
	XPadCursorState &s,
	float dx,
	float dy,
	bool a_pressed,
	Uint32 now_ms,
	int screen_w,
	int screen_h,
	bool force_active
) {
	int flags = 0;
	if (screen_w <= 0 || screen_h <= 0)
		return 0;

	Uint32 dt = now_ms - s.last_update_ms;
	if (s.last_update_ms == 0 || dt > kMaxStepMs)
		dt = kMaxStepMs; // first call or a long frame: avoid jumps
	s.last_update_ms = now_ms;

	if (s.x < 0.0f || s.y < 0.0f) {
		s.x = screen_w * 0.5f;
		s.y = screen_h * 0.5f;
	}

	if (dx != 0.0f || dy != 0.0f) {
		float step = screen_w * kScreensPerSecond * (dt / 1000.0f);
		s.x = std::min(std::max(s.x + dx * step, 0.0f), float(screen_w - 1));
		s.y = std::min(std::max(s.y + dy * step, 0.0f), float(screen_h - 1));
		s.last_move_ms = now_ms;
		s.active = true;
		flags |= kMoved;
	} else if (s.active && now_ms - s.last_move_ms > kActiveWindowMs && !s.click_down) {
		s.active = false;
	}

	if (a_pressed && !s.click_down && (s.active || force_active)) {
		s.click_down = true;
		flags |= kDown;
	} else if (!a_pressed && s.click_down) {
		s.click_down = false; // always release, even if the pointer went inactive meanwhile
		flags |= kUp;
	}
	return flags;
}

static void XPadCursorFrame() {
	SDL_GameController *ctrl = get_gamecontroller();
	if (!ctrl)
		return;

	float dx, dy;
	XPadStickVector(
		SDL_GameControllerGetAxis(ctrl, SDL_CONTROLLER_AXIS_RIGHTX),
		SDL_GameControllerGetAxis(ctrl, SDL_CONTROLLER_AXIS_RIGHTY),
		kDeadZone,
		dx,
		dy
	);
	// Physical A (east) is SDL's "B" on a Nintendo layout.
	bool zr = SDL_GameControllerGetAxis(ctrl, SDL_CONTROLLER_AXIS_TRIGGERRIGHT) > 16000;
	bool click = SDL_GameControllerGetButton(ctrl, SDL_CONTROLLER_BUTTON_B) != 0 || zr;
	// Outside the race (menus, loading screens) and on the pause screen the pointer is always live.
	bool force_active = zr || !g_xpad_in_race || Pause;

	int flags = XPadCursorStep(
		g_state, dx, dy, click, SDL_GetTicks(), xgrScreenSizeX, xgrScreenSizeY, force_active
	);

	// Triggers as buttons: send SDL_CONTROLLERBUTTON events too, so the controls screen can
	// capture them when rebinding.
	static bool zl_down = false, zr_down_ev = false;
	bool zl = SDL_GameControllerGetAxis(ctrl, SDL_CONTROLLER_AXIS_TRIGGERLEFT) > 16000;
	if (zl != zl_down || zr != zr_down_ev) {
		for (int t = 0; t < 2; t++) {
			bool now = t == 0 ? zl : zr;
			bool &was = t == 0 ? zl_down : zr_down_ev;
			if (now == was)
				continue;
			was = now;
			SDL_Event b;
			SDL_zero(b);
			b.type = now ? SDL_CONTROLLERBUTTONDOWN : SDL_CONTROLLERBUTTONUP;
			b.cbutton.which = 0;
			b.cbutton.button = t == 0 ? XPAD_VBUTTON_ZL : XPAD_VBUTTON_ZR;
			b.cbutton.state = now ? SDL_PRESSED : SDL_RELEASED;
			SDL_PushEvent(&b);
		}
	}

	// Minus (Back) = Escape: leave menus / open the pause menu.
	static bool esc_down = false;
	bool esc = SDL_GameControllerGetButton(ctrl, SDL_CONTROLLER_BUTTON_BACK) != 0;
	if (esc != esc_down) {
		esc_down = esc;
		SDL_Event k;
		SDL_zero(k);
		k.type = esc ? SDL_KEYDOWN : SDL_KEYUP;
		k.key.state = esc ? SDL_PRESSED : SDL_RELEASED;
		k.key.repeat = 0;
		k.key.keysym.scancode = SDL_SCANCODE_ESCAPE;
		k.key.keysym.sym = SDLK_ESCAPE;
		SDL_PushEvent(&k);
	}

	if (flags & kDown) {
		// Make sure the game knows where the pointer is before the click lands.
		flags |= kMoved;
	}

	if (flags & kMoved) {
		SDL_Event e;
		SDL_zero(e);
		e.type = SDL_MOUSEMOTION;
		e.motion.which = 0; // not SDL_TOUCH_MOUSEID, the game ignores touch-generated motion
		e.motion.x = int(g_state.x);
		e.motion.y = int(g_state.y);
		SDL_PushEvent(&e);
	}
	if (flags & (kDown | kUp)) {
		SDL_Event e;
		SDL_zero(e);
		e.type = (flags & kDown) ? SDL_MOUSEBUTTONDOWN : SDL_MOUSEBUTTONUP;
		e.button.which = 0;
		e.button.button = SDL_BUTTON_LEFT;
		e.button.state = (flags & kDown) ? SDL_PRESSED : SDL_RELEASED;
		e.button.clicks = 1;
		e.button.x = int(g_state.x);
		e.button.y = int(g_state.y);
		SDL_PushEvent(&e);
	}
}

void XPadCursorInstall() {
	xtSetFrameHook(XPadCursorFrame);
}
