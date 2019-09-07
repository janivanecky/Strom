#pragma once
#include <d3d11.h>
#include <d3dcompiler.h>
#include <stdint.h>
#include "IO.h"
#include <stdio.h>

#define CHECK_WIN_ERROR(x,y) if(FAILED(x)) {printf("%s\n", y);}
#define CHECK_WIN_ERROR_CLEANUP(x,y, z) if(FAILED(x)) {printf("%s\n", y); z}
#define RELEASE_DX_RESOURCE(x) x ? x->Release() : 0; x = NULL;	
#define MAX(a, b) (((a) > (b)) ? (a) : (b))

#undef far
#undef near
#define DEBUG

struct Screen
{
	ID3D11RenderTargetView *screenTargetView = NULL;
	ID3D11DepthStencilView *screenDepthMap = NULL;
	ID3D11ShaderResourceView *screenTargetViewResource = NULL;
	ID3D11ShaderResourceView *screenDepthMapResource = NULL;
	D3D11_VIEWPORT viewport;

	bool multisample;
};

struct Shaders
{
	ID3D11VertexShader *vertexShader;
	ID3D11PixelShader *pixelShader;
	ID3D11InputLayout *inputLayout;
};

struct DrawContext
{
	ID3D11Device *device;
	ID3D11DeviceContext *deviceContext;
	IDXGISwapChain *swapChain;
	
	Screen screen;

	ID3D11Buffer *constBuffers[10] = { 0 };
	uint32_t bufferSizes[10] = { 0 };

	void *textures[10] = { 0 };

	IUnknown **objects;
	uint32_t objectCapacity = 0;
	uint32_t objectCount = 0;

	ID3D11SamplerState *clampSampler;
	ID3D11SamplerState *wrapSampler;

	ID3D11BlendState *blendState3D;
	ID3D11BlendState *blendStateAlpha;

	ID3D11DepthStencilState *depthTestOnState;
	ID3D11DepthStencilState *depthTestOffState;

	HWND windowHandle;

	DXGI_MODE_DESC *displayModes;
	uint32_t displayModeCount;
	uint32_t currentDisplayMode;

	bool fullScreen = false;
};

struct Mesh
{
	ID3D11Buffer *vertexBuffer;
	ID3D11Buffer *indexBuffer;
	uint32_t vertexStride;
	uint32_t indexCount;
	DXGI_FORMAT indexFormat;
};

struct Texture2D
{
	ID3D11ShaderResourceView *texture;
	UINT width;
	UINT height;
};

struct Texture3D
{
	ID3D11ShaderResourceView *texture;
	UINT width;
	UINT height;
	UINT depth;
};

enum BufferTypes
{
	COLOR_BUFFER = 1,
	DEPTH_BUFFER = 2
};

inline BufferTypes operator|(BufferTypes a, BufferTypes b)
{
	return static_cast<BufferTypes>(static_cast<int>(a) | static_cast<int>(b));
}

inline BufferTypes operator&(BufferTypes a, BufferTypes b)
{
	return static_cast<BufferTypes>(static_cast<int>(a) & static_cast<int>(b));
}

namespace Draw
{
	enum VERTEX_ATTRIBUTES
	{
		POSITION,
		POSITION_NORMAL,
		POSITION_TEXCOORD
	};

	DrawContext GetContext(HWND window, UINT screenWidth, UINT screenHeight, bool rightHanded = true);
	void ResizeSwapChain(DrawContext *context, UINT width = 0, UINT height = 0);
	void ResizeWindow(DrawContext *context, UINT displayModeID);
	void ResizeWindow(DrawContext *context, UINT windowWidth, UINT windowHeight);
	void MoveWindow(DrawContext *context, UINT dx, UINT dy);
	void ToggleFullScreen(DrawContext *context);

	Texture2D GetTexture2D(DrawContext *context, UINT width, UINT height, void *data);
	Texture3D GetTexture3D(DrawContext *context, UINT width, UINT height, UINT depth, void *data);
	Shaders GetShaders(DrawContext *context, char *vertexShaderFile, char *pixelShaderFile, Draw::VERTEX_ATTRIBUTES attributes);
	Mesh GetMesh(DrawContext *context, void *vertices, uint32_t vertexStride, uint32_t vertexCount, 
					   void *indicies, uint32_t indexSize, uint32_t indexCount);
	Screen GetScreen(DrawContext *context, UINT screenWidth, UINT screenHeight, 
					 bool multisample, BufferTypes bufferTypes = COLOR_BUFFER | DEPTH_BUFFER);

	void SetShaders(DrawContext *context, Shaders *shaders);
	void Draw(DrawContext *context, Mesh *mesh);
	void DrawMulti(DrawContext *context, Mesh *mesh, uint32_t count);

	void SetConstantData(DrawContext *context, uint32_t slot, void *data, uint32_t dataSize);

	void SetTexture(DrawContext *context, uint32_t slot, Texture2D *texture);
	void SetTexture(DrawContext *context, uint32_t slot, Texture3D *texture);
	void SetTexture(DrawContext *context, uint32_t slot, Screen *screen);

	void SetScreen(DrawContext *context);
	void SetScreen(DrawContext *context, Screen *screen);
	void SetScreen(DrawContext *context, Screen *screen, uint32_t screenCount);

	void ClearScreen(DrawContext *context, float r = 0.0f, float g = 0.0f, float b = 0.0f, float a = 1.0f);
	void ClearScreen(DrawContext *context, Screen *screen, float r = 0.0f, float g = 0.0f, float b = 0.0f, float a = 1.0f);

	void SetAlphaBlend(DrawContext *context, bool alphaBlend);
	void SetDepthTest(DrawContext *context, bool depthTest);

	void Swap(DrawContext *context);

	void Release(DrawContext *context);
	void Release(DrawContext *context, Screen *screen);
	void Release(DrawContext *context, Mesh *mesh);
}

#ifdef DRAW
#define DEBUG

