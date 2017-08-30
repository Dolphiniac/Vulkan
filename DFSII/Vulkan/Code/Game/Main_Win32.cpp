#include "Game/TheGame.hpp"
#include "Engine/Core/EngineSystemManager.hpp"

#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <cassert>
#include <crtdbg.h>
#include "Engine/Core/Time.hpp"
#include "Engine/Renderer/Mesh.hpp"
#include "Engine/Renderer/UniformBlock.hpp"
#include "Engine/Core/Memory.hpp"
#include "Engine/Renderer/Texture.hpp"
#include "Engine/Renderer/BitmapFont.hpp"
#include "Engine/Input/TheInput.hpp"
#include "Engine/Core/Audio.hpp"
#include "Engine/Core/Profiler.hpp"
#include "Engine/Core/JobSystem.hpp"
#include "Engine/Renderer/Sprite.hpp"
#include "Engine/Renderer/ShaderStorageBlock.hpp"
#include "Engine/Network/NetworkSystem.hpp"
#include "Engine/Core/EventSystem.hpp"
#include "Quantum/Core/EventSystem.h"
#include "Quantum/FileSystem/FileUtils.h"
#include "Quantum/FileSystem/Path.h"
#include "Engine/Core/FileUtils.hpp"
#include "Engine/Renderer/VulkanUtils.hpp"

#include "Quantum/Hephaestus/PipelineGenerator.h"
#include "Quantum/Hephaestus/Manager.h"
#include "Quantum/Hephaestus/CommandBuffer.h"
#include "Quantum/Hephaestus/PhysicalDevice.h"
#include "Quantum/Hephaestus/Swapchain.h"
#include "Quantum/Hephaestus/BufferDescriptor.h"
#include "Quantum/Hephaestus/RenderPass.h"
#include "Quantum/Hephaestus/Material.h"
#include "Quantum/Hephaestus/Mesh.h"
#include "Quantum/Hephaestus/MeshRenderer.h"
#include "Quantum/Hephaestus/DescriptorSetLayoutGenerator.h"
#include "Quantum/Hephaestus/Texture.h"
#include "Engine/Model/MeshBuilder.hpp"
#include "Quantum/Hephaestus/Spirv.h"
#include "Quantum/Hephaestus/MaterialState.h"

#define STBI_HEADER_FILE_ONLY
#include "ThirdParty/stb/stb_image.c"

#define VK_USE_PLATFORM_WIN32_KHR
#include <vulkan.h>
#include "Quantum/Hephaestus/VertexType.h"
#include "Quantum/Hephaestus/Pipeline.h"
#pragma comment (lib, "C:/VulkanSDK/1.0.30.0/Bin32/vulkan-1")



//-----------------------------------------------------------------------------------------------
const int OFFSET_FROM_WINDOWS_DESKTOP = 50;
const int WINDOW_PHYSICAL_WIDTH = 1600;
const int WINDOW_PHYSICAL_HEIGHT = 900;
const double VIEW_LEFT = 0.0;
const double VIEW_RIGHT = 1600.0;
const double VIEW_BOTTOM = 0.0;
const double VIEW_TOP = VIEW_RIGHT * static_cast<double>(WINDOW_PHYSICAL_HEIGHT) / static_cast<double>(WINDOW_PHYSICAL_WIDTH);

HWND g_hWnd = nullptr;
HDC g_displayDeviceContext = nullptr;
HGLRC g_openGLRenderingContext = nullptr;
const char* APP_NAME = "Graphics";


