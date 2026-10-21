/* ---------------------------- INCLUDE SECTION ----------------------------- */

#include "../xgraph/xgraph.h"
#include "lang.h"
#include "xglobal.h"
#include "xt_list.h"

#ifdef __SWITCH__
#	include <stdio.h>
#	include <unistd.h>
extern "C" {
#	include <switch/services/clkrst.h>
}
extern unsigned g_prof_blit_us, g_prof_render_us, g_prof_present_us;
extern unsigned g_stage_us[8];
static ClkrstSession g_cpuClk;
static bool g_cpuClkOpen = false;
static u32 g_cpuClkOrig = 0;
static void switch_cpu_restore() {
	if (g_cpuClkOpen) {
		if (g_cpuClkOrig)
			clkrstSetClockRate(&g_cpuClk, g_cpuClkOrig);
		clkrstCloseSession(&g_cpuClk);
		g_cpuClkOpen = false;
	}
	clkrstExit();
}
#endif

#if defined(__unix__) || defined(__linux__) || defined(__APPLE__)
#	include <locale.h>
#endif

int __internal_argc;
char **__internal_argv;

/* ----------------------------- STRUCT SECTION ----------------------------- */

struct xtMsgHandlerObject {
	int ID;

	void (*Handler)(SDL_Event *);

	void *list;
	xtMsgHandlerObject *next;
	xtMsgHandlerObject *prev;

	xtMsgHandlerObject(void (*p)(SDL_Event *), int id);
};

/* ----------------------------- EXTERN SECTION ----------------------------- */
/* --------------------------- PROTOTYPE SECTION ---------------------------- */

void xtSetExit(void);
// int xtNeedExit(void);

int xtCallXKey(SDL_Event *m);
void xtSysQuant(void);

void xtAddSysObj(XList *lstPtr, void (*fPtr)(void), int id);
void xtDeleteSysObj(XList *lstPtr, int id);
void xtDeactivateSysObj(XList *lstPtr, int id);

void xtReadConsoleInput(void);

void xtRegisterSysMsgFnc(void (*fPtr)(SDL_Event *), int id);
void xtRegisterSysFinitFnc(void (*fPtr)(void), int id);
void xtUnRegisterSysFinitFnc(int id);
void xtDeactivateSysFinitFnc(int id);
void xtSysFinit(void);

// void xtPostMessage(HANDLE hWnd,int msg,int wp,int lp);
int xtDispatchMessage(SDL_Event *msg);
void xtProcessMessageBuffer(void);

/* --------------------------- DEFINITION SECTION --------------------------- */

// #define _RTO_LOG_

typedef void (*XFNC)();

#define XT_DEFAULT_TABLE_SIZE 32

XRuntimeObject **XRObjTable = NULL;
unsigned int XRObjTableSize = 0;

XRuntimeObject *XRObjFirst = NULL;
XRuntimeObject *XRObjLast = NULL;

const char *XToolClassName = "XToolClass";
const char *XToolWndName = "XToolWindow";

void *XAppHinst = NULL;
void *XGR_hWnd = NULL;

void *hXConOutput = NULL;
void *hXConInput = NULL;

int XAppMode = 0;

void (*press_handler)(SDL_Event *m);
void (*unpress_handler)(SDL_Event *m);

XList XSysQuantLst;
XList XSysFinitLst;
xtList<xtMsgHandlerObject> XSysHandlerLst;

double XTCORE_FRAME_DELTA = 0;
double XTCORE_FRAME_NORMAL = 0;

#ifdef _RTO_LOG_
XStream xtRTO_Log;
#endif

int xtSysQuantDisabled = 0;
extern bool XGR_FULL_SCREEN;

int SkipIntro = 0;

bool autoconnect = false;
char *autoconnectHost;
unsigned short autoconnectPort = 2197;
bool autoconnectJoinGame = false;
int autoconnectGameID;

XRuntimeObject *XObj = nullptr;

int getCurRtoId() {
	return XObj == nullptr ? 0 : XObj->ID;
}

