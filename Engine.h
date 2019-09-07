#pragma once
#include "Draw.h"
#include "Math.h"
#include "Messenger.h"
#include "Loaders.h"

struct LookAtController
{
	float polar;
	float targetPolar;
	float azimuth;
	float targetAzimuth;
	float radius;
	float targetRadius;

	LookAtController(float polar = PIHALF, float azimuth = 0.0f, float radius = 200.0f):
		polar(polar),
		targetPolar(polar),
		azimuth(azimuth),
		targetAzimuth(azimuth),
		radius(radius),
		targetRadius(radius)
	{}
};

struct Projector
{
	virtual Matrix4x4 GetProjectionMatrix() = 0;
};

struct PerspectiveProjector: public Projector
{
	float aspectRatio;
	float fov;
	float near;
	float far;

	PerspectiveProjector(float aspectRatio, float fov = Math::Deg2Rad(60), 
						 float near = 0.1f, float far = 1000.0f) :
		aspectRatio(aspectRatio),
		fov(fov),
		near(near),
		far(far)
	{}

	Matrix4x4 GetProjectionMatrix();
};



struct OrthographicProjector: public Projector
{
	float left;
	float right;
	float bottom;
	float top;
	float near;
	float far;

	OrthographicProjector(float left, float right, float bottom, float top,
						  float near, float far) :
		left(left),
		right(right),
		bottom(bottom),
		top(top),
		near(near),
		far(far)
	{}

	Matrix4x4 GetProjectionMatrix();
};

struct Camera
{
	LookAtController controller;
	Projector *projector;
	float speed;

	Camera(LookAtController controller, Projector *projector) :
		controller(controller),
		projector(projector)
	{}
};

namespace CameraControl
{
	void UpdatePosition(Camera *camera, float mouseDX, float mouseDY, float scrollDelta);
	void Update(Camera *camera, float dt);
	Matrix4x4 GetProjectionMatrix(Camera *camera);
	Matrix4x4 GetViewMatrix(Camera *camera, Vector3 *camPosition = NULL);
}

struct Material
{
	Vector4 ambientColor;
	Vector4 diffuseColor;
	Vector4 specularColor;
	float roughness;
};

struct RenderEngine: public System
{
	Shaders shaders[10];
	Screen screens[10];
	Mesh meshes[10];
#define MAX_ENTITY_COUNT 1000
	Mesh *entityMeshes;
	Material *entityMaterials;
	Matrix4x4 *entityMatrices;
	uint32_t meshCounter;
	Camera *cameras[2] = { 0 };

	Matrix4x4 rectangleMatrices[500];
	Vector4 rectangleColors[500];
	uint32_t rectCounter;
	
	Matrix4x4 textMatrices[500];
	Vector4 textColors[500];
	Vector4 textSourceRects[500];
	Vector4 textData[500];
	uint32_t textCounter;

	Vector3 lightPosition;
	Texture2D randomTexture;
	Texture2D fontTexture;
	Texture3D noiseTexture;
	Font font;
	Vector2 screenSize;

	bool inputDisabled;
	bool ssao;
	bool shadows;
	bool vignette;
};

namespace REngine
{
	RenderEngine GetEngine(DrawContext *dContext, uint32_t screenX, uint32_t screenY);
	void SetLightPosition(RenderEngine *engine, Vector3 position);
	void Render(RenderEngine *engine, Mesh *mesh, Material material, Vector3 position, float scale = 1.0f);
	void RenderRectangle(RenderEngine *engine, Vector2 position, Vector2 size, Vector4 color);
	//void RenderText(RenderEngine *engine, char *text, Vector2 position, Vector4 color, float fontHeight);
	float GetTextWidth(RenderEngine *engine, const char *text, float fontHeight);
	void RenderText(RenderEngine *engine, const char *text, Vector2 position, Vector4 color, float fontHeight, Vector2 origin = Vector2(0,0));
	void Update(RenderEngine *engine, float dt);
	void Render(DrawContext *dContext, RenderEngine *engine);
	void Release(Camera *camera);
	void Release(RenderEngine *engine);

	void HandleMessage(RenderEngine *engine, Message message, void *data);
};