//-----------------------------------------------------------------------------------------------
LRESULT CALLBACK WindowsMessageHandlingProcedure(HWND windowHandle, UINT wmMessageCode, WPARAM wParam, LPARAM lParam)
{
	if (!The.Input)
	{
		return 0;
	}
	unsigned char asKey = (unsigned char)wParam;
	switch (wmMessageCode)
	{
	case WM_CLOSE:
	case WM_DESTROY:
	case WM_QUIT:
		return 0;
	case WM_CHAR:
		//g_theGame->ConcatToWorkingString(asKey);
		break;

	case WM_KEYDOWN:
		The.Input->SetKeyStatus(asKey, true);
		break;
	case WM_KEYUP:
		The.Input->SetKeyStatus(asKey, false);
		break;
	case WM_SETFOCUS:
		The.Input->SetInFocus(true);
		break;
	case WM_KILLFOCUS:
		The.Input->SetInFocus(false);
		break;
	case WM_LBUTTONDOWN:
		The.Input->SetKeyStatus(M_LEFT, true);
		break;
	case WM_LBUTTONUP:
		The.Input->SetKeyStatus(M_LEFT, false);
		break;
	case WM_RBUTTONDOWN:
		The.Input->SetKeyStatus(M_RIGHT, true);
		break;
	case WM_RBUTTONUP:
		The.Input->SetKeyStatus(M_RIGHT, false);
		break;
	case WM_MOUSEWHEEL:
		short deltaWheel = GET_WHEEL_DELTA_WPARAM(wParam);
		if (deltaWheel > 0)
		{
			The.Input->SetKeyStatus(M_WHEEL_UP, true);
		}
		else if (deltaWheel < 0)
		{
			The.Input->SetKeyStatus(M_WHEEL_DOWN, true);
		}
		// 	case WM_INPUT:
		// 		HRAWINPUT rawH = (HRAWINPUT)lParam;
		// 		UINT dwSize;
		// 		GetRawInputData(rawH, RID_INPUT, NULL, &dwSize, sizeof(RAWINPUTHEADER));
		// 		LPBYTE lpb = new BYTE[dwSize];
		// 
		// 		GetRawInputData(rawH, RID_INPUT, lpb, &dwSize, sizeof(RAWINPUTHEADER));
		// 		RAWINPUT* ri = (RAWINPUT*)lpb;
		// 
		// 		if (ri->header.dwType == RIM_TYPEMOUSE)
		// 		{
		// 			RAWMOUSE rm = ri->data.mouse;
		// 			if (RI_MOUSE_LEFT_BUTTON_DOWN & rm.usButtonFlags)
		// 				The.Input->SetKeyStatus(M_LEFT, true);
		// 
		// 			if (RI_MOUSE_LEFT_BUTTON_UP & rm.usButtonFlags)
		// 				The.Input->SetKeyStatus(M_LEFT, false);
		// 
		// 			if (RI_MOUSE_RIGHT_BUTTON_DOWN & rm.usButtonFlags)
		// 				The.Input->SetKeyStatus(M_RIGHT, true);
		// 
		// 			if (RI_MOUSE_RIGHT_BUTTON_UP & rm.usButtonFlags)
		// 				The.Input->SetKeyStatus(M_RIGHT, false);
		// 
		// 			if (RI_MOUSE_WHEEL & rm.usButtonFlags)
		// 			{
		// 				short mouseDelta = rm.usButtonData;
		// 				if (mouseDelta > 0)
		// 				{
		// 					The.Input->SetKeyStatus(M_WHEEL_UP, true);
		// 				}
		// 				else if (mouseDelta < 0)
		// 				{
		// 					The.Input->SetKeyStatus(M_WHEEL_DOWN, true);
		// 				}
		// 				else
		// 				{
		// 					The.Input->SetKeyStatus(M_WHEEL_DOWN, false);
		// 					The.Input->SetKeyStatus(M_WHEEL_UP, false);
		// 				}
		// 			}
		// 		}
	}

	return DefWindowProc(windowHandle, wmMessageCode, wParam, lParam);
}

//-----------------------------------------------------------------------------------------------
void RunMessagePump()
{
	The.Input->SetKeyStatus(M_WHEEL_UP, false);
	The.Input->SetKeyStatus(M_WHEEL_DOWN, false);
	MSG queuedMessage;
	for (;;)
	{
		const BOOL wasMessagePresent = PeekMessage(&queuedMessage, NULL, 0, 0, PM_REMOVE);
		if (!wasMessagePresent)
		{
			break;
		}

		TranslateMessage(&queuedMessage);
		DispatchMessage(&queuedMessage);
	}
}
#include "Quantum/Hephaestus/Shader.h"
#include "Quantum/Hephaestus/LogicalDevice.h"
#include "Quantum/Hephaestus/Declarations.h"