void AddObject(DrawContext *context, IUnknown *object)
{
	if (context->objectCapacity <= context->objectCount)
	{
		uint32_t newCapacity = 2 * context->objectCapacity;
		IUnknown **newObjects = new IUnknown *[newCapacity];
		memcpy(newObjects, context->objects, sizeof(IUnknown *) * context->objectCapacity);
		delete []context->objects;
		context->objectCapacity = newCapacity;
		context->objects = newObjects;
	}

	context->objects[context->objectCount++] = object;
}

void RemoveObject(DrawContext *context, IUnknown *object)
{
	for (uint32_t i = 0; i < context->objectCount; ++i)
	{
		if (context->objects[i] == object)
		{
			RELEASE_DX_RESOURCE(context->objects[i]);
			--context->objectCount;
			if(i < context->objectCount)
				context->objects[i] = context->objects[context->objectCount];
			break;
		}
	}
}

#define ABS(x, y) MAX(x - y, y - x)
void Draw::ResizeWindow(DrawContext *context, UINT windowWidth, UINT windowHeight)
{
	UINT bestMatchingDisplayMode;
	UINT distance = 10000;
	for (uint32_t i = 0; i < context->displayModeCount; ++i)
	{
		UINT currentDistance = ABS(context->displayModes[i].Width, windowWidth) + ABS(context->displayModes[i].Height, windowHeight);
		if (currentDistance < distance)
		{
			distance = currentDistance;
			bestMatchingDisplayMode = i;
		}
	}
	ResizeWindow(context, bestMatchingDisplayMode);
}

void Draw::ResizeWindow(DrawContext *context, UINT displayModeID)
{
	context->currentDisplayMode = displayModeID;
	if (!context->fullScreen)
	{
		context->swapChain->ResizeTarget(&context->displayModes[displayModeID]);
	}
}

void Draw::ResizeSwapChain(DrawContext *context, UINT width, UINT height)
{
	if (context->screen.screenTargetView)
	{
		RemoveObject(context, context->screen.screenTargetView);
	}
	if (context->screen.screenDepthMap)
	{
		RemoveObject(context, context->screen.screenDepthMap);
	}
	context->swapChain->ResizeBuffers(0, width, height, DXGI_FORMAT_R8G8B8A8_UNORM, DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH);

	ID3D11Texture2D *backBuffer;
	HRESULT hr = context->swapChain->GetBuffer(0, __uuidof(ID3D11Texture2D), (void **)&backBuffer);
	CHECK_WIN_ERROR(hr, "Error calling IDXGISwapChain::GetBuffer");

	hr = context->device->CreateRenderTargetView(backBuffer, NULL, &context->screen.screenTargetView);
	CHECK_WIN_ERROR(hr, "Error calling ID3D11Device::CreateRenderTargetView");
	backBuffer->Release();

	D3D11_TEXTURE2D_DESC depthStencilDesc = {};
	backBuffer->GetDesc(&depthStencilDesc);
	depthStencilDesc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
	depthStencilDesc.Usage = D3D11_USAGE_DEFAULT;
	depthStencilDesc.BindFlags = D3D11_BIND_DEPTH_STENCIL;

	ID3D11Texture2D* depthStencilTex;
	hr = context->device->CreateTexture2D(&depthStencilDesc, NULL, &depthStencilTex);
	CHECK_WIN_ERROR(hr, "Error creating Depth Buffer texture\n");

	CD3D11_DEPTH_STENCIL_VIEW_DESC depthStencilViewDesc(D3D11_DSV_DIMENSION_TEXTURE2DMS, DXGI_FORMAT_D24_UNORM_S8_UINT);
	hr = context->device->CreateDepthStencilView(depthStencilTex, &depthStencilViewDesc, &context->screen.screenDepthMap);
	CHECK_WIN_ERROR(hr, "Error creating Depth stencil view");
	depthStencilTex->Release();

	context->screen.screenTargetViewResource = NULL;

	context->screen.viewport = { 0 };
	context->screen.viewport.Width = (float)depthStencilDesc.Width;
	context->screen.viewport.Height = (float)depthStencilDesc.Height;
	context->screen.viewport.MinDepth = 0.0f;
	context->screen.viewport.MaxDepth = 1.0f;

	AddObject(context, context->screen.screenTargetView),
		AddObject(context, context->screen.screenDepthMap);

}

void Draw::ToggleFullScreen(DrawContext *context)
{
	context->fullScreen = !context->fullScreen;
	if (context->fullScreen)
	{
		UINT width = context->displayModes[context->displayModeCount - 1].Width, height = context->displayModes[context->displayModeCount - 1].Height;
		context->swapChain->ResizeTarget(&context->displayModes[context->displayModeCount - 1]);
		SetWindowPos(context->windowHandle, 0, 0, 0, width, height, 0);
	}
	else
	{
		UINT width = context->displayModes[context->currentDisplayMode].Width, height = context->displayModes[context->currentDisplayMode].Height;
		context->swapChain->ResizeTarget(&context->displayModes[context->currentDisplayMode]);
		SetWindowPos(context->windowHandle, 0, 100, 100, width, height, 0);
	}
}

void Draw::MoveWindow(DrawContext *context, UINT dx, UINT dy)
{
	UINT width = context->displayModes[context->currentDisplayMode].Width, height = context->displayModes[context->currentDisplayMode].Height;
	RECT rect;
	GetWindowRect(context->windowHandle, &rect);
	UINT newX = rect.left + dx;
	UINT newY = rect.top + dy;
	SetWindowPos(context->windowHandle, 0, newX, newY, width, height, 0);
}

