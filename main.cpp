#include "Draw.h"
#include "main.h"
#include "Math.h"
#include "Engine.h"
#include "Strom/Strom.h"
#include "IO.h"
#include <string>
#include <mutex>
#include <thread>

#include <cmath>
#include <cstdlib>
#include <string>

/*

Strom
V1:
	- No resources, just L-system branching
	- branching angle, deflection angle, phyllotactic angle
	- Simple model, no fancy connectors
V2:
	- Voxel space
	- BH
	- shedding
V3: 
	- priority model
	- gravitropism
V4:
	- gravimorphism
	- fancy 3d model

VFIN
	- All the features of the growing alg.
	- No textures
	- Mesh simple





TODO:
	- Bark texture
	- Leaf billboard

*/

std::mutex meshMutex;
std::mutex drawMutex;

Vector3 *voxelMatrices;
float *voxelScales;
uint32_t voxelCount;

TreeParameters GetRandomParams()
{
	TreeParameters treeParams;
#define RANDOM
#ifdef RANDOM
	treeParams.apicalControlMaxOrder = rand() % 2 + 1;
	treeParams.branchingAngle = Math::Random(Math::Deg2Rad(20), Math::Deg2Rad(40));
	treeParams.defaultOrientationWeight = Math::Random(0.1f, 0.3f);
	treeParams.deflectionAngle = Math::Random(0, 1.0f);
	treeParams.environmentOrientationWeight = 1 - treeParams.defaultOrientationWeight;
	treeParams.lambda = Math::Random(0.44f, 0.56);
	treeParams.phyllotacticAngle = Math::Deg2Rad(135);
	treeParams.resourceThreshold = 0.25f;
	treeParams.shedThreshold = .25f;
	treeParams.tropismAngle = Math::Random(0, PI);
	treeParams.tropismMinOrder = rand() % 4 + 1;
	treeParams.tropismOrientationWeight = Math::Random(0, 0.5f);
	treeParams.widthPower = Math::Random(2.5f, 3.0f);
	treeParams.resourceToLightRatio = Math::Random(0.5f, 0.6f);
	treeParams.verticalBudPreference = 1.0f;
	treeParams.horizontalBudPreference = 1.0f;
	treeParams.upwardBudPreference = 0.0f;
#else
	treeParams.apicalControlMaxOrder = 0;// rand() % 5 + 1;
	treeParams.branchingAngle = Math::Deg2Rad(30);// Math::Random(Math::Deg2Rad(20), Math::Deg2Rad(40));
	treeParams.defaultOrientationWeight = 0.3f;// Math::Random(0.2f, 0.5f);
	treeParams.deflectionAngle = 0;// Math::Random();
	treeParams.environmentOrientationWeight = 1 - treeParams.defaultOrientationWeight;
	treeParams.lambda = 0.46f;// Math::Random(0.44f, 0.64f);
	treeParams.phyllotacticAngle = Math::Deg2Rad(135);
	treeParams.resourceThreshold = 0.25f;
	treeParams.shedThreshold = 0.25f;
	treeParams.tropismAngle = PI;// Math::Random(0, PI);
	treeParams.tropismMinOrder = 3;// rand() % 4 + 1;
	treeParams.tropismOrientationWeight = 0.4f;// Math::Random(0, 0.5f);
	treeParams.widthPower = 3.0f;// Math::Random(2.0f, 3.0f);
	treeParams.resourceToLightRatio = 0.5f;
	treeParams.verticalBudPreference = 0.1f;
#endif
	return treeParams;
}

uint32_t currentId = 0;

float progress = 1.0f;
std::mutex progressMutex;
bool fadeIn = false;
bool fadeOut = false;
float fadeProgress = 0.0f;

std::mutex counterMutex;
std::condition_variable counterVariable;
uint32_t threadCounter = 0;
bool end = false;

LARGE_INTEGER freq;
LARGE_INTEGER start;

