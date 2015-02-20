#include <SDL.h>
#include "video.h"

#define KEYBUFFERSIZE	256		// 键盘缓冲区大小

static int keybuffer[256];
static int keybuf_start;
static int keybuf_end;
static int keymap[256];


void KeyBuf_Init(void)
{
	keybuf_start = 0;
	keybuf_end = 0;
}


void KeyBuf_PutKey(int key)
{
	int t = keybuf_end;
	if (++t > KEYBUFFERSIZE)
		t = 0;
	if (t == keybuf_start)		// keybuffer is full
		return;
	keybuffer[t] = key;
	keybuf_end = t;
}


bool KeyBuf_IsEmpty(void)
{
	return keybuf_start == keybuf_end;
}


// returns -1 if there is no key to be read.
int KeyBuf_GetKey(void)
{
	int key;
	if (KeyBuf_IsEmpty())
		return -1;
	key = keybuffer[keybuf_start++];
	if (keybuf_start > KEYBUFFERSIZE)
		keybuf_start = 0;
}


static int KeyFilter(void *userdata, SDL_Event * event)
{
	static int Esc_KeyPress = 0;
	static int Space_KeyPress = 0;
	static int Return_KeyPress = 0;

	int r = 1;
	switch (event->type) {
	case SDL_KEYDOWN:
		switch (event->key.keysym.sym) {
		case SDLK_ESCAPE:
			if (1 == Esc_KeyPress) {
				r = 0;
			} else {
				Esc_KeyPress = 1;
			}
			break;
		case SDLK_RETURN:
			if (1 == Return_KeyPress) {
				r = 0;
			} else {
				Return_KeyPress = 1;
			}
			break;
		case SDLK_SPACE:
			if (1 == Space_KeyPress) {
				r = 0;
			} else {
				Space_KeyPress = 1;
			}
			break;
		default:
			break;
		}
		break;
	case SDL_KEYUP:
		switch (event->key.keysym.sym) {
		case SDLK_ESCAPE:
			Esc_KeyPress = 0;
			break;
		case SDLK_SPACE:
			Space_KeyPress = 0;
			break;
		case SDLK_RETURN:
			Return_KeyPress = 0;
			break;
		default:
			break;
		}
		break;
	default:
		break;
	}

	return r;
}


//过滤ESC、RETURN、SPACE键，使他们按下后不能重复。
//static int KeyFilter(const SDL_Event *event)

Uint32 dirOrgX, dirOrgY;

typedef struct tagTouchButton {
    SDL_Rect        rect;
    bool        down;
    SDL_FingerID    fingerId;
} TouchButton;

TouchButton escTb, digTb, dirTb;

SDL_Point upPt, leftPt, downPt, rightPt, centerPt;

bool PtInRect(SDL_Point *pt, SDL_Rect *rect)
{
    return pt->x > rect->x && 
        pt->x < rect->x + rect->w && 
        pt->y > rect->y && 
        pt->y < rect->y + rect->h;
}


int GetDistance(SDL_Point *pt1, SDL_Point *pt2)
{
    int dx = pt1->x - pt2->x;
    int dy = pt1->y - pt2->y;
    return dx * dx + dy * dy;
}


int GetMinItemIndex(int v[], size_t count)
{
    int value = 0;
    int i = 0;
    int ret = 0;

    for (i=0, value=v[i], ret=i; i < count; i++) {
        if (v[i] < value) {
            value = v[i];
            ret = i;
        }
    }
    return ret;
}

int GetDirKey(SDL_Point *pt)
{
    int v[4];

    v[0] = GetDistance(&upPt, pt);
    v[1] = GetDistance(&leftPt, pt);
    v[2] = GetDistance(&downPt, pt);
    v[3] = GetDistance(&rightPt, pt);

    switch (GetMinItemIndex(v, 4)) {
        case 0:
            return SDLK_UP;
            break;
        case 1:
            return SDLK_LEFT;
            break;
        case 2:
            return SDLK_DOWN;
            break;
        case 3:
            return SDLK_RIGHT;
            break;
    }
}