#ifdef ANDROID
extern int vangers_main(int argc, char *argv[])
#else
int main(int argc, char *argv[])
#endif
{
	int id, prevID, clockDelta, clockCnt, clockNow, clockCntGlobal, clockNowGlobal;
	__internal_argc = argc;
	__internal_argv = argv;

#ifdef __SWITCH__
	// No console on the Switch: keep logs on the SD card and run from the data directory.
	freopen("sdmc:/switch/vangers/stdout.txt", "w", stdout);
	freopen("sdmc:/switch/vangers/stderr.txt", "w", stderr);
	setvbuf(stdout, NULL, _IOFBF, 1 << 16); // flushed every ~2 s from the main loop (SD writes stall frames)
	setvbuf(stderr, NULL, _IONBF, 0);
	chdir("sdmc:/switch/vangers/data");
	XGR_FULL_SCREEN = true;
	// No command line on the console: an empty file "russian" next to the nro switches the game to Russian.
	if (access("sdmc:/switch/vangers/russian", F_OK) == 0) {
		setLang(RUSSIAN);
		std::cout << "Language: Russian (flag file found)" << std::endl;
	}
	// The engine is CPU bound (software renderer): ask for the high CPU clock (1785 MHz).
	if (R_SUCCEEDED(clkrstInitialize())) {
		if (R_SUCCEEDED(clkrstOpenSession(&g_cpuClk, PcvModuleId_CpuBus, 3))) {
			g_cpuClkOpen = true;
			u32 hz = 0;
			clkrstGetClockRate(&g_cpuClk, &hz);
			g_cpuClkOrig = hz;
			Result rc = clkrstSetClockRate(&g_cpuClk, 1785000000);
			std::cout << "CPU clock " << hz << " -> 1785000000, result 0x" << std::hex << rc << std::dec << std::endl;
			atexit(switch_cpu_restore);
		}
	}
#endif

#ifdef _WIN32
	std::cout << "Load backtrace" << std::endl;
	LoadLibraryA("backtrace.dll");
	std::cout << "Set priority class" << std::endl;
	SetPriorityClass(GetCurrentProcess(), HIGH_PRIORITY_CLASS);
	putenv("SDL_AUDIODRIVER=DirectSound");
#endif

	for (int i = 1; i < argc; i++) {
		std::string cmd_key = argv[i];
		if (cmd_key == "-fullscreen") {
			XGR_FULL_SCREEN = true;
		} else if (cmd_key == "-skipintro") {
			SkipIntro = 1;
		} else if (cmd_key == "-russian") {
			setLang(RUSSIAN);
		} else if (cmd_key == "-server") {
			if (argc > i) {
				i++;
				autoconnect = true;
				autoconnectHost = argv[i];
			} else {
				std::cout << "Invalid parameter usage: '-server hostname' expected" << std::endl;
			}
		} else if (cmd_key == "-port") {
			if (argc > i) {
				i++;
				autoconnectPort = (unsigned short)strtol(argv[i], NULL, 0);
			} else {
				std::cout << "Invalid parameter usage: '-port value' expected" << std::endl;
			}
		} else if (cmd_key == "-game") {
			if (argc > i) {
				i++;
				std::string value = argv[i];
				autoconnectJoinGame = true;
				if (value == "new") {
					autoconnectGameID = 0;
				} else if (value == "any") {
					autoconnectGameID = -1;
				} else {
					autoconnectGameID = (int)strtol(argv[i], NULL, 0);
				}
			} else {
				std::cout << "Invalid parameter usage: '-game [id|new|any]' expected" << std::endl;
			}
		} else {
			std::cout << "Unknown parameter: '" << cmd_key << "'" << std::endl;
		}
	}

#if defined(__unix__) || defined(__linux__) || defined(__APPLE__)
	std::cout << "Set locale. ";
	char *res = setlocale(LC_NUMERIC, "POSIX");
	std::cout << "Result:" << res << std::endl;
#endif
	// Set handlers to null
	press_handler = NULL;
	unpress_handler = NULL;

	XMsgBuf = new XMessageBuffer;

	initclock();
	prevID = 0;
#ifdef _WIN32
	set_signal_handler();
#endif
	id = xtInitApplication();
	XObj = xtGetRuntimeObject(id);
#ifdef _RTO_LOG_
	if (XRec.flags & XRC_PLAY_MODE)
		xtRTO_Log.open("xt_rto_p.log", XS_OUT);
	else
		xtRTO_Log.open("xt_rto_w.log", XS_OUT);
#endif

#ifdef __SWITCH__
	Uint32 prof_quant_ms = 0;
#endif
	while (XObj) {
		XObj->Init(prevID);
		prevID = id;
		id = 0;

		clockCnt = clocki();
		clockCntGlobal = clockCnt;
		while (!id) {
			if (XObj->Timer) {
#ifdef __SWITCH__
				Uint32 prof_t0 = SDL_GetTicks();
#endif
				id = XObj->Quant();
#ifdef __SWITCH__
				prof_quant_ms = SDL_GetTicks() - prof_t0;
#endif
				clockNow = clockNowGlobal = clocki();
				clockDelta = clockNow - clockCnt;
				XTCORE_FRAME_DELTA = (clockNowGlobal - clockCntGlobal) / 1000.0;
				XTCORE_FRAME_NORMAL = XTCORE_FRAME_DELTA / 0.050; // 20FPS
				clockCntGlobal = clockNowGlobal;
				// std::cout<<"XTCORE_FRAME_DELTA:"<<XTCORE_FRAME_DELTA
				// 		 <<" XTCORE_FRAME_NORMAL:"<<XTCORE_FRAME_NORMAL
				// 		 <<" clockDelta:"<<clockDelta<<std::endl;

				if (clockDelta < XObj->Timer) {
					// std::cout<<"clockDelta:"<<clockDelta<<" Timer:"<<XObj->Timer<<std::endl;
					SDL_Delay(XObj->Timer - clockDelta);
				} else {
#ifndef __SWITCH__
					std::cout << "Strange deltas clockDelta:" << clockDelta
							  << " Timer:" << XObj->Timer << std::endl;
#endif
					if (clockDelta > 300) {
						// something wrong and for preventing abnormal physics set something neutral
						XTCORE_FRAME_NORMAL = 1.0;
					}
				}
				clockCnt = clocki();
			} else {
				id = XObj->Quant();
			}

#ifdef __SWITCH__
			Uint32 prof_x0 = SDL_GetTicks();
#endif
			if (!xtSysQuantDisabled)
				XRec.Quant(); // впускает внешние события, записывает их или воспроизводит
#ifdef __SWITCH__
			Uint32 prof_f0 = SDL_GetTicks();
			Uint32 prof_xrec_ms = prof_f0 - prof_x0;
#endif
			XGR_Flip();
#ifdef __SWITCH__
			{
				static Uint32 acc_q = 0, acc_f = 0, acc_n = 0, win_start = 0, last_end = 0;
				Uint32 now = SDL_GetTicks(), flip_ms = now - prof_f0;
				if (!win_start)
					win_start = now;
				Uint32 frame_ms = last_end ? now - last_end : 0;
				last_end = now;
				acc_q += prof_quant_ms;
				acc_f += flip_ms;
				acc_n++;
				if (frame_ms > 120 && frame_ms < 2000) {
					std::cout << "SLOW frame " << frame_ms << " ms: quant=" << prof_quant_ms << " events=" << prof_xrec_ms
							  << " [sound=" << g_stage_us[0] / 1000 << " actint=" << g_stage_us[1] / 1000
							  << " uvs=" << g_stage_us[2] / 1000 << " moveland=" << g_stage_us[3] / 1000
							  << " objects=" << g_stage_us[4] / 1000 << " terrain=" << g_stage_us[5] / 1000
							  << " models=" << g_stage_us[6] / 1000 << "] flip=" << flip_ms << " (blit=" << g_prof_blit_us / 1000
							  << " render=" << g_prof_render_us / 1000 << " present=" << g_prof_present_us / 1000
							  << ")\n";
				}
				static Uint32 last_flush = 0;
				if (now - last_flush >= 2000) {
					fflush(stdout);
					last_flush = now;
				}
				if (now - win_start >= 5000) {
					std::cout << "PERF " << acc_n * 1000 / (now - win_start) << " fps, avg quant=" << acc_q / acc_n
							  << " ms flip=" << acc_f / acc_n << " ms (blit=" << g_prof_blit_us / 1000
							  << " render=" << g_prof_render_us / 1000 << " present=" << g_prof_present_us / 1000
							  << " last)\n";
					fflush(stdout);
					acc_q = acc_f = acc_n = 0;
					win_start = now;
				}
			}
#endif
		}

		XObj->Finit();
#ifdef _RTO_LOG_
		xtRTO_Log < "\r\nChange RTO: " <= XObj->ID < " -> " <= id < " frame -> " <= XRec.frameCount;
#endif
		XObj = xtGetRuntimeObject(id);
	}
	xtDoneApplication();
	xtSysFinit();

#ifdef _RTO_LOG_
	xtRTO_Log.close();
#endif
	return 0;
}