uint32_t IT_NUMBER = 20;
static uint32_t vertexCount = 0;
void GetTreeMesh(DrawContext *dContext, TreeShell shell, Mesh *targetTreeMesh, Mesh *targetLeavesMesh, uint32_t id, uint32_t seed)
{
	srand(seed);
	counterMutex.lock();
	++threadCounter;
	counterMutex.unlock();
	TreeMesh leavesMesh;
	TreeMesh treeMesh = Strom::GetTreeMesh(&shell, &leavesMesh);
	Mesh treeMesh3D = Draw::GetMesh(dContext, treeMesh.vertices, treeMesh.vertexStride,
									treeMesh.vertexCount, treeMesh.indices, sizeof(uint32),
									treeMesh.indexCount);

	Mesh leavesMesh3D = Draw::GetMesh(dContext, leavesMesh.vertices, leavesMesh.vertexStride,
									  leavesMesh.vertexCount, leavesMesh.indices, sizeof(uint32),
									  leavesMesh.indexCount);
	meshMutex.lock();
	if (currentId < id)
	{
		Mesh oldTreeMesh = *targetTreeMesh;
		Mesh oldLeavesMesh = *targetLeavesMesh;
		*targetTreeMesh = treeMesh3D;
		*targetLeavesMesh = leavesMesh3D;
		currentId = id;
		Draw::Release(dContext, &oldTreeMesh);
		Draw::Release(dContext, &oldLeavesMesh);

		progressMutex.lock();
		progress = id / (float)(IT_NUMBER - 1);
		vertexCount = treeMesh.vertexCount;
		if (progress == 1.0f)
		{
			fadeIn = false;
			fadeOut = true;
			LARGE_INTEGER end;
			QueryPerformanceCounter(&end);
			float time = (float)(end.QuadPart - start.QuadPart) / freq.QuadPart;
			char buffer[100];
			sprintf_s(buffer, 100, "%f", time);
			OutputDebugStringA("\n*********************\n");
			OutputDebugStringA(buffer);
			OutputDebugStringA("\n*********************\n");

		}
		progressMutex.unlock();
	}
	else
	{
		Draw::Release(dContext, &treeMesh3D);
		Draw::Release(dContext, &leavesMesh3D);
	}
	meshMutex.unlock();
	Strom::Release(&treeMesh);
	Strom::Release(&shell);
	counterMutex.lock();
	--threadCounter;
	counterVariable.notify_all();
	counterMutex.unlock();
}