//-----------------------------------------------------------------------------------------------
void UpdateView(Camera3D &cam)
{
	The.Input->Tick();
	RunMessagePump();
	static double lastFrame = GetCurrentTimeSeconds();
	double thisFrame = GetCurrentTimeSeconds();
	float deltaSeconds = thisFrame - lastFrame;
	lastFrame = thisFrame;
	cam.m_pitchDegreesAboutX = Clampf(cam.m_pitchDegreesAboutX + The.Input->GetDeltaMousePosf().y, -89.9f, 89.9f);
	cam.m_yawDegreesAboutY -= The.Input->GetDeltaMousePosf().x;
	while (cam.m_yawDegreesAboutY >= 360.f)
	{
		cam.m_yawDegreesAboutY -= 360.f;
	}
	while (cam.m_yawDegreesAboutY < 0.f)
	{
		cam.m_yawDegreesAboutY += 360.f;
	}
	cam.m_position += The.Input->GetHorizontalAxisBinary() * deltaSeconds * cam.MOVEMENT_SPEED * cam.GetRightXZ() * (The.Input->GetKey(KB_SHIFT) ? 5.f : 1.f);
	cam.m_position += The.Input->GetVerticalAxisBinary() * deltaSeconds * cam.MOVEMENT_SPEED * cam.GetForwardXZ() * (The.Input->GetKey(KB_SHIFT) ? 5.f : 1.f);
	if (The.Input->GetKey('Z') || The.Input->GetKey('X') || The.Input->GetKey('C'))
	{
		cam.m_position.y -= deltaSeconds * cam.MOVEMENT_SPEED  * (The.Input->GetKey(KB_SHIFT) ? 5.f : 1.f);
	}
	if (The.Input->GetKey(KB_SPACE))
	{
		cam.m_position.y += deltaSeconds * cam.MOVEMENT_SPEED  * (The.Input->GetKey(KB_SHIFT) ? 5.f : 1.f);
	}
	GlobalMatrices.View.MakeTransformationMatrix(1.f, Vector3(cam.m_pitchDegreesAboutX, cam.m_yawDegreesAboutY, 0.f), cam.m_position);
	GlobalMatrices.InvView = GlobalMatrices.View;
	GlobalMatrices.View.Invert();
	GlobalMatrices.Push();
}