DrawContext Draw::GetContext(HWND window, UINT screenWidth, UINT screenHeight, bool rightHanded)
{
	DrawContext result;
	result.objectCount = 0;
	result.objectCapacity = 50;
	result.objects = new IUnknown *[result.objectCapacity];
	result.windowHandle = window;

	DXGI_SWAP_CHAIN_DESC swapChainDesc = {};
	swapChainDesc.BufferDesc.Width = screenWidth;
	swapChainDesc.BufferDesc.Height = screenHeight;
	swapChainDesc.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	swapChainDesc.SampleDesc.Count = 1;
	swapChainDesc.SampleDesc.Quality = 0;
	swapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
	swapChainDesc.BufferCount = 2;
	swapChainDesc.OutputWindow = window;
	swapChainDesc.BufferDesc.RefreshRate.Numerator = 60;
	swapChainDesc.BufferDesc.RefreshRate.Denominator = 1;
	swapChainDesc.Windowed = true;
	swapChainDesc.Flags = DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH;
	swapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_SEQUENTIAL;

	D3D_FEATURE_LEVEL featureLevel = D3D_FEATURE_LEVEL_11_0;
	D3D_FEATURE_LEVEL supportedFeatureLevel;

	UINT flags = 0;
#ifdef DEBUG
	flags = D3D11_CREATE_DEVICE_DEBUG; // D3D11.1 -  D3D11_CREATE_DEVICE_DEBUGGABLE;
#endif

	HRESULT hr = D3D11CreateDeviceAndSwapChain(NULL, D3D_DRIVER_TYPE_HARDWARE, NULL, flags, &featureLevel, 1, D3D11_SDK_VERSION,
											   &swapChainDesc, &result.swapChain, &result.device, &supportedFeatureLevel, &result.deviceContext);
	CHECK_WIN_ERROR(hr, "Error calling D3D11CreateDeviceAndSwapChain");

	IDXGIOutput *output;
	hr = result.swapChain->GetContainingOutput(&output);
	UINT numberOfModes;
	hr = output->GetDisplayModeList(DXGI_FORMAT_R8G8B8A8_UNORM, 0, &numberOfModes, NULL);
	auto supportedModes = new DXGI_MODE_DESC[numberOfModes];
	ZeroMemory(supportedModes, sizeof(DXGI_MODE_DESC) * numberOfModes);
	hr = output->GetDisplayModeList(DXGI_FORMAT_R8G8B8A8_UNORM, 0, &numberOfModes, supportedModes);
	output->Release();
	UINT displayWidth = supportedModes[numberOfModes - 1].Width;
	UINT displayHeight = supportedModes[numberOfModes - 1].Height;
	UINT currentWidth = 0;
	UINT currentHeight = 0;
	DXGI_MODE_DESC *currentBestMode = NULL;
	DXGI_MODE_DESC *bestModes = new DXGI_MODE_DESC[numberOfModes];
	UINT bestModeCount = 0;
	for (uint32_t i = 0; i < numberOfModes; ++i)
	{
		DXGI_MODE_DESC *mode = &supportedModes[i];
		if (mode->Width > currentWidth || mode->Height > currentHeight)
		{
			if (currentWidth > 0)
				bestModes[bestModeCount++] = *currentBestMode;
			currentWidth = mode->Width;
			currentHeight = mode->Height;
			currentBestMode = mode;
		}
		else
		{
			double oldRefreshRate = currentBestMode->RefreshRate.Numerator / currentBestMode->RefreshRate.Denominator;
			double refreshRate = mode->RefreshRate.Numerator / mode->RefreshRate.Denominator;
			if (refreshRate > oldRefreshRate)
			{
				currentBestMode = mode;
			}
			else if (currentBestMode->RefreshRate.Numerator == mode->RefreshRate.Numerator &&
					 currentBestMode->RefreshRate.Denominator == mode->RefreshRate.Denominator)
			{
				if (currentBestMode->Scaling == DXGI_MODE_SCALING_CENTERED && mode->Scaling != DXGI_MODE_SCALING_CENTERED)
				{
					currentBestMode = mode;
				}
			}
		}
		if (i == numberOfModes - 1)
		{
			bestModes[bestModeCount++] = *currentBestMode;
		}
	}
	result.displayModeCount = bestModeCount;
	result.displayModes = new DXGI_MODE_DESC[bestModeCount];
	memcpy(result.displayModes, bestModes, sizeof(DXGI_MODE_DESC) * bestModeCount);
	delete[]bestModes, delete[]supportedModes;

	uint32_t bestMode;
	uint32_t minXDiff = 10000, minYDiff = 10000;
	for (uint32_t i = 0; i < result.displayModeCount; ++i)
	{
		uint32_t xDiff = MAX(screenWidth - result.displayModes[i].Width, result.displayModes[i].Width - screenWidth);
		uint32_t yDiff = MAX(screenHeight - result.displayModes[i].Height, result.displayModes[i].Height - screenHeight);
		if (xDiff < minXDiff)
		{
			minXDiff = xDiff;
			minYDiff = yDiff;
			bestMode = i;
		}
		else if (xDiff = minXDiff && yDiff < minYDiff)
		{
			minYDiff = yDiff;
			bestMode = i;
		}
	}
	ResizeWindow(&result, bestMode);

	ResizeSwapChain(&result);

	D3D11_SAMPLER_DESC samplerDesc = {};
	samplerDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
	samplerDesc.AddressU = D3D11_TEXTURE_ADDRESS_WRAP;
	samplerDesc.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;
	samplerDesc.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;
	samplerDesc.ComparisonFunc = D3D11_COMPARISON_NEVER;
	samplerDesc.MinLOD = -D3D11_FLOAT32_MAX;
	samplerDesc.MaxLOD = D3D11_FLOAT32_MAX;
	hr = result.device->CreateSamplerState(&samplerDesc, &result.wrapSampler);
	CHECK_WIN_ERROR(hr, "Error creating Sampler \n");

	samplerDesc = {};
	samplerDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
	samplerDesc.AddressU = D3D11_TEXTURE_ADDRESS_CLAMP;
	samplerDesc.AddressV = D3D11_TEXTURE_ADDRESS_CLAMP;
	samplerDesc.AddressW = D3D11_TEXTURE_ADDRESS_CLAMP;
	samplerDesc.ComparisonFunc = D3D11_COMPARISON_NEVER;
	samplerDesc.MinLOD = -D3D11_FLOAT32_MAX;
	samplerDesc.MaxLOD = D3D11_FLOAT32_MAX;
	hr = result.device->CreateSamplerState(&samplerDesc, &result.clampSampler);
	CHECK_WIN_ERROR(hr, "Error creating Sampler \n");

	D3D11_RASTERIZER_DESC rasterizerDesc = {};
	rasterizerDesc.FillMode = D3D11_FILL_SOLID;
	rasterizerDesc.CullMode = D3D11_CULL_BACK;
	rasterizerDesc.FrontCounterClockwise = rightHanded;
	ID3D11RasterizerState *rasterizerState;
	hr = result.device->CreateRasterizerState(&rasterizerDesc, &rasterizerState);
	CHECK_WIN_ERROR(hr, "Error creating rasterizer state \n");

	result.deviceContext->RSSetState(rasterizerState);
	RELEASE_DX_RESOURCE(rasterizerState);

	D3D11_DEPTH_STENCIL_DESC dsDesc;
	dsDesc.DepthEnable = true;
	dsDesc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ALL;
	dsDesc.DepthFunc = D3D11_COMPARISON_LESS;
	dsDesc.StencilEnable = false;
	hr = result.device->CreateDepthStencilState(&dsDesc, &result.depthTestOnState);
	CHECK_WIN_ERROR(hr, "Error creating DepthStencilState state \n");

	dsDesc.DepthEnable = false;
	hr = result.device->CreateDepthStencilState(&dsDesc, &result.depthTestOffState);
	CHECK_WIN_ERROR(hr, "Error creating DepthStencilState state \n");

	result.deviceContext->OMSetDepthStencilState(result.depthTestOnState, 1);

	D3D11_BLEND_DESC blendState = {};
	blendState.RenderTarget[0].BlendEnable = FALSE;
	blendState.RenderTarget[0].RenderTargetWriteMask = D3D11_COLOR_WRITE_ENABLE_ALL;
	hr = result.device->CreateBlendState(&blendState, &result.blendState3D);
	CHECK_WIN_ERROR(hr, "Error creating 3D blend state");

	blendState = {};
	blendState.RenderTarget[0].BlendEnable = TRUE;
	blendState.RenderTarget[0].SrcBlend = D3D11_BLEND_SRC_ALPHA;
	blendState.RenderTarget[0].DestBlend = D3D11_BLEND_INV_SRC_ALPHA;
	blendState.RenderTarget[0].BlendOp = D3D11_BLEND_OP_ADD;
	blendState.RenderTarget[0].SrcBlendAlpha = D3D11_BLEND_ONE;
	blendState.RenderTarget[0].DestBlendAlpha = D3D11_BLEND_ZERO;
	blendState.RenderTarget[0].BlendOpAlpha = D3D11_BLEND_OP_ADD;
	blendState.RenderTarget[0].RenderTargetWriteMask = D3D11_COLOR_WRITE_ENABLE_ALL;
	blendState.RenderTarget[0].RenderTargetWriteMask = D3D11_COLOR_WRITE_ENABLE_ALL;
	hr = result.device->CreateBlendState(&blendState, &result.blendStateAlpha);
	CHECK_WIN_ERROR(hr, "Error creating alpha blend state");

	float blendFactor[4] = { 0.0f, 0.0f, 0.0f, 0.0f };
	result.deviceContext->OMSetBlendState(result.blendState3D, blendFactor, 0xffffffff);

	AddObject(&result, result.device),
		AddObject(&result, result.deviceContext),
		AddObject(&result, result.swapChain),
		AddObject(&result, result.clampSampler),
		AddObject(&result, result.wrapSampler),
		AddObject(&result, result.blendState3D),
		AddObject(&result, result.blendStateAlpha),
		AddObject(&result, result.depthTestOnState), 
		AddObject(&result, result.depthTestOffState);
	return result;
}