#include "Strom\VoxelSpace.h"
void ReloadTree(DrawContext *dContext, Mesh *targetTreeMesh, Mesh *targetLeavesMesh, uint32_t seed, bool growth)
{
	QueryPerformanceFrequency(&freq);
	QueryPerformanceCounter(&start);

	counterMutex.lock();
	++threadCounter;
	counterMutex.unlock();
	srand(seed);
	currentId = 1;
	WorldConstants worldConsts;
	worldConsts.basicWidth = 0.2f;// 0.2f;
	worldConsts.basicLength = 2.5f;
	worldConsts.voxelSize = 2.0f;
	worldConsts.shadowConstantA = 0.1f;
	worldConsts.shadowConstantB = 1.75f;
	worldConsts.fullLightExposure = 1.0f;
	worldConsts.voxelsPerEdge = 149;
	worldConsts.canopyHeight = rand() % 4 == 0 ? (rand() % 1000) / 1000.0f * 40.0f + 10.0f : 0.0f;// 400.0f;
	Tree tree = Strom::Plant(&GetRandomParams(), &worldConsts);
	for (uint32_t i = 2; i < IT_NUMBER; ++i)
	{
		counterMutex.lock();
		if (end)
		{
			counterMutex.unlock();
			break;
		}
		counterMutex.unlock();
		Strom::Grow(&tree, i);
		if (growth)
		{
			TreeShell shell = Strom::GetShell(&tree);
			auto thread = std::thread(GetTreeMesh, dContext, shell, targetTreeMesh, targetLeavesMesh, i, seed);
			thread.detach();
		}
		else
		{
			if (i == IT_NUMBER - 1)
			{
				if (false)
				{
					voxelCount = 0;
					for (uint32_t z = 0; z < tree.voxelSpace.voxelsPerEdge; ++z)
					{
						for (uint32_t y = 0; y < tree.voxelSpace.voxelsPerEdge; ++y)
						{
							for (uint32_t x = 0; x < tree.voxelSpace.voxelsPerEdge; ++x)
							{
								Vector3i coordinate = Vector3i(x, y, z);
								float sValue = Voxels::ShadowValueFromCoordinate(&tree.voxelSpace, coordinate);
								if (sValue > 0.01f)
								{
									Vector3 position = Voxels::VoxelCenterPositionFromCoordinate(&tree.voxelSpace, coordinate);
									voxelScales[voxelCount] = Math::Clamp(sValue / 2.0f, 0, 1);
									voxelMatrices[voxelCount++] = position * 0.5f;
								}
							}

						}
					}
				}

				TreeShell shell = Strom::GetShell(&tree);
				TreeMesh leavesMesh;
				TreeMesh treeMesh = Strom::GetTreeMesh(&shell, &leavesMesh);
				Mesh treeMesh3D = Draw::GetMesh(dContext, treeMesh.vertices, treeMesh.vertexStride,
												treeMesh.vertexCount, treeMesh.indices, sizeof(uint32),
												treeMesh.indexCount);

				Mesh leavesMesh3D = Draw::GetMesh(dContext, leavesMesh.vertices, leavesMesh.vertexStride,
												  leavesMesh.vertexCount, leavesMesh.indices, sizeof(uint32),
												  leavesMesh.indexCount);
				meshMutex.lock();
				*targetTreeMesh = treeMesh3D;
				*targetLeavesMesh = leavesMesh3D;
				meshMutex.unlock();
				Strom::Release(&leavesMesh);
				Strom::Release(&treeMesh);
				Strom::Release(&shell);
			}
			progressMutex.lock();
			progress = i / (float)(IT_NUMBER - 1);
			if (progress == 1.0f)
			{
				fadeOut = true;
				fadeIn = false;
			}
			progressMutex.unlock();
		}
	}
	Strom::Release(&tree);
	counterMutex.lock();
	--threadCounter;
	counterMutex.unlock();
}

bool growth = true;
float timer = 4.0f;
void HandleMessage(AppContext *aContext, Message message, void *data)
{
	switch (message)
	{
	case RELOAD_TREE:
	{
		progressMutex.lock();
		float currentProgress = progress;
		progressMutex.unlock();
		if (currentProgress == 1.0f)
		{
			// Reset all the global progress variables. This is done on the main thread so there's no chance
			// that two threads are launched (Which could well be the case if the progress was being reset in 
			// thread).
			progressMutex.lock();
			progress = 0.0f;
			fadeIn = true;
			fadeOut = false;
			progressMutex.unlock();
			std::thread treeThread(ReloadTree, aContext->dContext, &aContext->treeMesh, &aContext->leavesMesh, (uint32_t)(timer * 123), growth);
			treeThread.detach();
		}
	} break;
	}
}

struct Settings
{
	Vector2 resolution;
	bool fullScreen;
};