void xtCreateRuntimeObjectTable(int len) {
	int i;
	if (!len)
		len = XT_DEFAULT_TABLE_SIZE;
	XRObjTableSize = len;
	XRObjTable = new XRuntimeObject *[len];

	for (i = 0; i < len; i++) {
		XRObjTable[i] = NULL;
	}
}

XRuntimeObject *xtGetRuntimeObject(unsigned int id) {
	if (id == XT_TERMINATE_ID)
		return NULL;
	if (!XRObjTable || !XRObjTableSize || id < 1 || id > XRObjTableSize)
		ErrH.Abort("XTool system error...");
	return XRObjTable[id - 1];
}

void xtRegisterRuntimeObject(XRuntimeObject *p) {
	if (!XRObjFirst) {
		XRObjFirst = XRObjLast = p;
	} else {
		XRObjLast->next = p;
		XRObjLast = p;
	}
	XRObjTable[p->ID - 1] = p;
}

int xtCallXKey(SDL_Event *m) {
	switch (m->type) {
	case SDL_KEYDOWN:
		if (press_handler) {
			(*press_handler)(m);
		}
		break;
	case SDL_KEYUP:
		if (unpress_handler) {
			(*unpress_handler)(m);
		}
		break;
	case SDL_TEXTINPUT:
		if (press_handler) {
			(*press_handler)(m);
		}
		break;
	case SDL_JOYBUTTONDOWN:
		// std::cout<<"jevent down button:"<<(int)m->jbutton.button<<std::endl;
		if (press_handler) {
			(*press_handler)(m);
		}
		break;
	case SDL_JOYBUTTONUP:
		// std::cout<<"jevent up"<<std::endl;
		if (unpress_handler) {
			(*unpress_handler)(m);
		}
		break;
	case SDL_CONTROLLERBUTTONDOWN:
		// std::cout<<"CONTROLLERBUTTONDOWN"<<std::endl;
		if (press_handler) {
			(*press_handler)(m);
		}
		break;
	case SDL_CONTROLLERBUTTONUP:
		// std::cout<<"CONTROLLERBUTTONUP"<<std::endl;
		if (unpress_handler) {
			(*unpress_handler)(m);
		}
		break;
	case SDL_JOYHATMOTION:
		// std::cout<<"SDL_JOYHATMOTION:"<<(int)m->jhat.hat<<" value"<<(int)m->jhat.value<<"
		// k:"<<std::endl;
		if (press_handler) {
			(*press_handler)(m);
		}
		break;
	case SDL_JOYBALLMOTION:
		// std::cout<<"SDL_JOYBALLMOTION:"<<(int)m->jball.ball<<" xrel:"<<m->jball.xrel<<"
		// yrel:"<<m->jball.yrel<<std::endl;
		break;
	case SDL_JOYAXISMOTION:
		// std::cout<<"SDL_JOYAXISMOTION:"<<(int)m->jaxis.axis<<" value"<<m->jaxis.value<<std::endl;
		break;
	case SDL_MOUSEWHEEL:
		if (press_handler) {
			(*press_handler)(m);
		}
		break;
	}
	return 1;
}

