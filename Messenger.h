#pragma once
#include <stdint.h>

struct Messenger;
struct System
{
	Messenger *messenger;
};

struct MOUSE_CHANGE_DATA
{
	float dx;
	float dy;
	float positionX;
	float positionY;
	float scrollDelta;
	bool leftButtonDown;
	bool leftButtonPressed;
};

struct DrawContext;
struct RESIZE_DATA
{
	DrawContext *dContext;
	uint32_t screenWidth;
	uint32_t screenHeight;
};

enum Message
{
	MOUSE_CHANGE,
	UI_INPUT_LOCK,
	UI_INPUT_UNLOCK,
	RESIZE_WINDOW,
	RELOAD_TREE,
};

#define MESSAGE_HANDLER(name) void name(System *system, Message message, void *data)
typedef MESSAGE_HANDLER(MessageHandler);

struct Messenger
{
	MessageHandler *handlers[10] = {};
	System *systems[10] = {};
	uint32_t systemCounter = 0;
};


namespace Messaging
{
	void RegisterSystem(Messenger *messenger, System *system, MessageHandler *handler);
	void Dispatch(Messenger *messenger, Message message, void *data = NULL);
}