Vector3 treePosition[15];
Mesh cubeMesh;
#define CUBE_MESH
#include "Models.h"
void Init(AppContext *aContext, uint32_t screenX, uint32_t screenY)
{
	voxelMatrices = new Vector3[150 * 150 * 150];
	voxelScales = new float[150 * 150 * 150];
	voxelCount = 0;

	LARGE_INTEGER perfCounter;
	QueryPerformanceCounter(&perfCounter);
	srand(perfCounter.LowPart);
	srand(0);

	DrawContext *dContext = aContext->dContext;

	aContext->renderEngine = REngine::GetEngine(dContext, screenX, screenY);
	aContext->uiContext = UI::GetUIContext(&aContext->renderEngine);

	Messaging::RegisterSystem(&aContext->messenger, &aContext->renderEngine,
							  (MessageHandler *)REngine::HandleMessage);
	Messaging::RegisterSystem(&aContext->messenger, &aContext->uiContext,
							  (MessageHandler *)UI::HandleMessage);
	Messaging::RegisterSystem(&aContext->messenger, aContext, (MessageHandler *)HandleMessage);

	aContext->leavesMesh.indexCount = 0;
	aContext->treeMesh.indexCount = 0;

	aContext->displayModeTitles = (char **)malloc(sizeof(char *) * aContext->dContext->displayModeCount);
	for (uint32_t i = 0; i < aContext->dContext->displayModeCount; ++i)
	{
		aContext->displayModeTitles[i] = (char *)malloc(sizeof(char) * 80);
		sprintf_s(aContext->displayModeTitles[i], 80, "%dx%d", dContext->displayModes[i].Width, dContext->displayModes[i].Height);
	}
	aContext->displayModeCount = dContext->displayModeCount;

	//File settingsData = ReadFile("settings");
	//if (settingsData.size > 0)
	//{
	//	Settings *settings = (Settings *)settingsData.data;
	//	if (settings->fullScreen)
	//	{
	//		Draw::ToggleFullScreen(aContext->dContext);
	//		RESIZE_DATA resizeData = { aContext->dContext, settings->resolution.x, settings->resolution.y };
	//		Messaging::Dispatch(&aContext->messenger, RESIZE_WINDOW, &resizeData);
	//	}
	//	Draw::ResizeWindow(aContext->dContext, settings->resolution.x, settings->resolution.y);
	//	ReleaseFile(settingsData);
	//}

	cubeMesh = Draw::GetMesh(aContext->dContext, cubeVertices, cubeStride, cubeVertexCount, cubeIndices, 2, cubeIndexCount);
}

#define TASKBAR_HEIGHT 35.0f
#define Color(r,g,b,a) (Vector4(r / 255.0f, g / 255.0f, b / 255.0f, a / 255.0f))
Vector4 leafColor(0.4f, 200.0f / 255.0f, 35.0f  / 255.0f, 1.0f);
bool showLeaves = true;
uint32 minutes = 0;
bool showedFirst = false;