/*int xtCallXKey(SDL_Event* m)
{
	int rec_flag = 0,ret = 0;
	switch(m->type){
		case SDL_KEYDOWN:
			//std::cout<<"xtCallXKey sym:"<<m->key.keysym.sym<<"
scancode:"<<(int)m->key.keysym.scancode<<" SDLK_q:"<<SDLK_q<<std::endl;
			// TODO(amdmi3): this is supposed to be executed on WM_CHAR; non-char keys should
probably be filtered here XKey.LastChar = m->key.keysym.sym;

			XKey.PressFnc(m->key.keysym.sym, m->key.keysym.sym);
			rec_flag = 1;

			if (m->key.keysym.sym == SDLK_LALT || m->key.keysym.sym == SDLK_RALT ||
m->key.keysym.sym == SDLK_F10) ret = 1; break; case SDL_KEYUP: XKey.UnPressFnc(m->key.keysym.sym,
m->key.keysym.sym); rec_flag = 1;

			if (m->key.keysym.sym == SDLK_LALT || m->key.keysym.sym == SDLK_RALT ||
m->key.keysym.sym == SDLK_F10) ret = 1; break;
	}
	if(rec_flag && XRec.flags & XRC_RECORD_MODE){
		XRec.PutSysMessage(XRC_SYSTEM_MESSAGE, m);
	}

	return ret;
}*/