int Input_Init(void)
{

    escTb.rect.x = Video_GetWindowWidth() / 2;
    escTb.rect.y = 0;
    escTb.rect.w = Video_GetWindowWidth() / 2;
    escTb.rect.h = Video_GetWindowHeight() / 2;
    escTb.down = false;
    escTb.fingerId = -1;

    digTb.rect.x = escTb.rect.x;
    digTb.rect.y = escTb.rect.h;
    digTb.rect.w = escTb.rect.w;
    digTb.rect.h = escTb.rect.h;
    digTb.down = false;
    digTb.fingerId = -1;

    dirTb.rect.x = 0;
    dirTb.rect.y = 0;
    dirTb.rect.w = Video_GetWindowWidth() / 2;
    dirTb.rect.h = Video_GetWindowHeight();
    dirTb.down = false;
    dirTb.fingerId = -1;

    centerPt.x = dirTb.rect.w / 2;
    centerPt.y = dirTb.rect.h / 2;

    upPt.x = centerPt.x;
    upPt.y = 0;

    leftPt.x = 0;
    leftPt.y = centerPt.y;

    downPt.x = centerPt.x;
    downPt.y = dirTb.rect.h - 1;

    rightPt.x = dirTb.rect.w - 1;
    rightPt.y = centerPt.y;
	SDL_SetEventFilter(KeyFilter, NULL);

	return 0;
}


void Input_Quit(void)
{
}


/*
 * @return:
 *  -1: no keyboard input
 *  -3: Window's close button is pressed
 */

//int JY_GetKey(void)
const char *JY_GetCommand(void)
{
    static bool virgin = true;
    static SDL_Keycode lastDir = -1;
    if (virgin) {
        Input_Init();
        virgin = false;
    }
	SDL_Event event;
	SDL_Keycode keycode = -1;
    SDL_Point touchPt;

	while (SDL_PollEvent(&event)) {
		switch (event.type) {
		case SDL_KEYDOWN:
			keycode = event.key.keysym.sym;
			if (keycode == SDLK_F4) {
				return "quit";
            }
            else {
                switch (keycode) {
                case SDLK_ESCAPE:
                    return "menu";
                    break;
                case SDLK_RETURN:
                case SDLK_SPACE:
                    return "action";
                    break;
                case SDLK_UP:
                    return "up";
                    break;
                case SDLK_LEFT:
                    return "left";
                    break;
                case SDLK_RIGHT:
                    return "right";
                    break;
                case SDLK_DOWN:
                    return "down";
                    break;
                }
            }
			break;

			// When Close Window Button was pressed
		case SDL_QUIT:
			return "quit";
			break;
        case SDL_FINGERDOWN:
            touchPt.x = event.tfinger.x * Video_GetWindowWidth();
            touchPt.y = event.tfinger.y * Video_GetWindowHeight();
            if (PtInRect(&touchPt, &escTb.rect) && !escTb.down) {
                //SDL_Log("ESC Button Pressed");
                escTb.down = true;
                escTb.fingerId = event.tfinger.fingerId;
                keycode = SDLK_ESCAPE;
            } else if (PtInRect(&touchPt, &dirTb.rect) && !dirTb.down) {
                //SDL_Log("Dir Button Pressed");
                dirTb.down = true;
                dirTb.fingerId = event.tfinger.fingerId;
                keycode = GetDirKey(&touchPt);
                lastDir = keycode;
            } else if (PtInRect(&touchPt, &digTb.rect) && !digTb.down) {
                //SDL_Log("Dig Button Pressed");
                digTb.down = true;
                digTb.fingerId = event.tfinger.fingerId;
                keycode = SDLK_SPACE;
            }
            break;

        case SDL_FINGERUP:
            if (event.tfinger.fingerId == escTb.fingerId) {
                //SDL_Log("ESC Button Up");
                escTb.down = false;
                escTb.fingerId = -1;
            } else if (event.tfinger.fingerId == dirTb.fingerId) {
                //SDL_Log("Dir Button Up");
                dirTb.down = false;
                dirTb.fingerId = -1;
            } else if (event.tfinger.fingerId == digTb.fingerId) {
                //SDL_Log("Dig Button Up");
                digTb.down = false;
                digTb.fingerId = -1;
            }
            break;

		default:
			break;
		}
	}

	if (dirTb.down) {
        switch (lastDir) {
        case SDLK_ESCAPE:
            return "menu";
            break;
        case SDLK_RETURN:
        case SDLK_SPACE:
            return "action";
            break;
        case SDLK_UP:
            return "up";
            break;
        case SDLK_LEFT:
            return "left";
            break;
        case SDLK_RIGHT:
            return "right";
            break;
        case SDLK_DOWN:
            return "down";
            break;
        }
    }

	return "null";
}