int Update(AppContext *aContext, Input *input, float dt)
{
	DrawContext *dContext = aContext->dContext;
	UIContext *uiContext = &aContext->uiContext;
	float enterProgress = 1.0f;

	if (!showedFirst)
	{
		enterProgress = Math::Clamp((timer - 5.0f) / 0.25f, 0, 1);
		enterProgress = Math::Sqrt(enterProgress);
	}
	UI::NewWindow(uiContext, Vector2(-200 * (1 - enterProgress), TASKBAR_HEIGHT));
	timer += dt;
	if (minutes == 0 && timer > 4.0f && timer - dt <= 4.0f)
	{
		Messaging::Dispatch(&aContext->messenger, RELOAD_TREE, NULL);
	}
	if (timer > 60.0f)
	{
		timer -= 60.0f;
		minutes++;
	}

	if (input->leftMouseButtonPressed || input->mouseScrollDelta != 0 ||
		input->mouseDX != 0.0f || input->mouseDY != 0.0f)
	{
		Vector2 realScreenSize = Vector2(dContext->screen.viewport.Width, dContext->screen.viewport.Height);
		Vector2 virtualScreenSize = aContext->renderEngine.screenSize;
		Vector2 scale = Vector2(virtualScreenSize.x / realScreenSize.x, virtualScreenSize.y / realScreenSize.y);
		MOUSE_CHANGE_DATA mouseData;
		mouseData.leftButtonDown = input->leftMouseButtonDown;
		mouseData.leftButtonPressed = input->leftMouseButtonPressed;
		mouseData.dx = input->mouseDX * scale.x;
		mouseData.dy = input->mouseDY * scale.y;
		mouseData.positionX = input->mousePositionX * scale.x;
		mouseData.positionY = input->mousePositionY * scale.y;
		mouseData.scrollDelta = input->mouseScrollDelta;
		Messaging::Dispatch(&aContext->messenger, MOUSE_CHANGE, &mouseData);
	}
	if (input->keyPressed['L'])
	{
		showLeaves = !showLeaves;
	}
	if (input->keyPressed['F'])
	{
		Draw::ToggleFullScreen(aContext->dContext);
	}

	float interval = 2.5f;
	if (input->keyPressed['R'])// || (Math::Fmod(timer, interval) < 0.1f && Math::Fmod(timer - dt, interval) > interval - 0.1f))
	{
		Messaging::Dispatch(&aContext->messenger, RELOAD_TREE, NULL);
	}
	if (input->leftButtonPressed)
	{
		if (IT_NUMBER > 3)
			IT_NUMBER--;
		Messaging::Dispatch(&aContext->messenger, RELOAD_TREE, NULL);
	}
	if (input->rightButtonPressed)
	{
		IT_NUMBER++;
		Messaging::Dispatch(&aContext->messenger, RELOAD_TREE, NULL);
	}
	RenderEngine *renderEngine = &aContext->renderEngine;
	renderEngine->cameras[0]->speed = 5.0f; // FIXME(jan): Add a slider to UI so it's adjustable
	REngine::Update(renderEngine, dt);
	Material barkMat =
	{
		Vector4(127.0f / 255.0f, 118.0f / 255.0f, 99.0f / 255.0f, 1) * 0.6f,
		Vector4(127.0f / 255.0f, 118.0f / 255.0f, 99.0f / 255.0f, 1) * 0.1f,
		Vector4(0, 0, 0, 0),
		2.0f
	};
	Material leafMat =
	{
		leafColor * 0.8f,
		leafColor * 0.2f,
		Vector4(0, 0, 0, 0),
		//Vector4(0.2f, 0.7f, 0.5f, 1.0f) * 0.1f,
		1.0f
	};
	
	int exit = 0;

	//
	// UI rendering
	//
#define TREE 0
#define SETTINGS 1
#define DEBUG 2
	
	static int menuState = -1;
	
	std::string labels[] = {"TREE", "SETTINGS", "DEBUG"};
	
	menuState = UI::Menu(uiContext, labels, ARRAYSIZE(labels), menuState);
	
	if (menuState == TREE)
	{
		UI::NextBar(uiContext);
		leafColor = UI::ColorPicker(uiContext, "LEAF COLOR", leafColor);
		showLeaves = UI::Button(uiContext, "LEAVES", showLeaves, true);
		growth = UI::Button(uiContext, "GROW", growth, true);
	}
	if (menuState == SETTINGS)
	{
		UI::NextBar(uiContext);
		uint32_t newSelected = UI::DropDown(uiContext, "RESOLUTION", aContext->displayModeTitles, aContext->displayModeCount, aContext->dContext->currentDisplayMode, 5);
		if (newSelected != aContext->dContext->currentDisplayMode)
		{
			if (dContext->fullScreen)
			{
				RESIZE_DATA resizeData = { aContext->dContext, dContext->displayModes[newSelected].Width, dContext->displayModes[newSelected].Height};
				Messaging::Dispatch(&aContext->messenger, RESIZE_WINDOW, &resizeData);
			}
			Draw::ResizeWindow(dContext, newSelected);
		}
		bool newFullScreen = UI::Button(uiContext, "FULLSCREEN", dContext->fullScreen, true);
		if (dContext->fullScreen != newFullScreen)
		{
			Draw::ToggleFullScreen(aContext->dContext);
		}

		renderEngine->ssao = UI::Button(uiContext, "SSAO", renderEngine->ssao, true);
		renderEngine->shadows = UI::Button(uiContext, "SHADOWS", renderEngine->shadows, true);
		renderEngine->vignette = UI::Button(uiContext, "VIGNETTE", renderEngine->vignette, true);
	}
	if (menuState == DEBUG)
	{
		UI::NextBar(uiContext);
	}

	//
	//	
	//

	if (timer < 4.0f && minutes == 0)
	{
		float alpha = Math::Clamp(timer / 0.5f, 0, 1) - Math::Clamp((timer - 3.5f) / 0.5f, 0, 1);
		alpha *= alpha;
		REngine::RenderText(renderEngine, "STROM", renderEngine->screenSize / 2.0f, Vector4(0.1f, 0.1f, 0.1f, alpha), 80.0f, Vector2(0.5f, 0.5f));
		meshMutex.lock();
	}
	else
	{
		if(!showedFirst)
		{
			if (fadeOut)
			{
				fadeIn = false;
				fadeOut = false;
				showedFirst = true;
			}
			fadeProgress = 0.0f;
		}

		progressMutex.lock();
		const float fadeSpeed = 8.0f;
		if (fadeIn)
		{
			fadeProgress += dt * fadeSpeed;
			if (fadeProgress >= 1.0f)
				fadeIn = false;
		}
		if (fadeOut)
		{
			fadeProgress -= dt * fadeSpeed;
			if (fadeProgress <= 0.0f)
				fadeOut = false;
		}

		float progressAlpha = 0.8f * fadeProgress * fadeProgress;
		Vector4 progressColor = Vector4(0.1f, 0.1f, 0.1f, progressAlpha);
		Vector2 progressPosition = renderEngine->screenSize + Vector2(-290.0f, -110.0f);
		auto progressString = std::to_string((int)(progress * 100)) + "%";
		auto vertexCountString = std::to_string(vertexCount);
		REngine::RenderText(renderEngine, vertexCountString.c_str(), progressPosition + Vector2(250.0f, -60.0f), Vector4(0.1f, 0.1f, 0.1f, 0.8f), 50.0f, Vector2(1.0f, 0.0f));
		REngine::RenderText(renderEngine, progressString.c_str(), progressPosition + Vector2(250.0f, 0.0f), progressColor, 50.0f, Vector2(1.0f, 0.0f));
		REngine::RenderRectangle(renderEngine, progressPosition + Vector2(-250.0f, 60.0f), Vector2(500.0f * progress, 10.0f), progressColor);
		progressMutex.unlock();
		
		float barWidth = 150.0f;
		float xPosition = -(1 - enterProgress) * barWidth;

		std::string fpsString = std::to_string(1.0f / dt);
		fpsString.erase(fpsString.find_first_of('.'), std::string::npos);
		fpsString.insert(0, "FPS: ");
		REngine::RenderText(renderEngine, fpsString.c_str(), Vector2(20.0f + xPosition, renderEngine->screenSize.y - 40.0f), Vector4(1, 1, 1, enterProgress), 20.0f, Vector2(0, 0));
		
		if (UI::Button(uiContext, "NEW TREE", false, Vector2(renderEngine->screenSize.x - 200.0f, 90.0f)))
		{
			Messaging::Dispatch(&aContext->messenger, RELOAD_TREE, NULL);
		}

		meshMutex.lock();
		if (aContext->treeMesh.indexCount)
		{
			for (uint32_t i = 0; i < 1; ++i)
			{
				REngine::Render(renderEngine, &aContext->treeMesh, barkMat, treePosition[i]);
				if(showLeaves)
					REngine::Render(renderEngine, &aContext->leavesMesh, leafMat, treePosition[i]);
			}
			for (uint32_t i = 0; i < voxelCount; ++i)
			{
				REngine::Render(renderEngine, &cubeMesh, barkMat, voxelMatrices[i], 2.0f * voxelScales[i]);
			}
		}
	}

	REngine::RenderRectangle(renderEngine, Vector2(0, 0), Vector2(renderEngine->screenSize.x, TASKBAR_HEIGHT), Vector4(0.1f, 0.1f, 0.1f, 1.0f));
	float TASKBAR_PADDING = 7;
	float TASKBAR_CONTENT_SIZE = TASKBAR_HEIGHT - 2 * TASKBAR_PADDING;

	REngine::RenderText(renderEngine, "STROM", Vector2(TASKBAR_PADDING * 2, TASKBAR_PADDING), Vector4(1, 1, 1, 0.9f), TASKBAR_CONTENT_SIZE);
	//REngine::RenderText(renderEngine, "x", Vector2(renderEngine->screenSize.x - 25.0f, 22.0f), Vector4(1, 1, 1, 0.9f), 35.0f, Vector2(0.5, 0.5f));
	static bool movingWindow = false;
	static bool almostExit = false;
	static Vector2 mousePos;
	Vector4 exitColor = Vector4(0.6f, 0.1f, 0.1f, 0.9f);
	if (input->mousePositionX >= renderEngine->screenSize.x - TASKBAR_HEIGHT - TASKBAR_PADDING &&
		input->mousePositionX <= renderEngine->screenSize.x - TASKBAR_PADDING &&
		input->mousePositionY >= TASKBAR_PADDING && input->mousePositionY <= TASKBAR_HEIGHT - TASKBAR_PADDING)
	{
		exitColor *= 0.8f;
		if (almostExit)
		{
			exitColor *= 0.5f;
		}
		if (input->leftMouseButtonPressed)
		{
			Messaging::Dispatch(&aContext->messenger, Message::UI_INPUT_LOCK, NULL);
			almostExit = true;
		}
		if (almostExit && !input->leftMouseButtonDown)
		{
			exit = 1;
		}
	}
	else if (input->mousePositionY < TASKBAR_HEIGHT && !dContext->fullScreen) 
	{
		if (input->leftMouseButtonPressed)
		{
			movingWindow = true;
			mousePos = Vector2(input->rawPositionX, input->rawPositionY);
			Messaging::Dispatch(&aContext->messenger, Message::UI_INPUT_LOCK, NULL);
		}
	}
	if (almostExit && !input->leftMouseButtonDown)
	{
		almostExit = false;
		Messaging::Dispatch(&aContext->messenger, Message::UI_INPUT_UNLOCK, NULL);
	}
	REngine::RenderRectangle(renderEngine, Vector2(renderEngine->screenSize.x - TASKBAR_CONTENT_SIZE - TASKBAR_PADDING, TASKBAR_PADDING), Vector2(TASKBAR_CONTENT_SIZE, TASKBAR_CONTENT_SIZE), exitColor);
	if (movingWindow)
	{
		if (!input->leftMouseButtonDown)
		{
			movingWindow = false;
			Messaging::Dispatch(&aContext->messenger, Message::UI_INPUT_UNLOCK, NULL);
		}
		else
		{
			float dX = input->rawPositionX - mousePos.x;
			float dY = input->rawPositionY - mousePos.y;
			Draw::MoveWindow(dContext, dX, dY);

			mousePos.x = input->rawPositionX;
			mousePos.y = input->rawPositionY;
		}
	}
	REngine::Render(dContext, renderEngine);
	meshMutex.unlock();
	return exit;
}

void Release(AppContext *aContext)
{
	Settings newSettings = { aContext->renderEngine.screenSize, aContext->dContext->fullScreen };
	WriteFile("settings", &newSettings, sizeof(Settings));

	std::unique_lock<std::mutex> lock(counterMutex);
	end = true;
	while (threadCounter > 0)
		counterVariable.wait(lock);
	REngine::Release(&aContext->renderEngine);
	for (uint32_t i = 0; i < aContext->displayModeCount; ++i)
	{
		free(aContext->displayModeTitles[i]);
	}
	free(aContext->displayModeTitles);
}

void Resize(AppContext *aContext, uint32_t screenX, uint32_t screenY)
{
	Draw::ResizeSwapChain(aContext->dContext);
	if (!aContext->dContext->fullScreen)
	{
		RESIZE_DATA resizeData = { aContext->dContext, screenX, screenY };
		Messaging::Dispatch(&aContext->messenger, RESIZE_WINDOW, &resizeData);
	}
}