void Draw::SetScreen(DrawContext *context)
{
	Draw::SetScreen(context, &context->screen);
}

void UnbindTarget(DrawContext *context, void *target)
{
	if (!target)
		return;
	for (uint32_t i = 0; i < ARRAYSIZE(context->textures); ++i)
	{
		if (context->textures[i] == target)
		{
			ID3D11ShaderResourceView *null[] = {NULL};
			context->deviceContext->PSSetShaderResources(i, 1, null);
		}
	}
}

void Draw::SetScreen(DrawContext *context, Screen *screen)
{
	UnbindTarget(context, screen->screenDepthMap);
	UnbindTarget(context, screen->screenTargetView);
	ID3D11RenderTargetView *emptyTarget[] = { NULL };
	ID3D11RenderTargetView **renderTarget = screen->screenTargetView ? &screen->screenTargetView : emptyTarget;
	context->deviceContext->OMSetRenderTargets(1, renderTarget, screen->screenDepthMap);
	context->deviceContext->RSSetViewports(1, &screen->viewport);
}

void Draw::SetScreen(DrawContext *context, Screen *screens, uint32_t screenCount)
{
	UnbindTarget(context, screens[0].screenDepthMap);

	D3D11_VIEWPORT viewports[10];
	ID3D11RenderTargetView *renderTargets[10];

	for (uint32_t i = 0; i < screenCount; ++i)
	{
		Screen *currentScreen = &screens[i];
		UnbindTarget(context, currentScreen->screenTargetView);
		viewports[i] = currentScreen->viewport;
		renderTargets[i] = currentScreen->screenTargetView;
	}

	context->deviceContext->OMSetRenderTargets(screenCount, renderTargets, screens[0].screenDepthMap);
	context->deviceContext->RSSetViewports(screenCount, viewports);
}


void Draw::ClearScreen(DrawContext *context, Screen *screen, float r, float g, float b, float a)
{
	if (screen->screenTargetView)
	{
		float blank[4] = {r, g, b, a};
		//float blank[4] = { 0.0f, 0.0f, 0.0f, 0.0f };
		context->deviceContext->ClearRenderTargetView(screen->screenTargetView, blank);
	}

	if (screen->screenDepthMap)
	{
		context->deviceContext->ClearDepthStencilView(screen->screenDepthMap, 
													  D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0);
	}
}

void Draw::ClearScreen(DrawContext *context, float r, float g, float b, float a)
{
	Draw::ClearScreen(context, &context->screen, r, g, b, a);
}