#include "Quantum/Hephaestus/Compiler.h"
#include "Engine/Model/Fbx.hpp"
//-----------------------------------------------------------------------------------------------
void CreateOpenGLWindow(HINSTANCE applicationInstanceHandle)
{
	// Define a window class
	WNDCLASSEX windowClassDescription;
	memset(&windowClassDescription, 0, sizeof(windowClassDescription));
	windowClassDescription.cbSize = sizeof(windowClassDescription);
	windowClassDescription.style = CS_OWNDC; // Redraw on move, request own Display Context
	windowClassDescription.lpfnWndProc = static_cast<WNDPROC>(WindowsMessageHandlingProcedure); // Assign a win32 message-handling function
	windowClassDescription.hInstance = GetModuleHandle(NULL);
	windowClassDescription.hIcon = NULL;
	windowClassDescription.hCursor = LoadCursor(NULL, IDC_ARROW);
	windowClassDescription.lpszClassName = TEXT("Simple Window Class");
	RegisterClassEx(&windowClassDescription);

	const DWORD windowStyleFlags = WS_CAPTION | WS_BORDER | WS_THICKFRAME | WS_SYSMENU | WS_OVERLAPPED;
	const DWORD windowStyleExFlags = WS_EX_APPWINDOW;

	RECT desktopRect;
	HWND desktopWindowHandle = GetDesktopWindow();
	GetClientRect(desktopWindowHandle, &desktopRect);

	RECT windowRect = { OFFSET_FROM_WINDOWS_DESKTOP, OFFSET_FROM_WINDOWS_DESKTOP, OFFSET_FROM_WINDOWS_DESKTOP + WINDOW_PHYSICAL_WIDTH, OFFSET_FROM_WINDOWS_DESKTOP + WINDOW_PHYSICAL_HEIGHT };
	AdjustWindowRectEx(&windowRect, windowStyleFlags, FALSE, windowStyleExFlags);

	WCHAR windowTitle[1024];
	MultiByteToWideChar(GetACP(), 0, APP_NAME, -1, windowTitle, sizeof(windowTitle) / sizeof(windowTitle[0]));
	g_hWnd = CreateWindowEx(
		windowStyleExFlags,
		windowClassDescription.lpszClassName,
		windowTitle,
		windowStyleFlags,
		windowRect.left,
		windowRect.top,
		windowRect.right - windowRect.left,
		windowRect.bottom - windowRect.top,
		NULL,
		NULL,
		applicationInstanceHandle,
		NULL);

	ShowWindow(g_hWnd, SW_SHOW);
	SetForegroundWindow(g_hWnd);
	SetFocus(g_hWnd);

	g_displayDeviceContext = GetDC(g_hWnd);

	HCURSOR cursor = LoadCursor(NULL, IDC_ARROW);
	SetCursor(cursor);

	PIXELFORMATDESCRIPTOR pixelFormatDescriptor;
	memset(&pixelFormatDescriptor, 0, sizeof(pixelFormatDescriptor));
	pixelFormatDescriptor.nSize = sizeof(pixelFormatDescriptor);
	pixelFormatDescriptor.nVersion = 1;
	pixelFormatDescriptor.dwFlags = PFD_DRAW_TO_WINDOW | PFD_SUPPORT_OPENGL | PFD_DOUBLEBUFFER;
	pixelFormatDescriptor.iPixelType = PFD_TYPE_RGBA;
	pixelFormatDescriptor.cColorBits = 24;
	pixelFormatDescriptor.cDepthBits = 24;
	pixelFormatDescriptor.cAccumBits = 0;
	pixelFormatDescriptor.cStencilBits = 8;

	HManager::CreateInstance();
	HManager::RetrieveAndSetGPU();
	HQueue* queue;
	HManager::CreateLogicalDeviceSimple(&queue);
	HManager::InitializeWin32Surface(applicationInstanceHandle, g_hWnd);
	HManager::CreateSwapchain(WINDOW_PHYSICAL_WIDTH, WINDOW_PHYSICAL_HEIGHT);
	HCommandBuffer* commandBuffer = queue->RetrieveCommandBuffer(false, false);
	HRenderPass* renderPass = HRenderPass::Create(queue, "SSAO");
	HRenderPass* lightmap = HRenderPass::Create(queue, "LightmapRender");

	HManager::SetRenderPass(renderPass);
	HManager::LinkRenderPassesBySemaphore(lightmap->GetHandle(), renderPass->GetHandle(), H_PIPELINE_STAGE_DEPTH_STENCIL_BIT);
	HPhysicalDevice* gpu = HManager::GetPhysicalDevice();
	Time.time = 0.f;
	Time.Push();

	GlobalMatrices.Projection.MakePerspective(90.f, 16.f / 9.f, .05f, 1000.f);
	GlobalMatrices.InvProjection = GlobalMatrices.Projection.Inverse();
	GlobalMatrices.View.MakeTransformationMatrix(0.f, Vector3(0.f, 0.f, 0.f), Vector3(0.f, 0.f, 0.f));
	GlobalMatrices.Push();
	SceneImport* si = FbxLoadSceneFromFile("sibenik", "Data/Models/sibenik.fbx", Matrix44::Identity, false);
	
	std::vector<HMesh> meshes;
	std::vector<HMaterial> materials;
	std::vector<HMeshRenderer> renderers;
	meshes.reserve(si->m_meshes.size());
	materials.reserve(si->m_meshes.size());
	renderers.reserve(si->m_meshes.size());
	Camera3D lightCam;
	lightCam.m_position = Vector3(-38.478f, 14.2095f, -.1087f);
	lightCam.m_pitchDegreesAboutX = 29.099f;
	lightCam.m_yawDegreesAboutY = 270.f;
	Matrix44 lightView;
	lightView.MakeTransformationMatrix(1.f, Vector3(lightCam.m_pitchDegreesAboutX, lightCam.m_yawDegreesAboutY, 0.f), lightCam.m_position);
	lightView.InvertOrthonormal();
	for (uint32 meshIndex = 0; meshIndex < si->m_meshes.size(); meshIndex++)
	{
		MeshBuilder* mb = si->m_meshes[meshIndex];
		void* data;
		uint32 numVerts;
		uint32 dataSize;
		//mb->GenerateIndexData();
		//mb->CalculateNormalsFromFaces();
		//mb->GenerateTangentSpace(true);
		mb->CopyInterleavedMeshData(H_VERTEX_TYPE_PCTNT, &data, &numVerts, &dataSize);
		HMesh mesh;
		mesh.SetVertexData(data, dataSize, numVerts);
		//mb->CopyIndexData(&indexData, &numIndices);
		//mesh.SetIndexData(indexData, numIndices, H_INDEX_TYPE_UINT16);
		meshes.push_back(std::move(mesh));
		HMaterial mat = HMaterial::FromAssociation("Sibenik", mb->m_materialName.c_str());
		mat.BindUniformBufferData("ObjectLocal", &Matrix44::Identity, sizeof(Matrix44));
		mat.BindUniformBufferData("LightmapInfo", &lightView, sizeof(Matrix44), "LightmapRender.Render");
		materials.push_back(std::move(mat));
	
		HMeshRenderer rend = HMeshRenderer(&meshes[meshIndex], &materials[meshIndex]);
		renderers.push_back(std::move(rend));
	}
	
	Camera3D cam;
	The.Input = new TheInput();
	The.Input->SetDefaultMousePos(IntVector2(500, 500));
	The.Input->SetAutomaticMouseReset(true);
	HMesh skybox = HMesh::GetSkyboxMesh();
	HMaterial stormySkies("StormySkybox");
	HMeshRenderer skyboxRenderer = HMeshRenderer(&skybox, &stormySkies);
	HMesh fullscreenMesh;
	HVertexPCT verts[4];
	verts[0].position = float3(-1.f, -1.f, 0.f);
	verts[1].position = float3(1.f, -1.f, 0.f);
	verts[2].position = float3(1.f, 1.f, 0.f);
	verts[3].position = float3(-1.f, 1.f, 0.f);

	uint16 indices[6] =
	{
		0, 1, 2,
		2, 3, 0
	};
	fullscreenMesh.SetVertexData(verts, sizeof(verts), ARRAY_LENGTH(verts));
	fullscreenMesh.SetIndexData(indices, ARRAY_LENGTH(indices), H_INDEX_TYPE_UINT16);
	
	HMaterial fullscreenMat = HMaterial("DeferredComposite");
	HMeshRenderer fullscreenRenderer = HMeshRenderer(&fullscreenMesh, &fullscreenMat);
	float selector = 0.f;
	fullscreenMat.BindUniformBufferData("ViewSelector", &selector, sizeof(float));
	fullscreenMat.BindUniformBufferData("LightmapInfo", &lightView, sizeof(Matrix44));

	for (;;)
	{
		if (The.Input->GetKeyDown('M'))
		{
			selector += 1.f;
			fullscreenMat.BindUniformBufferData("ViewSelector", &selector, sizeof(float));
		}
		UpdateView(cam);

		lightmap->Begin();
		for (uint32 rendererIndex = 0; rendererIndex < renderers.size(); rendererIndex++)
		{
			renderers[rendererIndex].Draw();
		}
		lightmap->Complete();

		renderPass->Begin();
		Time.time = (float)GetCurrentTimeSeconds();
		Time.Push();
 		for (uint32 rendererIndex = 0; rendererIndex < renderers.size(); rendererIndex++)
 		{
 			renderers[rendererIndex].Draw();
 		}
		renderPass->NextSubpass();
		skyboxRenderer.Draw();
 		for (uint32 rendererIndex = 0; rendererIndex < renderers.size(); rendererIndex++)
 		{
 			renderers[rendererIndex].Draw();
 		}
		renderPass->NextSubpass();
		fullscreenRenderer.Draw();
		renderPass->Complete();
	}
}


