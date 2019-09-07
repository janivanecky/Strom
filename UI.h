#pragma once
#include "Math.h"
#include "Messenger.h"
#include <string>
#include<vector>

struct RenderEngine;
struct UIContext: public System
{
	Vector2 layoutPosition;
	Vector2 uiOffset;
	Vector4 backgroundColor;
	uint32_t barID;
	RenderEngine *engine;

	Vector2 mousePosition;
	Vector2 mousePositionDelta;
	bool mouseDown;
	bool mousePressed;
	bool scrollbarActive;
	float mouseScroll;

	uint64_t activeID;

	float scrollbarPosition;
};

namespace UI
{
	void NewWindow(UIContext *context, Vector2 offset = Vector2(0,0));
	UIContext GetUIContext(RenderEngine *engine);
	void NextBar(UIContext *context);
	bool Button(UIContext *context, char *text, bool pressed, bool checkBox = false);
	int32_t Menu(UIContext *context, std::string *labels, uint32_t labelCount, int32_t pressed);
	bool Button(UIContext *context, char *text, bool pressed, Vector2 position);
	Vector4 ColorPicker(UIContext *context, char *label, Vector4 color);
	uint32_t DropDown(UIContext *context, char *label, char **options, uint32_t optionCount, uint32_t selectedOption, uint32_t numberOfShownOptions);

	void HandleMessage(UIContext *context, Message message, void *data);

}