Shaders Draw::GetShaders(DrawContext *context, char *vertexShaderFile, char *pixelShaderFile, Draw::VERTEX_ATTRIBUTES attributes)
{
	File vertexShaderData = ReadFile(vertexShaderFile);

	ID3DBlob *vertexShaderBlob;
	ID3DBlob *errorMsg;
	HRESULT hr = D3DCompile(vertexShaderData.data, vertexShaderData.size, NULL, NULL, NULL, "main", "vs_5_0", 0, D3DCOMPILE_PACK_MATRIX_COLUMN_MAJOR | D3DCOMPILE_DEBUG, &vertexShaderBlob, &errorMsg);
	CHECK_WIN_ERROR(hr, "Error creating vertex shader!");
	if (errorMsg) {
		printf("%s\n", (char *)errorMsg->GetBufferPointer());
		errorMsg->Release();
	}

	ID3D11VertexShader *vertexShader = NULL;
	hr = context->device->CreateVertexShader(vertexShaderBlob->GetBufferPointer(), vertexShaderBlob->GetBufferSize(), NULL, &vertexShader);
	CHECK_WIN_ERROR(hr, "Error creating vertex shader!");

	ID3D11PixelShader *pixelShader = NULL;
	if (pixelShaderFile)
	{
		File pixelShaderData = ReadFile(pixelShaderFile);
		ID3DBlob *pixelShaderBlob;
		hr = D3DCompile(pixelShaderData.data, pixelShaderData.size, NULL, NULL, NULL, "main", "ps_5_0", 0, D3DCOMPILE_PACK_MATRIX_COLUMN_MAJOR | D3DCOMPILE_DEBUG, &pixelShaderBlob, &errorMsg);
		if (errorMsg) {
			printf("%s\n", (char *)errorMsg->GetBufferPointer());
			errorMsg->Release();
		}	
		CHECK_WIN_ERROR(hr, "Error creating pixel shader!");
		hr = context->device->CreatePixelShader(pixelShaderBlob->GetBufferPointer(), pixelShaderBlob->GetBufferSize(), NULL, &pixelShader);
		CHECK_WIN_ERROR_CLEANUP(hr, "Error creating pixel shader!", RELEASE_DX_RESOURCE(pixelShader));
		ReleaseFile(pixelShaderData);
	}
	static D3D11_INPUT_ELEMENT_DESC positionNormalDesc[3] = {};
	positionNormalDesc[0].SemanticName = "POSITION";
	positionNormalDesc[0].AlignedByteOffset = D3D11_APPEND_ALIGNED_ELEMENT;
	positionNormalDesc[0].Format = DXGI_FORMAT_R32G32B32A32_FLOAT;
	positionNormalDesc[1].SemanticName = "NORMAL";
	positionNormalDesc[1].AlignedByteOffset = D3D11_APPEND_ALIGNED_ELEMENT;
	positionNormalDesc[1].Format = DXGI_FORMAT_R32G32B32A32_FLOAT;
	positionNormalDesc[2].SemanticName = "ADDITIONAL";
	positionNormalDesc[2].AlignedByteOffset = D3D11_APPEND_ALIGNED_ELEMENT;
	positionNormalDesc[2].Format = DXGI_FORMAT_R32G32B32A32_FLOAT;

	static D3D11_INPUT_ELEMENT_DESC positionTexcoordDesc[2] = {};
	positionTexcoordDesc[0].SemanticName = "POSITION";
	positionTexcoordDesc[0].AlignedByteOffset = D3D11_APPEND_ALIGNED_ELEMENT;
	positionTexcoordDesc[0].Format = DXGI_FORMAT_R32G32B32A32_FLOAT;
	positionTexcoordDesc[1].SemanticName = "TEXCOORD";
	positionTexcoordDesc[1].AlignedByteOffset = D3D11_APPEND_ALIGNED_ELEMENT;
	positionTexcoordDesc[1].Format = DXGI_FORMAT_R32G32_FLOAT;

	D3D11_INPUT_ELEMENT_DESC *layoutDesc = attributes == Draw::VERTEX_ATTRIBUTES::POSITION_TEXCOORD ? positionTexcoordDesc : positionNormalDesc;
	UINT attributeCount = attributes == Draw::VERTEX_ATTRIBUTES::POSITION ? 1 : (attributes == VERTEX_ATTRIBUTES::POSITION_TEXCOORD ? 2 : 3);

	ID3D11InputLayout *inputLayout = NULL;
	hr = context->device->CreateInputLayout(layoutDesc, attributeCount, vertexShaderBlob->GetBufferPointer(), vertexShaderBlob->GetBufferSize(), &inputLayout);
	CHECK_WIN_ERROR_CLEANUP(hr, "Error creating input layout!", RELEASE_DX_RESOURCE(vertexShader) RELEASE_DX_RESOURCE(pixelShader));
	ReleaseFile(vertexShaderData);

	AddObject(context, vertexShader), AddObject(context, inputLayout);
	if (pixelShaderFile)
		AddObject(context, pixelShader);
	Shaders shaders = {vertexShader, pixelShader, inputLayout};
	return shaders;
}

Mesh Draw::GetMesh(DrawContext *context, void *vertices, uint32_t vertexStride, uint32_t vertexCount, void *indices, uint32_t indexSize, uint32_t indexCount)
{
	ID3D11Buffer *vertexBuffer = NULL, *indexBuffer = NULL;

	D3D11_BUFFER_DESC vertexBufferDesc = {};
	vertexBufferDesc.ByteWidth = vertexStride * vertexCount;
	vertexBufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;

	D3D11_SUBRESOURCE_DATA vertexBufferData = {};
	vertexBufferData.pSysMem = vertices;
	HRESULT hr = context->device->CreateBuffer(&vertexBufferDesc, &vertexBufferData, &vertexBuffer);
	CHECK_WIN_ERROR(hr, "Error creating vertex buffer!");

	D3D11_BUFFER_DESC indexBufferDesc = {};
	indexBufferDesc.ByteWidth = indexCount * indexSize;
	indexBufferDesc.BindFlags = D3D11_BIND_INDEX_BUFFER;

	D3D11_SUBRESOURCE_DATA indexBufferData = {};
	indexBufferData.pSysMem = indices;
	context->device->CreateBuffer(&indexBufferDesc, &indexBufferData, &indexBuffer);
	CHECK_WIN_ERROR_CLEANUP(hr, "Error creating vertex buffer!", RELEASE_DX_RESOURCE(vertexBuffer));

	Mesh result = {};
	result.vertexStride = vertexStride;
	result.indexCount = indexCount;
	result.vertexBuffer = vertexBuffer;
	result.indexBuffer = indexBuffer;
	result.indexFormat = indexSize > sizeof(uint16_t) ? DXGI_FORMAT_R32_UINT : DXGI_FORMAT_R16_UINT;

	AddObject(context, vertexBuffer), AddObject(context, indexBuffer);

	return result;
}