float deltaSeconds;
//-----------------------------------------------------------------------------------------------
void Tick()
{
	PROFILE_LOG_SECTION(tick);
	static double s_timeLastFrameBegan = GetCurrentTimeSeconds(); // "static" local variables are initialized once, when first encountered, thus this function call only happens once
	double timeThisFrameBegan = GetCurrentTimeSeconds();
	deltaSeconds = static_cast<float>(timeThisFrameBegan - s_timeLastFrameBegan);
	s_timeLastFrameBegan = timeThisFrameBegan;

	static int frameNumber = 0;
	++frameNumber;
	TickEvent te;
	te.deltaSeconds = deltaSeconds;
	g_eventSystem->TriggerEvent("Tick", &te);
	QuEvent::Fire("Tick", QuNamedProperties("DeltaSeconds", "Hello", "DooltaSooconds", deltaSeconds / 2.f, "DailtaSaiconds", deltaSeconds * 2.f));
	//g_theGame->Tick(deltaSeconds);
	The.Input->Tick();
//	The.Audio->Tick(deltaSeconds);
	std::vector<QuString> paths = QuFile::GetPaths("Data/", "*.anim", true);
	DebuggerPrintf("");
	//	theApp->Update();
}

// void RenderPolygon(Vector2 center, float radius, float numSides, float degreeOffset)
// {
// 	const float completeRads = 2.f * pi;
// 	float iterations = completeRads / numSides;
// 	float radOffset = degreeOffset * (pi / 180.f);
// 	glBegin(GL_LINE_LOOP);
// 	{
// 		for (float currentRad = 0.f; currentRad < completeRads; currentRad += iterations)
// 		{
// 			Vector2 vertex;
// 			vertex.SetXY(center.x + (radius * cos(currentRad + radOffset)), center.y + (radius * sin(currentRad + radOffset)));
// 			glVertex2f(vertex.x, vertex.y);
// 		}
// 	}
// 	glEnd();
// }