XList::XList(void) {
	ClearList();
}

XList::~XList(void) {}

void XList::AddElement(XListElement *p) {
	if (!fPtr) {
		fPtr = lPtr = p;
		p->prev = p;
		p->next = NULL;
	} else {
		lPtr->next = p;
		p->prev = lPtr;
		p->next = NULL;
		lPtr = p;
		fPtr->prev = p;
	}
	ListSize++;
}

void XList::RemoveElement(XListElement *p) {
	XListElement *pPtr, *nPtr;

	ListSize--;

	if (ListSize) {
		pPtr = p->prev;
		nPtr = p->next;

		pPtr->next = nPtr;
		if (nPtr)
			nPtr->prev = pPtr;

		if (p == fPtr)
			fPtr = nPtr;
		if (p == lPtr)
			lPtr = pPtr;

		lPtr->next = NULL;
		fPtr->prev = lPtr;
	} else
		ClearList();
}

void xtRegisterSysQuant(void (*qPtr)(void), int id) {
	xtAddSysObj(&XSysQuantLst, qPtr, id);
}

void xtUnRegisterSysQuant(int id) {
	xtDeleteSysObj(&XSysQuantLst, id);
}

void xtRegisterSysFinitFnc(void (*fPtr)(void), int id) {
	xtAddSysObj(&XSysFinitLst, fPtr, id);
}

void xtDeactivateSysFinitFnc(int id) {
	xtDeactivateSysObj(&XSysFinitLst, id);
}

void xtUnRegisterSysFinitFnc(int id) {
	xtDeleteSysObj(&XSysFinitLst, id);
}

void xtDeleteSysObj(XList *lstPtr, int id) {
	XSysObject *p = (XSysObject *)lstPtr->fPtr;
	while (p) {
		if (p->ID == id) {
			lstPtr->RemoveElement((XListElement *)p);
			delete p;
			return;
		}
		p = (XSysObject *)p->next;
	}
}

void xtDeactivateSysObj(XList *lstPtr, int id) {
	XSysObject *p = (XSysObject *)lstPtr->fPtr;

	while (p) {
		if (p->ID == id)
			p->flags |= XSYS_OBJ_INACTIVE;
		p = (XSysObject *)p->next;
	}
}

void xtAddSysObj(XList *lstPtr, void (*fPtr)(void), int id) {
	XSysObject *p = (XSysObject *)lstPtr->fPtr;

	while (p) {
		if (p->ID == id)
			return;
		p = (XSysObject *)p->next;
	}

	p = new XSysObject;
	p->ID = id;
	p->QuantPtr = (void *)fPtr;

	lstPtr->AddElement((XListElement *)p);
}

void xtSysQuant(void) {
	XSysObject *p = (XSysObject *)XSysQuantLst.fPtr;
	while (p) {
		(*(XFNC)(p->QuantPtr))();
		p = (XSysObject *)p->next;
	}
}

void xtSysFinit(void) {
	int i, sz = XSysFinitLst.ListSize;
	XSysObject *p = (XSysObject *)XSysFinitLst.lPtr;
	for (i = 0; i < sz; i++) {
		if (!(p->flags & XSYS_OBJ_INACTIVE))
			(*(XFNC)(p->QuantPtr))();
		p = (XSysObject *)p->prev;
	}
	XRec.Close();
}

/*int xtIsActive(void)
{
//	return (WAIT_OBJECT_0 == WaitForSingleObject(hXActiveWndEvent,0)) ? 1 : 0;
};*/

/*int xtNeedExit()
{
//	return (WAIT_OBJECT_0 == WaitForSingleObject(hXNeedExitEvent, 0)) ? 1 : 0;
};*/

void xtSetExit() {
	std::cout << "Exit!" << std::endl;
	SDL_Quit();
	exit(0);
	//	ResetEvent(hXActiveWndEvent);
	//	SetEvent(hXNeedExitEvent);
};