enum ViewType
{
	RENDER_TARGET = 0,
	DEPTH_STENCIL = 1,
	SHADER_RESOURCE = 2,
	RENDER_TARGET_AND_SRESOURCE = 3,
	DEPTH_STENCIL_AND_SRESOURCE = 4,
};

static UINT BindFlagFromType[] = {
D3D11_BIND_RENDER_TARGET, 
D3D11_BIND_DEPTH_STENCIL, 
D3D11_BIND_SHADER_RESOURCE,
D3D11_BIND_RENDER_TARGET | D3D11_BIND_SHADER_RESOURCE,
D3D11_BIND_DEPTH_STENCIL | D3D11_BIND_SHADER_RESOURCE
};

ID3D11View *GetView(DrawContext *context, UINT screenWidth, UINT screenHeight, bool multiSample, ViewType type,
					DXGI_FORMAT format, ID3D11View *sharedResource)
{
	ID3D11Texture2D *tex = NULL;
	HRESULT hr;
	if (sharedResource == NULL)
	{
		D3D11_TEXTURE2D_DESC texDesc = {};
		texDesc.Width = screenWidth;
		texDesc.Height = screenHeight;
		texDesc.MipLevels = 1;
		texDesc.ArraySize = 1;
		texDesc.Format = format;
		texDesc.SampleDesc.Count = multiSample ? 4 : 1;
		texDesc.SampleDesc.Quality = multiSample ? D3D11_STANDARD_MULTISAMPLE_PATTERN : 0;
		texDesc.Usage = D3D11_USAGE_DEFAULT;
		texDesc.BindFlags = BindFlagFromType[type];

		hr = context->device->CreateTexture2D(&texDesc, NULL, &tex);
		CHECK_WIN_ERROR(hr, "Error creating texture\n");
	} 
	else
	{
		ID3D11Resource *res;
		sharedResource->GetResource(&res);
		res->QueryInterface(IID_ID3D11Texture2D, (void **)&tex);
		RELEASE_DX_RESOURCE(res);
	}

	ID3D11View *result = NULL;
	switch (type)
	{
		case RENDER_TARGET:
		case RENDER_TARGET_AND_SRESOURCE:
		{
			ID3D11RenderTargetView *renderTarget;
			CD3D11_RENDER_TARGET_VIEW_DESC renderTargetViewDesc(multiSample ? 
																D3D11_RTV_DIMENSION_TEXTURE2DMS : 
																D3D11_RTV_DIMENSION_TEXTURE2D, 
																format);
			hr = context->device->CreateRenderTargetView(tex, &renderTargetViewDesc, &renderTarget);
			CHECK_WIN_ERROR(hr, "Error creating rendertarget view\n");
			renderTarget->QueryInterface(IID_ID3D11View, (void **)&result);
			RELEASE_DX_RESOURCE(renderTarget);
		} break;
		case DEPTH_STENCIL:
		case DEPTH_STENCIL_AND_SRESOURCE:
		{
			ID3D11DepthStencilView *depthStencil;
			CD3D11_DEPTH_STENCIL_VIEW_DESC depthStencilViewDesc(multiSample ?
																D3D11_DSV_DIMENSION_TEXTURE2DMS :
																D3D11_DSV_DIMENSION_TEXTURE2D,
																format == DXGI_FORMAT_R24G8_TYPELESS ?
																DXGI_FORMAT_D24_UNORM_S8_UINT : format);
			hr = context->device->CreateDepthStencilView(tex, &depthStencilViewDesc, &depthStencil);
			CHECK_WIN_ERROR(hr, "Error creating depth stencil view\n");
			depthStencil->QueryInterface(IID_ID3D11View, (void **)&result);
			RELEASE_DX_RESOURCE(depthStencil);
		} break;
		case SHADER_RESOURCE:
		{
			ID3D11ShaderResourceView *shaderResource;
			CD3D11_SHADER_RESOURCE_VIEW_DESC shaderResourceViewDesc(multiSample ?
																D3D11_SRV_DIMENSION_TEXTURE2DMS :
																D3D11_SRV_DIMENSION_TEXTURE2D,
																format);
			hr = context->device->CreateShaderResourceView(tex, &shaderResourceViewDesc, &shaderResource);
			CHECK_WIN_ERROR(hr, "Error creating shader resource view\n");
			shaderResource->QueryInterface(IID_ID3D11View, (void **)&result);
			RELEASE_DX_RESOURCE(shaderResource);
		} break;
	}
	RELEASE_DX_RESOURCE(tex);
	return result;
}

Texture2D Draw::GetTexture2D(DrawContext *context, UINT width, UINT height, void *data)
{
	D3D11_TEXTURE2D_DESC texDesc = {};
	texDesc.Width = width;
	texDesc.Height = height;
	texDesc.MipLevels = 1;
	texDesc.ArraySize = 1;
	texDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	texDesc.SampleDesc.Count = 1;
	texDesc.SampleDesc.Quality = 0;
	texDesc.Usage = D3D11_USAGE_DEFAULT;
	texDesc.BindFlags = D3D11_BIND_SHADER_RESOURCE;

	D3D11_SUBRESOURCE_DATA texData = {};
	texData.pSysMem = data;
	texData.SysMemPitch = width * 4;

	ID3D11Texture2D *tex;
	HRESULT hr = context->device->CreateTexture2D(&texDesc, &texData, &tex);
	CHECK_WIN_ERROR(hr, "Error creating texture2D\n");

	D3D11_SHADER_RESOURCE_VIEW_DESC viewDesc = {};
	viewDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	viewDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
	viewDesc.Texture2D.MostDetailedMip = 0;
	viewDesc.Texture2D.MipLevels = -1;

	Texture2D result;
	hr = context->device->CreateShaderResourceView(tex, &viewDesc, &result.texture);
	CHECK_WIN_ERROR(hr, "Error creating texture view\n");
	tex->Release();

	result.width = width;
	result.height = height;

	AddObject(context, result.texture);

	return result;
}