/*float g_counter = 0.f;*/
//-----------------------------------------------------------------------------------------------
void Render()
{
	PROFILE_LOG_SECTION(render);
	g_theGame->Render(deltaSeconds);
	/*g_theApp->Render();*/

	// Some simple example OpenGL code; remove this and move all rendering inside theApp and child classes
	// 	glClearColor(0.5f, 0.f, 0.f, 1.f);
	// 	glClear(GL_COLOR_BUFFER_BIT);
	// 
	// 	Vector2 center (500.f, 500.f);
	// 
	// 	g_counter += .0004f;
	// 
	// 	RenderPolygon(center, 100.f, 30.f * abs(sin(g_counter)), 90.f);
	// 	// Draw a line from the bottom-left corner of the screen (0,0) to the center of the screen (800,450)
	// // 	glBegin(GL_LINES);
	// // 	glVertex2f(0.f, 0.f);
	// // 	glVertex2f(800.f, 450.f);
	// // 	glEnd();

	SwapBuffers(g_displayDeviceContext);
}


//-----------------------------------------------------------------------------------------------
void RunFrame()
{
	RunMessagePump();
	Tick();
	Render();
}

//-----------------------------------------------------------------------------------------------
void Initialize(HINSTANCE applicationInstanceHandle)
{
	QuPath::InitializeDataPath();
	Network::SystemStartup();
	SetProcessDPIAware();
	CreateOpenGLWindow(applicationInstanceHandle);
//	The.~EngineSystemManager();
//	The.Audio = new AudioSystem();
//	The.Renderer = new GLRenderer();
//	The.Input = new TheInput();
//
//	Mesh::InitializeDefaultMeshes();
//	UniformBlock::InitializeUniformBlocks();
//	ShaderStorageBlock::InitializeShaderStorageBlocks();
//	g_eventSystem = new EventSystem();
//	g_theGame = new TheGame();
//	g_theGame->StartSession();
}
extern Actor* loadedActor;

//-----------------------------------------------------------------------------------------------
void Shutdown()
{
	Network::SystemShutdown();
	Mesh::DestroyDefaultMeshes();
	UniformBlock::DestroyUniformBlocks();
	Texture::DestroyTextures();
	BitmapFont::DestroyFonts();
	SpriteResource::UnloadDatabase();
	ShaderStorageBlock::DestroyShaderStorageBlocks();
	QuEventSystem::DestroySystem();
	delete g_theGame;
	delete g_eventSystem;
	delete The.Audio;
	delete The.Renderer;
	delete The.Input;
	if (loadedActor)
	{
		delete loadedActor;
		loadedActor = nullptr;
	}

	g_theGame = nullptr;
	The.Audio = nullptr;
	The.Renderer = nullptr;
	The.Input = nullptr;
}

#include "Engine/Core/callstack.h"
#include "Engine/Core/Logger.hpp"

#include "Quantum/Hephaestus/BufferDescriptor.h"
//-----------------------------------------------------------------------------------------------
int WINAPI WinMain(HINSTANCE applicationInstanceHandle, HINSTANCE, LPSTR commandLineString, int)
{
	//CallstackSystemInit();
	//MemoryAnalyticsStartup();
	//Profiler::Startup();
	//UNUSED(commandLineString);
	Initialize(applicationInstanceHandle);
	//while (true);
	//while (!g_theGame->m_isQuitting)
	//{
	//	Profiler::MarkFrame();
	//	RunFrame();
	//}
	//Profiler::GetLastFrame();
	//Shutdown();
	//Profiler::Shutdown();
	//MemoryAnalyticsShutdown();
	//CallstackSystemDeinit();
	return 0;
}
