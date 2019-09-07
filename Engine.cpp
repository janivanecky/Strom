#include "Draw.h"
#include "Engine.h"
#include "Models.h"
#include "Util.h"
#define STB_IMAGE_IMPLEMENTATION
#define STBI_ONLY_PNG
#include "stb-image.h"

#define CB_MVP 0
#define CB_CAMPOS 1
#define CB_MATERIAL 2
#define CB_SCREEN 3
#define CB_SSAO_KERNELS 4
#define CB_UI_DATA 5
#define CB_SHADOWMATRIX 6
#define CB_RECT_DATA 7
#define CB_TEXT_DATA 7
#define CB_MODEL_DATA 7
#define CB_SETTINGS 8

#define TEX_DEFAULT 0
#define TEX_GEOM 1
#define TEX_SHADOW 3
#define TEX_RANDOM 6
#define TEX_NOISE 7
#define TEX_FONT 8

#define SHADERS_PBR 0
#define SHADERS_GEOM 1
#define SHADERS_SSAO 2
#define SHADERS_BLUR 3
#define SHADERS_FX 4
#define SHADERS_SHADOW 5
#define SHADERS_UI 6
#define SHADERS_FONT 7

#define SCREEN_SCENE 0
#define SCREEN_GEOM 1
#define SCREEN_SSAO 2
#define SCREEN_BLUR 3
#define SCREEN_SHADOW 4
#define SCREEN_DIFFUSE 5
#define SCREEN_SPECULAR 6

#define MESH_QUAD 0
#define MESH_FLOOR 1
#define MESH_RECT 2

#define CAMERA_SCENE 0
#define CAMERA_LIGHT 1


void CameraControl::UpdatePosition(Camera *camera, float mouseDX, float mouseDY, float scrollDelta)
{
	camera->controller.targetPolar -= mouseDY * 6.0f;
	camera->controller.targetPolar = MIN(PIHALF - 0.005f, camera->controller.targetPolar);
	camera->controller.targetPolar = MAX(0, camera->controller.targetPolar);
	camera->controller.targetPolar = MIN(PI, camera->controller.targetPolar);
	camera->controller.targetAzimuth -= mouseDX * 6.0f;
	camera->controller.targetRadius -= scrollDelta * 0.04f;
}

void CameraControl::Update(Camera *camera, float dt)
{
	camera->controller.polar += (camera->controller.targetPolar - camera->controller.polar) * dt * camera->speed;
	camera->controller.azimuth += (camera->controller.targetAzimuth- camera->controller.azimuth) * dt * camera->speed;
	camera->controller.radius += (camera->controller.targetRadius - camera->controller.radius) * dt * camera->speed;
}


Matrix4x4 CameraControl::GetProjectionMatrix(Camera *camera)
{
	Matrix4x4 projectionMatrix = camera->projector->GetProjectionMatrix();
	return projectionMatrix;
}

Matrix4x4 CameraControl::GetViewMatrix(Camera *camera, Vector3 *camPosition)
{
	Vector3 camPos = Vector3(Math::Sin(camera->controller.polar) * Math::Sin(camera->controller.azimuth),
							 Math::Cos(camera->controller.polar),
							 Math::Sin(camera->controller.polar) * Math::Cos(camera->controller.azimuth))
		* camera->controller.radius;
	// To compute Lookat matrix, we need an up direction, but in case the direction of our 
	// view is actually aligned with a y-axis, we need alternative up direction.
	// We choose a vector that is in x-z plane and points in the direction we're from origin in 
	// this plane.
	Vector3 alternativeUp = Math::Normalize(
		Vector3(-Math::Sin(camera->controller.azimuth), 0.0f, -Math::Cos(camera->controller.azimuth)));
	Vector3 up = camera->controller.polar == 0 ? alternativeUp :
		(camera->controller.polar == PI ? -alternativeUp : Vector3(0, 1, 0));
	Matrix4x4 view = Math::LookAt(camPos, Vector3(0, 0, 0), up);
	if (camPosition)
	{
		*camPosition = camPos;
	}
	return view;
}

