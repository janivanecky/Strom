#pragma once
#include "Messenger.h"
#include "Draw.h"
#include "Engine.h"
#include "UI.h"

struct DrawContext;
struct Input
{
	float mousePositionX;
	float mousePositionY;
	float rawPositionX;
	float rawPositionY;
	float mouseDX;
	float mouseDY;
	bool leftMouseButtonDown;
	bool leftMouseButtonPressed;
	float mouseScrollDelta;
	bool leftButtonDown;
	bool leftButtonPressed;
	bool rightButtonDown;
	bool rightButtonPressed;

	bool keyDown[200];
	bool keyPressed[200];
};

struct AppContext: public System
{
	RenderEngine renderEngine;
	UIContext uiContext;
	Messenger messenger;
	DrawContext *dContext;

	char **displayModeTitles;
	uint32_t displayModeCount;


	Mesh landMesh;
	Mesh treeMesh;
	Mesh leavesMesh;
};

void Resize(AppContext *aContext, uint32_t screenX, uint32_t screenY);
void Init(AppContext *aContext, uint32_t screenX, uint32_t screenY);
int Update(AppContext *aContext, Input *input, float dt);
void Release(AppContext *aContext);