Texture3D Draw::GetTexture3D(DrawContext *context, UINT width, UINT height, UINT depth, void *data)
{
	D3D11_TEXTURE3D_DESC texDesc = {};
	texDesc.Width = width;
	texDesc.Height = height;
	texDesc.Depth = depth;
	texDesc.MipLevels = 1;
	texDesc.Format = DXGI_FORMAT_R32_FLOAT;
	texDesc.Usage = D3D11_USAGE_DEFAULT;
	texDesc.BindFlags = D3D11_BIND_SHADER_RESOURCE;

	D3D11_SUBRESOURCE_DATA texData = {};
	texData.pSysMem = data;
	texData.SysMemPitch = width * 4;
	texData.SysMemSlicePitch = width * height * 4;

	ID3D11Texture3D *tex;
	HRESULT hr = context->device->CreateTexture3D(&texDesc, &texData, &tex);
	CHECK_WIN_ERROR(hr, "Error creating texture3D\n");

	D3D11_SHADER_RESOURCE_VIEW_DESC viewDesc = {};
	viewDesc.Format = DXGI_FORMAT_R32_FLOAT;
	viewDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE3D;
	viewDesc.Texture2D.MostDetailedMip = 0;
	viewDesc.Texture2D.MipLevels = -1;

	Texture3D result;
	hr = context->device->CreateShaderResourceView(tex, &viewDesc, &result.texture);
	CHECK_WIN_ERROR(hr, "Error creating texture view\n");
	tex->Release();

	result.width = width;
	result.height = height;
	result.depth = depth;

	AddObject(context, result.texture);

	return result;
}


Screen Draw::GetScreen(DrawContext *context, UINT screenWidth, UINT screenHeight, bool multisample,
					   BufferTypes bufferTypes)
{
	Screen result;
	result.screenTargetView = NULL;
	result.screenTargetViewResource = NULL;
	result.screenDepthMap = NULL;
	result.screenDepthMapResource = NULL;
	result.multisample = multisample;
	result.viewport = { 0 };
	result.viewport.Width = (float)screenWidth;
	result.viewport.Height = (float)screenHeight;
	result.viewport.MinDepth = 0.0f;
	result.viewport.MaxDepth = 1.0f;
	
	if (bufferTypes & BufferTypes::COLOR_BUFFER)
	{
		ID3D11View *temp = GetView(context, screenWidth, screenHeight, multisample,
								   multisample ? RENDER_TARGET : RENDER_TARGET_AND_SRESOURCE, DXGI_FORMAT_R8G8B8A8_UNORM, NULL);
		temp->QueryInterface(IID_ID3D11RenderTargetView, (void **)&result.screenTargetView);
		RELEASE_DX_RESOURCE(temp);
		AddObject(context, result.screenTargetView);
	}

	if (bufferTypes & BufferTypes::DEPTH_BUFFER)
	{
		ID3D11View *temp = GetView(context, screenWidth, screenHeight, multisample, 
								   multisample ? DEPTH_STENCIL : DEPTH_STENCIL_AND_SRESOURCE,
								   multisample ? DXGI_FORMAT_D24_UNORM_S8_UINT : DXGI_FORMAT_R24G8_TYPELESS, NULL);
		temp->QueryInterface(IID_ID3D11DepthStencilView, (void **)&result.screenDepthMap);
		RELEASE_DX_RESOURCE(temp);
		AddObject(context, result.screenDepthMap);
	}

	if (multisample && bufferTypes & BufferTypes::COLOR_BUFFER)
	{
		ID3D11View *temp = GetView(context, screenWidth, screenHeight, false, SHADER_RESOURCE,
					   DXGI_FORMAT_R8G8B8A8_UNORM, NULL);
		temp->QueryInterface(IID_ID3D11ShaderResourceView, (void **)&result.screenTargetViewResource);
		RELEASE_DX_RESOURCE(temp);
		AddObject(context, result.screenTargetViewResource);
	}
	else
	{
		if (bufferTypes & BufferTypes::COLOR_BUFFER)
		{
			ID3D11View *temp = GetView(context, screenWidth, screenHeight, false, SHADER_RESOURCE,
						   DXGI_FORMAT_R8G8B8A8_UNORM, result.screenTargetView);
			temp->QueryInterface(IID_ID3D11ShaderResourceView, (void **)&result.screenTargetViewResource);
			RELEASE_DX_RESOURCE(temp);
			AddObject(context, result.screenTargetViewResource);
		}

		if (bufferTypes & BufferTypes::DEPTH_BUFFER)
		{
			ID3D11View *temp = GetView(context, screenWidth, screenHeight, false, SHADER_RESOURCE,
						   DXGI_FORMAT_R24_UNORM_X8_TYPELESS, result.screenDepthMap);
			temp->QueryInterface(IID_ID3D11ShaderResourceView, (void **)&result.screenDepthMapResource);
			RELEASE_DX_RESOURCE(temp);
			AddObject(context, result.screenDepthMapResource);
		}
	}

	return result;
}

void Draw::SetTexture(DrawContext *context, uint32_t slot, Texture3D *texture)
{
	context->deviceContext->PSSetShaderResources(slot, 1, &texture->texture);
	context->textures[slot + 1] = texture->texture;
}

void Draw::SetTexture(DrawContext *context, uint32_t slot, Texture2D *texture)
{
	context->deviceContext->PSSetShaderResources(slot, 1, &texture->texture);
	context->textures[slot + 1] = texture->texture;
}