Matrix4x4 PerspectiveProjector::GetProjectionMatrix()
{
	Matrix4x4 projection = Math::GetPerspectiveProjectionDXRH(fov, aspectRatio, near, far);
	return projection;
}

Matrix4x4 OrthographicProjector::GetProjectionMatrix()
{
	Matrix4x4 projection = Math::GetOrthographicsProjectionDXRH(left, right, bottom, top, near, far);
	return projection;
}


float Random(float min, float max)
{
	return (rand() % 1000) / 1000.0f * (max - min) + min;
}


uint32_t VectorToRGBA(Vector4 v)
{
	uint32_t result;
	unsigned char r = (unsigned char)(v.x * 255);
	unsigned char g = (unsigned char)(v.y * 255);
	unsigned char b = (unsigned char)(v.z * 255);
	unsigned char a = (unsigned char)(v.w * 255);
	result = a << 24 | b << 16 | g << 8 | r;
	return result;
}

#define NO_KERNELS 64
uint32_t randomTex[16];
Vector4 vecs[NO_KERNELS];
void GetSSAOData()
{
	for (uint32_t i = 0; i < 16; ++i)
	{
		float azimuth = Random(0, PI2);
		Vector4 randomVec = Vector4(Math::Cos(azimuth), 0.0f, Math::Sin(azimuth), 0.0f);
		randomTex[i] = VectorToRGBA(randomVec * 0.5f + Vector4(0.5f, 0.5f, 0.5f, 0.0f));
	}

	for (uint32_t i = 0; i < NO_KERNELS; ++i)
	{
		float polar = Random(0, PIHALF - 0.15f), azimuth = Random(0, PI2);
		vecs[i] = Vector4(Math::Sin(polar) * Math::Cos(azimuth),
						  Math::Cos(polar),
						  Math::Sin(polar) * Math::Sin(azimuth), 0.0f);
		float scale = (float)i / (float)NO_KERNELS;
		vecs[i] *= 0.1f + 0.9f * scale * scale;
	}
}

void REngine::SetLightPosition(RenderEngine *engine, Vector3 position)
{
	engine->lightPosition = position;
}

void REngine::Render(RenderEngine *engine, Mesh *mesh, Material material, Vector3 position, float scale)
{
	engine->entityMeshes[engine->meshCounter] = *mesh;
	engine->entityMaterials[engine->meshCounter] = material;
	engine->entityMatrices[engine->meshCounter++] = Math::GetTranslation(position) * Math::GetScale(scale, scale, scale);
}

float GetTextWidth(Font *font, const char *text, float fontHeight)
{
	float width = 0.0f;
	float paddingVertical = font->paddingTop + font->paddingBot;
	float scaleFactor = fontHeight / (font->fontHeight - paddingVertical);
	Glyph glyph;
	while (*text)
	{
		uint32_t ascii = *text++;
		glyph = font->glyphs[ascii];
		width += (glyph.advance + 4.0f) * scaleFactor;
	}
	width -= font->paddingRight * scaleFactor;
	return width;
}

void REngine::Update(RenderEngine *engine, float dt)
{
	CameraControl::Update(engine->cameras[CAMERA_SCENE], dt);
}


float REngine::GetTextWidth(RenderEngine *engine, const char *text, float fontHeight)
{
	return GetTextWidth(&engine->font, text, fontHeight);
}


