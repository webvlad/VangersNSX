#ifndef VANGERS_XPAD_CURSOR_H
#define VANGERS_XPAD_CURSOR_H

#include <SDL.h>

// Stick-driven pointer emulation for devices without a mouse (Nintendo Switch).
//
// The game's menus and HUD are mouse driven. While a game controller is
// present, the right stick moves a virtual pointer. In menus (and the pause
// screen) the physical A button (SDL "B", east) clicks at any time. While
// driving it only clicks shortly after the stick was moved, so it stays a
// plain game button; ZR clicks at any time. Minus (Back) sends Escape.

// Virtual "button" ids for the analogue triggers (SDL has no button for them). Used as
// SDL_GameControllerButton values inside SDLK_GAMECONTROLLER_BUTTON_MASK key codes.
enum { XPAD_VBUTTON_ZL = 64, XPAD_VBUTTON_ZR = 65 };

struct XPadCursorState {
	float x = -1.0f; // pointer position in game-screen pixels; <0 = not placed yet
	float y = -1.0f;
	Uint32 last_update_ms = 0;
	Uint32 last_move_ms = 0;
	bool active = false;
	bool click_down = false;
};

// Normalise a raw SDL axis value to [-1, 1] with a radial dead zone applied
// to the stick vector (ax, ay). Output keeps direction, rescaled so the
// response starts at 0 right after the dead zone and has a squared curve for
// fine control near the centre.
void XPadStickVector(Sint16 ax, Sint16 ay, float dead_zone, float &out_x, float &out_y);

// Advance the pointer state by one step and report what to do.
//   dx, dy      - stick vector from XPadStickVector
//   a_pressed   - current state of the click button
// Returns bit flags: 1 = pointer moved, 2 = send button-down, 4 = send button-up.
int XPadCursorStep(
	XPadCursorState &s,
	float dx,
	float dy,
	bool a_pressed,
	Uint32 now_ms,
	int screen_w,
	int screen_h,
	bool force_active = false
);

// Registers the per-frame hook (runs before the SDL event queue is drained).
void XPadCursorInstall();

#endif