void Draw::SetTexture(DrawContext *context, uint32_t slot, Screen *screen)
{
	if (screen->multisample && screen->screenTargetViewResource)
	{
		ID3D11Resource *multisampleTexture;
		screen->screenTargetView->GetResource(&multisampleTexture);
		ID3D11Resource *texture;
		screen->screenTargetViewResource->GetResource(&texture);
		context->deviceContext->ResolveSubresource(texture, 0, multisampleTexture, 0, DXGI_FORMAT_R8G8B8A8_UNORM);
		RELEASE_DX_RESOURCE(multisampleTexture);
		RELEASE_DX_RESOURCE(texture);

		context->deviceContext->PSSetShaderResources(slot, 1, &screen->screenTargetViewResource);
	}
	else
	{
		if (screen->screenTargetViewResource)
		{
			context->deviceContext->PSSetShaderResources(slot, 1, &screen->screenTargetViewResource);
		}
		if (screen->screenDepthMapResource)
		{
			uint32_t depthMapSlot = screen->screenTargetViewResource ? slot + 1 : slot;
			context->deviceContext->PSSetShaderResources(depthMapSlot, 1, &screen->screenDepthMapResource);
			context->textures[depthMapSlot] = screen->screenDepthMap;
		}
	}
	if (screen->screenTargetView)
	{
		context->textures[slot] = screen->screenTargetView;
	}
	ID3D11SamplerState *samplers[2] = { context->wrapSampler, context->clampSampler };
	context->deviceContext->PSSetSamplers(0, 2, samplers);
}

void Draw::Draw(DrawContext *context, Mesh *mesh)
{
	context->deviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	UINT strides[] = { mesh->vertexStride }, offsets[] = {0};
	context->deviceContext->IASetVertexBuffers(0, 1, &mesh->vertexBuffer, strides, offsets);
	context->deviceContext->IASetIndexBuffer(mesh->indexBuffer, mesh->indexFormat, 0);
	context->deviceContext->DrawIndexed(mesh->indexCount, 0, 0);
}

void Draw::DrawMulti(DrawContext *context, Mesh *mesh, uint32_t count)
{
	context->deviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	UINT strides[] = { mesh->vertexStride }, offsets[] = { 0 };
	context->deviceContext->IASetVertexBuffers(0, 1, &mesh->vertexBuffer, strides, offsets);
	context->deviceContext->IASetIndexBuffer(mesh->indexBuffer, mesh->indexFormat, 0);
	context->deviceContext->DrawIndexedInstanced(mesh->indexCount, count, 0, 0, 0);
}

void Draw::SetShaders(DrawContext *context, Shaders *shaders)
{
	context->deviceContext->IASetInputLayout(shaders->inputLayout);
	context->deviceContext->VSSetShader(shaders->vertexShader, NULL, 0);
	context->deviceContext->PSSetShader(shaders->pixelShader, NULL, 0);
}

void Draw::SetConstantData(DrawContext *context, uint32_t slot, void *data, uint32_t dataSize)
{
	ID3D11Buffer *constantBuffer = context->constBuffers[slot];
	if (context->bufferSizes[slot] != dataSize)
	{
		if (context->bufferSizes[slot] > 0)
		{
			RELEASE_DX_RESOURCE(context->constBuffers[slot]);
		}
		context->bufferSizes[slot] = dataSize;

		dataSize = ((dataSize - 1)/ 16 + 1) * 16;
		D3D11_BUFFER_DESC constBufferDesc = {};
		constBufferDesc.ByteWidth = dataSize;
		constBufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;

		D3D11_SUBRESOURCE_DATA constBufferData = {};
		constBufferData.pSysMem = data;

		HRESULT hr = context->device->CreateBuffer(&constBufferDesc, &constBufferData, &constantBuffer);
		CHECK_WIN_ERROR(hr, "Error creating constant buffer!\n");
		context->constBuffers[slot] = constantBuffer;
	}

	context->deviceContext->UpdateSubresource(constantBuffer, 0, NULL, data, 0, 0);
	context->deviceContext->VSSetConstantBuffers(slot, 1, &constantBuffer);
	context->deviceContext->PSSetConstantBuffers(slot, 1, &constantBuffer);
}

void Draw::SetAlphaBlend(DrawContext *context, bool alphaBlend)\
{
	ID3D11BlendState *blendState = alphaBlend ? context->blendStateAlpha : context->blendState3D;
	float blendFactor[4] = { 0.0f, 0.0f, 0.0f, 0.0f };
	context->deviceContext->OMSetBlendState(blendState, blendFactor, 0xFFFFFFFF);
}

void Draw::SetDepthTest(DrawContext *context, bool depthTest)
{
	ID3D11DepthStencilState *depthStencilState = depthTest ? context->depthTestOnState : context->depthTestOffState;
	context->deviceContext->OMSetDepthStencilState(depthStencilState, 0);
}

void Draw::Swap(DrawContext *context)
{
	HRESULT hr = context->swapChain->Present(1, 0);
}

void Draw::Release(DrawContext *context)
{
	for (uint32_t i = 0; i < context->objectCount; ++i)
	{
		RELEASE_DX_RESOURCE(context->objects[i]);
	}

	delete []context->objects;
	delete []context->displayModes;

	for (uint32_t i = 0; i < ARRAYSIZE(context->constBuffers); ++i)
	{
		if (context->bufferSizes[i] > 0)
		{
			RELEASE_DX_RESOURCE(context->constBuffers[i]);
		}
	}
}

void Draw::Release(DrawContext *context, Screen *screen)
{
	if (screen->screenTargetView)
	{
		RemoveObject(context, screen->screenTargetView);
		RemoveObject(context, screen->screenTargetViewResource);
	}
	if (screen->screenDepthMap)
	{
		RemoveObject(context, screen->screenDepthMap);
		RemoveObject(context, screen->screenDepthMapResource);\
	}
}

void Draw::Release(DrawContext *context, Mesh *mesh)
{
	RemoveObject(context, mesh->vertexBuffer);
	RemoveObject(context, mesh->indexBuffer);
}

#endif