void REngine::RenderText(RenderEngine *engine, const char *text, Vector2 position, Vector4 color, float fontHeight, Vector2 origin)
{
	Font *font = &engine->font;

	Vector2 texSize = Vector2((float)engine->fontTexture.width, (float)engine->fontTexture.height);

	float paddingHorizontal = font->paddingLeft + font->paddingRight;
	float paddingVertical = font->paddingTop + font->paddingBot;
	float scaleFactor = fontHeight / (font->fontHeight - paddingVertical);
	Vector2 currentPosition = position - Vector2(origin.x * GetTextWidth(font, text, fontHeight), origin.y * fontHeight);
	currentPosition.y -= font->paddingTop * scaleFactor;
	while (*text)
	{
		uint32_t ascii = *text++;
		Glyph glyph = font->glyphs[ascii];
		Rect sourceRect;
		sourceRect.left = glyph.coord.x / texSize.x;
		sourceRect.top = glyph.coord.y / texSize.y;
		sourceRect.width = glyph.size.x / texSize.x;
		sourceRect.height = glyph.size.y / texSize.y;;

		float xPos = currentPosition.x + glyph.offset.x * scaleFactor;
		float yPos = currentPosition.y + glyph.offset.y * scaleFactor;
		float width = glyph.size.x * scaleFactor;
		float height = glyph.size.y * scaleFactor;
		Matrix4x4 matrix = Math::GetTranslation(xPos, yPos, 0) * Math::GetScale(width, height, 1.0f);

		currentPosition.x += (glyph.advance + 4.0f) * scaleFactor;

		engine->textColors[engine->textCounter] = color;
		engine->textMatrices[engine->textCounter] = matrix;
		engine->textData[engine->textCounter] = Vector4(fontHeight, 0, 0, 0);
		engine->textSourceRects[engine->textCounter++] = Vector4(sourceRect.left, sourceRect.top, sourceRect.width, sourceRect.height);
	}

}

void REngine::RenderRectangle(RenderEngine *engine, Vector2 position, Vector2 size, Vector4 color)
{
	Matrix4x4 matrix = Math::GetTranslation(position.x, position.y, 0.0f) * Math::GetScale(size.x, size.y, 1.0f);
	engine->rectangleMatrices[engine->rectCounter] = matrix;
	engine->rectangleColors[engine->rectCounter++] = color;
}