#ifdef __SWITCH__
// Taking the audio lock around pause/resume can deadlock with the audio thread after sleep.
#	define XT_AUDIO_LOCK() ((void)0)
#	define XT_AUDIO_UNLOCK() ((void)0)
#else
#	define XT_AUDIO_LOCK() SDL_LockAudioDevice(1)
#	define XT_AUDIO_UNLOCK() SDL_UnlockAudioDevice(1)
#endif
extern int Pause;
int xtDispatchMessage(SDL_Event *msg) {
	int ret = 0;

	xtMsgHandlerObject *p = XSysHandlerLst.first();
	while (p) {
		(*p->Handler)(msg);
		p = p->next;
	}

	ret += xtCallXKey(msg);
	switch (msg->type) {
	case SDL_QUIT:
		xtSetExit();
		break;
	case SDL_WINDOWEVENT:
		switch (msg->window.event) {
		case SDL_WINDOWEVENT_SHOWN:
			// Pause = 0;
			XT_AUDIO_LOCK();
			SDL_PauseAudioDevice(1, 0);
			XT_AUDIO_UNLOCK();
			std::cout << "window show" << std::endl;
			break;
		case SDL_WINDOWEVENT_HIDDEN:
			// Pause = 1;
			XT_AUDIO_LOCK();
			SDL_PauseAudioDevice(1, 1);
			XT_AUDIO_UNLOCK();
			std::cout << "window hidden" << std::endl;
			break;
		case SDL_WINDOWEVENT_RESTORED:
			// Pause = 0;
			XT_AUDIO_LOCK();
			SDL_PauseAudioDevice(1, 0);
			XT_AUDIO_UNLOCK();
			std::cout << "window restored" << std::endl;
			break;
		case SDL_WINDOWEVENT_FOCUS_LOST:
			// Pause = 1;
			XT_AUDIO_LOCK();
			SDL_PauseAudioDevice(1, 1);
			XT_AUDIO_UNLOCK();
			std::cout << "window focus lost" << std::endl;
			break;
		case SDL_WINDOWEVENT_FOCUS_GAINED:
			// Pause = 0;
			XT_AUDIO_LOCK();
			SDL_PauseAudioDevice(1, 0);
			XT_AUDIO_UNLOCK();
			std::cout << "window focus gained" << std::endl;
			break;
		}
		break;
	case SDL_USEREVENT:
		switch (msg->user.code) {
		case CursorAnimationEvent:
			doCursorAnimation();
		}
		break;
	}

	return ret;
}

static void (*xtFrameHook)(void) = nullptr;

void xtSetFrameHook(void (*hook)(void)) {
	xtFrameHook = hook;
}

void xtClearMessageQueue(void) {
	if (xtFrameHook)
		xtFrameHook();
	SDL_Event event;
	while (SDL_PollEvent(&event)) {
		// std::cout<<"event "<<event.type<<std::endl;
		if (XRec.CheckMessage(event.type)) {
			xtDispatchMessage(&event);
			//			if(!xtDispatchMessage(&event))
			//				DispatchMessage(&event);
		} else {
			XMsgBuf->put(&event);
		}
	}
}

void xtProcessMessageBuffer(void) {
	SDL_Event event;
	while (XMsgBuf->get(&event)) {
		if (!(XRec.flags & XRC_PLAY_MODE) || XRec.CheckMessage(event.type)) {
			xtDispatchMessage(&event);
			//			if(!xtDispatchMessage(&event))
			//				DispatchMessage(&event);
		}
	}
}

xtMsgHandlerObject::xtMsgHandlerObject(void (*p)(SDL_Event *), int id) {
	list = NULL;
	ID = id;

	Handler = p;
}

void xtRegisterSysMsgFnc(void (*fPtr)(SDL_Event *), int id) {
	xtMsgHandlerObject *p = new xtMsgHandlerObject(fPtr, id);
	XSysHandlerLst.append(p);
}

void win32_break(char *error, char *msg) {
	std::cout << "--------------------------------\n";
	std::cout << error << "\n";
	std::cout << msg << "\n";
	std::cout << "--------------------------------\n";
}

void *xtGet_hInstance(void) {
	return XAppHinst;
}

void *xtGet_hWnd(void) {
	return XGR_hWnd;
}

void xtSet_hWnd(void *hWnd) {
	XGR_hWnd = hWnd;
}

void xtSysQuantDisable(int v) {
	xtSysQuantDisabled = v;
}

void set_key_handlers(void (*pH)(SDL_Event *), void (*upH)(SDL_Event *)) {
	press_handler = pH;
	unpress_handler = upH;
}