RenderEngine REngine::GetEngine(DrawContext *dContext, uint32_t screenX, uint32_t screenY)
{
	RenderEngine result;
	result.shaders[SHADERS_PBR] = Draw::GetShaders(
		dContext, "EffectVertexShader.hlsl", "PixelShader.hlsl", Draw::VERTEX_ATTRIBUTES::POSITION_TEXCOORD);
	result.shaders[SHADERS_SHADOW] = Draw::GetShaders(
		dContext, "VertexShader.hlsl", NULL, Draw::VERTEX_ATTRIBUTES::POSITION_NORMAL);
	result.shaders[SHADERS_GEOM] = Draw::GetShaders(
		dContext, "GeomVertexShader.hlsl", "GeomPixelShader.hlsl", Draw::VERTEX_ATTRIBUTES::POSITION_NORMAL);
	result.shaders[SHADERS_FX] = Draw::GetShaders(
		dContext, "EffectVertexShader.hlsl", "EffectPixelShader.hlsl", Draw::VERTEX_ATTRIBUTES::POSITION_TEXCOORD);
	result.shaders[SHADERS_BLUR] = Draw::GetShaders(
		dContext, "EffectVertexShader.hlsl", "BlurPixelShader.hlsl", Draw::VERTEX_ATTRIBUTES::POSITION_TEXCOORD);
	result.shaders[SHADERS_SSAO] = Draw::GetShaders(
		dContext, "EffectVertexShader.hlsl", "SSAOPixelShader.hlsl", Draw::VERTEX_ATTRIBUTES::POSITION_TEXCOORD);
	result.shaders[SHADERS_UI] = Draw::GetShaders(
		dContext, "UIVertexShader.hlsl", "UIPixelShader.hlsl", Draw::VERTEX_ATTRIBUTES::POSITION_TEXCOORD);
	result.shaders[SHADERS_FONT] = Draw::GetShaders(
		dContext, "TextVertexShader.hlsl", "TextPixelShader.hlsl", Draw::VERTEX_ATTRIBUTES::POSITION_TEXCOORD);
	result.screens[SCREEN_SCENE] = Draw::GetScreen(dContext, screenX, screenY, false, COLOR_BUFFER);
	result.screens[SCREEN_GEOM] = Draw::GetScreen(dContext, screenX, screenY, false);
	result.screens[SCREEN_SSAO] = Draw::GetScreen(dContext, screenX, screenY, false, COLOR_BUFFER);
	result.screens[SCREEN_BLUR] = Draw::GetScreen(dContext, screenX, screenY, false, COLOR_BUFFER);
	result.screens[SCREEN_DIFFUSE] = Draw::GetScreen(dContext, screenX, screenY, false, COLOR_BUFFER);
	result.screens[SCREEN_SPECULAR] = Draw::GetScreen(dContext, screenX, screenY, false, COLOR_BUFFER);
	result.screens[SCREEN_SHADOW] = Draw::GetScreen(dContext, 2048, 2048, false,
													BufferTypes::DEPTH_BUFFER);

	result.meshes[MESH_QUAD] = Draw::GetMesh(dContext, quadVertices, sizeof(float) * 6, 4, quadIndices, sizeof(uint16_t), 6);
	result.meshes[MESH_FLOOR] = Draw::GetMesh(dContext, floorVertices, sizeof(float) * 12, 4, floorIndices, sizeof(uint16_t), 6);
	result.meshes[MESH_RECT] = Draw::GetMesh(dContext, rectangleVertices, sizeof(float) * 6, 4, rectangleIndices, sizeof(uint16_t), 6);

	result.screenSize = Vector2((float) screenX, (float)screenY);

	result.cameras[CAMERA_LIGHT] = new Camera(
		LookAtController(1.0f, 0.0f, 10.0f), new OrthographicProjector(-100.0f, 100.0f,
																	   -100.0f, 100.0f,
																	   100.0f, -200.0f));
	result.cameras[CAMERA_SCENE] = new Camera(LookAtController(PIHALF - 0.1f, 0.0f, 150.0f),
											  new PerspectiveProjector((float)screenX / screenY,
																	   Math::Deg2Rad(45),
																	   0.1f, 300.0f));
	GetSSAOData();
	result.randomTexture = Draw::GetTexture2D(dContext, 4, 4, randomTex);

	float *noiseData = (float *)malloc(sizeof(float) * 100000000);
	float *nData = noiseData;
	for (uint32_t z = 0; z < 100; ++z)
	{
		for (uint32_t y = 0; y < 100; ++y)
		{
			for (uint32_t x = 0; x < 100; ++x)
			{
				*(nData++) = Random(-1.0f, 1.0f);
			}
		}
	}
	result.noiseTexture = Draw::GetTexture3D(dContext, 100, 100, 100, noiseData);
	free(noiseData);


	int width, height, noChannels;
	File fontTexture = ReadFile("din.png");
	unsigned char *data = stbi_load_from_memory((unsigned char *)fontTexture.data, fontTexture.size, &width, &height, &noChannels, 0);
	ReleaseFile(fontTexture);
	result.fontTexture = Draw::GetTexture2D(dContext, width, height, data);
	stbi_image_free(data);
	result.font = Loaders::LoadFont("din.fnt");

	result.meshCounter = 0;
	result.rectCounter = 0;
	result.textCounter = 0;
	result.inputDisabled = false;

	result.ssao = true;
	result.shadows = true;
	result.vignette = true;

	result.entityMeshes = new Mesh[MAX_ENTITY_COUNT];
	result.entityMaterials = new Material[MAX_ENTITY_COUNT];
	result.entityMatrices = new Matrix4x4[MAX_ENTITY_COUNT];


	return result;
}

void REngine::Render(DrawContext *dContext, RenderEngine *engine)
{
	Vector3 camPos;
	Matrix4x4 p = CameraControl::GetProjectionMatrix(engine->cameras[CAMERA_SCENE]);
	Matrix4x4 v = Math::GetTranslation(Vector3(0, -20, 0)) * CameraControl::GetViewMatrix(engine->cameras[CAMERA_SCENE], &camPos);
	Matrix4x4 iP = Math::Invert(p);
	Matrix4x4 matrices[3] = { p,v,iP };

	Vector3 lightPos;
	p = CameraControl::GetProjectionMatrix(engine->cameras[CAMERA_LIGHT]);
	v = CameraControl::GetViewMatrix(engine->cameras[CAMERA_LIGHT], &lightPos);
	iP = Math::Invert(p);
	Matrix4x4 matricesLight[3] = { p,v,iP };
	Matrix4x4 matricesShadowMap[2] = { matricesLight[0] * matricesLight[1], Math::Invert(matrices[1]) };

	Draw::SetTexture(dContext, TEX_RANDOM, &engine->randomTexture);
	Draw::SetTexture(dContext, TEX_NOISE, &engine->noiseTexture);
	Draw::SetTexture(dContext, TEX_FONT, &engine->fontTexture);
	Draw::SetConstantData(dContext, CB_MVP, (void *)matricesLight, sizeof(Matrix4x4) * 3);
	Vector4 camLightPos[2] = { Vector4(camPos, 1.0f), Vector4(lightPos, 1.0f) * 100.0f};
	Draw::SetConstantData(dContext, CB_CAMPOS, (void *)camLightPos, sizeof(Vector4) * 2);
	Draw::SetConstantData(dContext, CB_SCREEN, (void *)&engine->screenSize, sizeof(Vector2));
	Draw::SetConstantData(dContext, CB_SSAO_KERNELS, vecs, sizeof(Vector4) * NO_KERNELS);
	Draw::SetConstantData(dContext, CB_SHADOWMATRIX, (void *)matricesShadowMap,
						  sizeof(Matrix4x4) * 2);
	Vector4 settings = Vector4(engine->ssao, engine->shadows, engine->vignette, 0);
	Draw::SetConstantData(dContext, CB_SETTINGS, (void *)&settings, sizeof(Vector4));
	Matrix4x4 uiProjectionMatrix = Math::GetOrthographicsProjectionDXRH(0, engine->screenSize.x, engine->screenSize.y, 0, 0.5f, -0.5f);

	Vector4 t = uiProjectionMatrix * Vector4(0, 200, 0, 1);
	Vector4 t2 = uiProjectionMatrix * Vector4(0, 0, 0, 1);
	Draw::SetConstantData(dContext, CB_UI_DATA, &uiProjectionMatrix, sizeof(Matrix4x4));

	Draw::ClearScreen(dContext, 1, 0, 0, 1);
	Draw::ClearScreen(dContext, &engine->screens[SCREEN_GEOM], 0, 0, 1, 0);
	Draw::ClearScreen(dContext, &engine->screens[SCREEN_SCENE], 1, 1, 1, 1);
	Draw::ClearScreen(dContext, &engine->screens[SCREEN_SSAO]);
	Draw::ClearScreen(dContext, &engine->screens[SCREEN_SHADOW]);
	Draw::ClearScreen(dContext, &engine->screens[SCREEN_BLUR]);
	Draw::ClearScreen(dContext, &engine->screens[SCREEN_DIFFUSE]);
	Draw::ClearScreen(dContext, &engine->screens[SCREEN_SPECULAR]);
	
	Draw::SetScreen(dContext, &engine->screens[SCREEN_SHADOW]);
	Draw::SetShaders(dContext, &engine->shaders[SHADERS_SHADOW]);

	Draw::SetAlphaBlend(dContext, false);
	Draw::SetDepthTest(dContext, true);

	for (uint32_t i = 0; i < engine->meshCounter; ++i)
	{
		Matrix4x4 model = engine->entityMatrices[i];
		Draw::SetConstantData(dContext, CB_MODEL_DATA, &model, sizeof(Matrix4x4));
		Draw::Draw(dContext, &engine->entityMeshes[i]);
	}

	// GEOMETRY PASS

	Draw::SetConstantData(dContext, CB_MVP, (void *)matrices, sizeof(Matrix4x4) * 3);
	Screen screens[] = { engine->screens[SCREEN_GEOM], engine->screens[SCREEN_DIFFUSE], engine->screens[SCREEN_SPECULAR] };
	Draw::SetScreen(dContext, screens, 3);
	Draw::SetShaders(dContext, &engine->shaders[SHADERS_GEOM]);

	Matrix4x4 identity = Math::GetIdentity();
	Material floorMaterial =
	{
		Vector4(1,1,1,1) * 0.8f,
		Vector4(1,1,1,1) * 1.0f,
		Vector4(1,1,1,1) * 0.5f,
		4.0f,
	};
	Draw::SetConstantData(dContext, CB_MATERIAL, &floorMaterial,
						  sizeof(Vector4) * 4);
	Draw::SetConstantData(dContext, CB_MODEL_DATA, &identity, sizeof(Matrix4x4));
	Draw::Draw(dContext, &engine->meshes[MESH_FLOOR]);
	for (uint32_t i = 0; i < engine->meshCounter; ++i)
	{
		Matrix4x4 model = engine->entityMatrices[i];
		Draw::SetConstantData(dContext, CB_MODEL_DATA, &model, sizeof(Matrix4x4));
		Draw::SetConstantData(dContext, CB_MATERIAL, &engine->entityMaterials[i],
							  sizeof(Vector4) * 4);
		Draw::Draw(dContext, &engine->entityMeshes[i]);
	}

	// SSAO PASS

	Draw::SetScreen(dContext, &engine->screens[SCREEN_SSAO]);
	Draw::SetShaders(dContext, &engine->shaders[SHADERS_SSAO]);
	Draw::SetTexture(dContext, TEX_GEOM, &engine->screens[SCREEN_GEOM]);

	Draw::Draw(dContext, &engine->meshes[MESH_QUAD]);

	Draw::SetScreen(dContext, &engine->screens[SCREEN_BLUR]);
	Draw::SetShaders(dContext, &engine->shaders[SHADERS_BLUR]);
	Draw::SetTexture(dContext, TEX_DEFAULT, &engine->screens[SCREEN_SSAO]);
	Draw::Draw(dContext, &engine->meshes[MESH_QUAD]);

	Draw::SetScreen(dContext, &engine->screens[SCREEN_SCENE]);
	Draw::SetShaders(dContext, &engine->shaders[SHADERS_PBR]);
	Draw::SetTexture(dContext, 0, &engine->screens[SCREEN_BLUR]);
	Draw::SetTexture(dContext, 3, &engine->screens[SCREEN_SHADOW]);
	Draw::SetTexture(dContext, 4, &engine->screens[SCREEN_DIFFUSE]);
	Draw::SetTexture(dContext, 5, &engine->screens[SCREEN_SPECULAR]);
	Draw::Draw(dContext, &engine->meshes[MESH_QUAD]);

	Draw::SetScreen(dContext);
	Draw::SetShaders(dContext, &engine->shaders[SHADERS_FX]);
	Draw::SetTexture(dContext, TEX_DEFAULT, &engine->screens[SCREEN_SCENE]);
	Draw::Draw(dContext, &engine->meshes[MESH_QUAD]);
	
	Draw::SetDepthTest(dContext, false);
	Draw::SetAlphaBlend(dContext, true);
	Draw::SetShaders(dContext, &engine->shaders[SHADERS_UI]);
	
	struct RectData
	{
		Matrix4x4 models[500];
		Vector4 colors[500];
	};
	RectData rectData = {};
	memcpy(rectData.models, engine->rectangleMatrices, sizeof(Matrix4x4) * engine->rectCounter);
	memcpy(rectData.colors, engine->rectangleColors, sizeof(Vector4) * engine->rectCounter);
	Draw::SetConstantData(dContext, CB_RECT_DATA, &rectData, sizeof(RectData));
	Draw::DrawMulti(dContext, &engine->meshes[MESH_RECT], engine->rectCounter);

	Draw::SetShaders(dContext, &engine->shaders[SHADERS_FONT]);


	struct TextData
	{
		Matrix4x4 models[500];
		Vector4 colors[500];
		Vector4 sourceRects[500];
		Vector4 fontSizes[500];
	};
	TextData textData;
	memcpy(textData.models, engine->textMatrices, sizeof(Matrix4x4) * engine->textCounter);
	memcpy(textData.colors, engine->textColors, sizeof(Vector4) * engine->textCounter);
	memcpy(textData.sourceRects, engine->textSourceRects, sizeof(Vector4) * engine->textCounter);
	memcpy(textData.fontSizes, engine->textData, sizeof(Vector4) * engine->textCounter);
	Draw::SetConstantData(dContext, CB_TEXT_DATA, &textData, sizeof(TextData));
	Draw::DrawMulti(dContext, &engine->meshes[MESH_RECT], engine->textCounter);

	Draw::Swap(dContext);
	engine->meshCounter = 0;
	engine->rectCounter = 0;
	engine->textCounter = 0;
}

void REngine::Release(Camera *camera)
{
	delete camera->projector;
}

void REngine::Release(RenderEngine *engine)
{
	for (uint32_t i = 0; i < ARRAYSIZE(engine->cameras); ++i)
	{
		REngine::Release(engine->cameras[i]);
		delete engine->cameras[i];
	}
}

void REngine::HandleMessage(RenderEngine *engine, Message message, void *data)
{
	switch (message)
	{
		case MOUSE_CHANGE:
		{
			MOUSE_CHANGE_DATA *floatData = (MOUSE_CHANGE_DATA *)data;
			if ((floatData->leftButtonDown || floatData->scrollDelta != 0.0f) && !engine->inputDisabled)
			{
				CameraControl::UpdatePosition(engine->cameras[CAMERA_SCENE],
											  floatData->dx / engine->screenSize.x, floatData->dy / engine->screenSize.x,
											  floatData->scrollDelta);
			}
		} break;
		case UI_INPUT_LOCK:
		{
			engine->inputDisabled = true;
		} break;
		case UI_INPUT_UNLOCK:
		{
			engine->inputDisabled = false;
		} break;
		case RESIZE_WINDOW:
		{
			RESIZE_DATA *resizeData = (RESIZE_DATA *)data;
			DrawContext *dContext = resizeData->dContext;
			Draw::Release(dContext, &engine->screens[SCREEN_SCENE]);
			Draw::Release(dContext, &engine->screens[SCREEN_GEOM]);
			Draw::Release(dContext, &engine->screens[SCREEN_SSAO]);
			Draw::Release(dContext, &engine->screens[SCREEN_BLUR]);
			Draw::Release(dContext, &engine->screens[SCREEN_DIFFUSE]);
			Draw::Release(dContext, &engine->screens[SCREEN_SPECULAR]);
			uint32_t screenX = resizeData->screenWidth, screenY = resizeData->screenHeight;
			engine->screens[SCREEN_SCENE] = Draw::GetScreen(dContext, screenX, screenY, false, COLOR_BUFFER);
			engine->screens[SCREEN_GEOM] = Draw::GetScreen(dContext, screenX, screenY, false);
			engine->screens[SCREEN_SSAO] = Draw::GetScreen(dContext, screenX, screenY, false, COLOR_BUFFER);
			engine->screens[SCREEN_BLUR] = Draw::GetScreen(dContext, screenX, screenY, false, COLOR_BUFFER);
			engine->screens[SCREEN_DIFFUSE] = Draw::GetScreen(dContext, screenX, screenY, false, COLOR_BUFFER);
			engine->screens[SCREEN_SPECULAR] = Draw::GetScreen(dContext, screenX, screenY, false, COLOR_BUFFER);
			engine->cameras[CAMERA_SCENE]->projector = new PerspectiveProjector((float)screenX / screenY, Math::Deg2Rad(45), 0.1f, 300.0f);
			engine->screenSize = Vector2((float)screenX, (float)screenY);
		} break;
	}
}
