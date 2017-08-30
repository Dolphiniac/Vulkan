#include "Game/TheGame.hpp"
#include "Engine/Math/Vector2.hpp"
#include "Engine/Input/TheKeyboard.hpp"
#include "Engine/Renderer/TheRenderer.hpp"
#include "Engine/Core/EngineSystemManager.hpp"
#include "Engine/Math/MathUtils.hpp"
#include "Engine/Renderer/DebugRenderCommand.hpp"
#include "Engine/Renderer/BitmapFont.hpp"
#include "Engine/Renderer/Texture.hpp"
#include "Engine/Core/ConsoleCommand.hpp"
#include "Engine/Actor/Actor.hpp"
#include "Engine/Math/Matrix44.hpp"
#include "Engine/Math/Vector4.hpp"
#include "Engine/Renderer/UniformBlock.hpp"
#include "Engine/Renderer/OpenGLExtensions.hpp"
#include "Engine/Model/FBX.hpp"
#include "Engine/Renderer/Light.hpp"
#include "Engine/Model/MeshBuilder.hpp"

#include "Engine/Core/EngineCommon.hpp"
#include "Engine/Core/FileUtils.hpp"
#include "ThirdParty/XML/xml.hpp"
#include "Engine/Core/ErrorWarningAssert.hpp"
#include "Engine/Renderer/SpriteRenderer.hpp"
#include "Engine/Core/Profiler.hpp"
#include "Engine/Renderer/ParticleSystem.hpp"
#include "Engine/Model/Animator.hpp"
#include "Engine/Core/JobSystem.hpp"
#include "Engine/Core/Memory.hpp"
#include "Engine/Renderer/Sprite.hpp"
#include "Engine/Core/XMLUtils.hpp"
#include "Engine/Text/TextBox.hpp"
#include "Engine/Core/Clock.hpp"
#include "Game/Bullet.hpp"
#include "Game/Enemy.hpp"
#include "Engine/Network/RemoteCommandService.hpp"
#include "Engine/Network/NetSession.hpp"
#include "Engine/Core/Audio.hpp"
#include "Quantum/Core/EventSystem.h"

#include <algorithm>


//-----------------------------------------------------------------------------------------------
TheGame* g_theGame = nullptr;


static Matrix44 proj;
static Matrix44 view;
static Light lights[2];
static TextBox* tb;
static TextBox* tb2;
enum EGameState
{
	MAIN_MENU,
	PLAYING,
	GAME_OVER
};
static EGameState s_gameState;
//-----------------------------------------------------------------------------------------------
TheGame::TheGame()
	: m_timeElapsedSeconds(0.f)
	, m_bgRGB(0.f)
	, m_objAlpha(1.f)
	, m_isQuitting(false)
	, m_debugCommands()
	, m_shouldTimeout(false)
	, m_dtt(DEPTHTEST)
	, m_consoleLog()
	, m_inConsole(false)
	, m_workingString()
{
//	StartMainMenu();
	//g_eventSystem->RegisterEvent<TheGame, &TheGame::Update>("Tick", this);
	QuEvent::Register("Tick", this, &TheGame::Update);
	m_clock = Clock::Create(1.f, nullptr);
	JobSystem::Startup(GENERIC | GENERIC_SLOW, -2);
	//ConsoleCommand cc("actorload unitychan");
	//cc.CallFunc();
	//SetUpUnityChanAnimator();
	The.Input->SetAutomaticMouseReset(true);
	The.Input->SetDefaultMousePos(800, 800);
	The.Input->SetMousePos(The.Input->GetDefaultMousePosf());
	The.Input->SetShouldHideMouse(true);
	m_camera.m_position = Vector3(0.f, 0.f, 0.f);
	m_camera.m_pitchDegreesAboutX = 0.f;
	m_camera.m_yawDegreesAboutY = 0.f;

	proj.MakePerspective(120.f, 16.f / 9.f, .1f, 1000.f);
	UniformBlock::SetMat4("GlobalMatrices", "gProj", proj);
	ConsolePrint = GConsolePrint;
	ConsolePrintf = GConsolePrintf;

	g_spriteRenderer = new SpriteRenderer();
	g_spriteRenderer->SetClearColor(BLACK);
	g_spriteRenderer->SetImportAndVirtualSize(720, 20);
	glViewport(350, 0, 900, 900);
	//SpriteResource::Register("character", "Data/Textures/character.png");
	//SpriteResource::Register("background", "Data/Textures/background.png");
	//SpriteResource::Register("bullet", "Data/Textures/soft_particle.png");
	SpriteResource::Register("johncena", "Data/Textures/johnCena.png");
	//SpriteResource::Register("lucas", "Data/Textures/lucas.png");
	//SpriteResource::Register("soft particle", "Data/Textures/soft_particle.png");
	//ParticleSystemInit();
}


//-----------------------------------------------------------------------------------------------
void TheGame::StartMainMenu()
{
	s_gameState = MAIN_MENU;
	tb = new TextBox(10.f, 10.f, .03f);
	XMLNode textNode = XMLNode::createXMLTopNode("Text");
	textNode.addAttribute("value", "[[Waifu]] [[Hell]]");
	XMLNode effectNode = textNode.addChild("Effect", 0);
	effectNode.addAttribute("color1", "FF0000");
	effectNode.addAttribute("color2", "0000FF");
	XMLNode effectNode2 = textNode.addChild("Effect", 1);
	effectNode2.addAttribute("color1", "FF0000");
	effectNode2.addAttribute("color2", "000000");
	tb->m_transform.position = Vector3(-3.5f, -2.f, 0.f);
	tb->SetFromXMLNode(textNode);


	tb2 = new TextBox(20.f, 10.f, .03f);
	XMLNode textNode2 = XMLNode::createXMLTopNode("Text");
	textNode2.addAttribute("value", "Press [[Start]] to Begin");
	XMLNode effectNode3 = textNode2.addChild("Effect", 0);
	effectNode3.addAttribute("color1", "FFFFFF");
	effectNode3.addAttribute("color2", "00FF00");
	tb2->SetFromXMLNode(textNode2);
	tb2->m_transform.position = Vector3(-8.f, -12.f, 0.f);

}


//-----------------------------------------------------------------------------------------------
TheGame::~TheGame()
{
	QuEvent::UnregisterFromAll(this);
	SAFE_DELETE(g_netSession);
	SAFE_DELETE(m_clock);
	SAFE_DELETE(g_remoteCommandService);
	ClearDebugCommands();
	Material::DestroyMaterials();
	Texture::DestroyTextures();
	ParticleSystemDefinition::DestroySystems();
	delete g_spriteRenderer;
	JobSystem::Shutdown();
}

static int effectState = 0;
static int maxEffectState = 2;
static bool renderSkeleton = true;
static bool debugging = false;
static bool usingListView = true;
static ParticleSystem* myParticleSystem = nullptr;


//-----------------------------------------------------------------------------------------------
void TheGame::Update(QuNamedProperties& props)
{
	float deltaSeconds;
	props.Get("DeltaSeconds", deltaSeconds);
	Tick(deltaSeconds);
}


//-----------------------------------------------------------------------------------------------
void OnUpdateReceived(NetSender& sender, NetMessage& msg)
{
	if (!sender.connection)
	{
		return;
	}
	Sprite* theirActor = g_theGame->FindSprite(sender.connection->GetIndex());

	if (theirActor)
	{
		Vector3 pos;
		if (msg.Read<Vector3>(pos))
		{
			theirActor->m_transform.position = pos;
		}
	}
}


// Still hacky here, still desperate, sorry sorry
// static bool g_hasInitialized = false;
// static bool g_hasConstructedAudio = false;
// static char* g_audioBuffer = nullptr;
// static int g_numChunksReceived = 0;
// static FMOD::Sound* g_receivedRecording = nullptr;
// -----------------------------------------------------------------------------------------------
// void OnAudioReceived(NetSender& sender, NetMessage& msg)
// {
// 	if (!sender.connection)
// 	{
// 		return;
// 	}
// 	if (g_hasConstructedAudio)
// 	{
// 		return;
// 	}
// 
// 	unsigned int totalBytes;
// 	int thisIndex;
// 	int numChunks;
// 	ushort numBytes;
// 	msg.Read<unsigned int>(totalBytes);
// 	msg.Read<int>(thisIndex);
// 	msg.Read<int>(numChunks);
// 	
// 	if (!g_hasInitialized)
// 	{
// 		g_audioBuffer = new char[totalBytes];
// 		g_hasInitialized = true;
// 	}
// 
// 	g_numChunksReceived++;
// 
// 	//For indexing into the buffer
// 	const int k_numBytesPerChunk = 1 KB;
// 
// 	msg.ReadBuffer(g_audioBuffer + thisIndex * k_numBytesPerChunk, numBytes);
// 
// 	if (g_numChunksReceived == numChunks)
// 	{
// 		g_receivedRecording = The.Audio->CreateFromBuffer(g_audioBuffer, totalBytes);
// 		The.Audio->PlaySound(g_receivedRecording);
// 		g_hasConstructedAudio = true;
// 		delete[] g_audioBuffer;
// 	}
// }


//-----------------------------------------------------------------------------------------------
Sprite* TheGame::FindSprite(byte playerIndex)
{
	auto iter = m_sprites.find(playerIndex);

	if (iter == m_sprites.end())
	{
		return nullptr;
	}
	else
	{
		return iter->second;
	}
}


//-----------------------------------------------------------------------------------------------
void TheGame::OnNetworkTick(Event* e)
{
	NetworkTickEvent* nte = (NetworkTickEvent*)e;

	if (nte->connection->IsMe())
	{
		return;
	}

	//Sprite* actor = FindSprite(m_playerIndex);
	//
	//if (actor)
	//{
	//	NetMessage msg(NETMESSAGE_GAME_UPDATE);
	//	msg.Write<Vector3>(actor->m_transform.position);
	//	nte->connection->AddMessage(msg);
	//}

	while (nte->connection->ConstructPacketAndSend(m_playerIndex));
}


//-----------------------------------------------------------------------------------------------
void TheGame::OnConnectionJoin(Event* e)
{
	ConnectionChangeEvent* cce = (ConnectionChangeEvent*)e;

	Sprite* toCreate = SpriteResource::GetInstance("johncena", PLAYER_LAYER);
	toCreate->Enable();
	toCreate->BindUniformBlock("GlobalMatrices");

	m_sprites[cce->connection->GetIndex()] = toCreate;

	if (cce->connection->IsMe())
	{
		m_playerIndex = cce->connection->GetIndex();
	}
}


//-----------------------------------------------------------------------------------------------
void TheGame::OnConnectionLeave(Event* e)
{
	ConnectionChangeEvent* cce = (ConnectionChangeEvent*)e;

	byte index = cce->connection->GetIndex();
	auto iter = m_sprites.find(index);
	if (iter == m_sprites.end())
	{
		return;
	}
	Sprite* toDestroy = iter->second;
	toDestroy->Disable();

	SAFE_DELETE(toDestroy);
	m_sprites.erase(index);
}


//-----------------------------------------------------------------------------------------------
void TheGame::Tick(float deltaSeconds)
{
	//UpdateRecordSound(deltaSeconds);
	if (g_remoteCommandService)
	{
		g_remoteCommandService->Update();
	}
	if (g_netSession)
	{
		g_netSession->Update();
	}
	m_timeElapsedSeconds += deltaSeconds;
	m_clock->Update(deltaSeconds);
	GameUpdate();
	if (!m_inConsole)
	{
		PROFILE_LOG_SECTION(input);
		m_camera.m_position += Camera3D::MOVEMENT_SPEED * deltaSeconds * m_camera.GetForwardXZ() * The.Input->GetVerticalKeyboardAxis() * (The.Input->GetKey(KB_SHIFT) ? 8.f : 1.f);
		m_camera.m_position += Camera3D::MOVEMENT_SPEED * deltaSeconds * m_camera.GetRightXZ() * The.Input->GetHorizontalKeyboardAxis() * (The.Input->GetKey(KB_SHIFT) ? 8.f : 1.f);
		if (The.Input->GetKey('X') || The.Input->GetKey('Z') || The.Input->GetKey('C'))
			m_camera.m_position.y -= Camera3D::MOVEMENT_SPEED * deltaSeconds * (The.Input->GetKey(KB_SHIFT) ? 8.f : 1.f);
		if (The.Input->GetKey(KB_SPACE))
			m_camera.m_position.y += Camera3D::MOVEMENT_SPEED * deltaSeconds * (The.Input->GetKey(KB_SHIFT) ? 8.f : 1.f);


		Vector2 deltaMouse = The.Input->GetDeltaMousePosf();
		m_camera.m_pitchDegreesAboutX += deltaMouse.y * deltaSeconds * 16.f;
		m_camera.m_yawDegreesAboutY -= deltaMouse.x * deltaSeconds * 16.f;

		if (The.Input->GetKeyDown('T'))
		{
			effectState++;
			if (effectState > maxEffectState)
			{
				effectState = 0;
			}
			UniformBlock::SetInt("EffectState", "gEffectState", effectState);
		}

		if (The.Input->GetKeyDown('P'))
		{
			renderSkeleton = !renderSkeleton;
		}
		if (The.Input->GetKeyDown('U'))
		{
			Profiler::ToggleProfiling();
		}
		if (The.Input->GetKeyDown('I'))
		{
			usingListView = !usingListView;
		}

		if (The.Input->GetKeyDown('G'))
		{
			ParticleSystemPlay("spark", Vector2(0.f), FX_LAYER);
		}

		if (The.Input->GetKeyDown('B'))
		{
			if (myParticleSystem)
			{
				ParticleSystemDestroyImmediate(myParticleSystem);
			}
			myParticleSystem = ParticleSystemCreate("smoke", Vector2(0.f), FX_LAYER);
		}

		if (The.Input->GetKeyDown('V'))
		{
			if (myParticleSystem)
			{
				ParticleSystemDestroyImmediate(myParticleSystem);
				myParticleSystem = nullptr;
			}
		}

		if (The.Input->GetKeyDown('N'))
		{
			if (myParticleSystem)
			{
				ParticleSystemDestroy(myParticleSystem);
				myParticleSystem = nullptr;
			}
		}
	}

	lights[0].m_position = m_camera.m_position;

	DebugRenderInput();
	FixAndClampAngles();

	view.SetRotation(m_camera.m_yawDegreesAboutY, m_camera.m_pitchDegreesAboutX, 0.f);
	view.SetTranslation(m_camera.m_position);
	view.InvertOrthonormal();
	
	UniformBlock::SetMat4("GlobalMatrices", "gView", view);
	g_spriteRenderer->Update(deltaSeconds);
}

//Very hacky, but very desperate.  Sorry sorry
// static bool g_isRecording = false;
// static bool g_isDoneRecording = false;
// static float g_timeIntoRecording = 0.f;
// static FMOD::Sound* g_recording;
// 
// static void* g_lock2 = nullptr;
// static void* g_lock1 = nullptr;
// static unsigned int g_lock1Len;
// static unsigned int g_lock2Len;
// //-----------------------------------------------------------------------------------------------
// void TheGame::UpdateRecordSound(float deltaSeconds)
// {
// 	if (g_isDoneRecording)
// 	{
// 		return;
// 	}
// 	if (The.Input->GetKeyDown('O') && !m_inConsole)
// 	{
// 		if (!g_isRecording)
// 		{
// 			g_isRecording = true;
// 			g_timeIntoRecording = 0.f;
// 
// 			g_recording = The.Audio->GetRecordingSound(44100 * 2 * 2);
// 			The.Audio->StartRecord(g_recording);
// 
// 			return;
// 		}
// 	}
// 
// 	if (g_isRecording)
// 	{
// 		g_timeIntoRecording += deltaSeconds;
// 
// 		if (g_timeIntoRecording >= 1.f)
// 		{
// 			The.Audio->StopRecord();
// 			g_isDoneRecording = true;
// 
// 			//This is the size of the audio file in bytes (sample rate * bytes per short(format) * number of channels (2 mics))
// 			const unsigned int numBytes = 44100 * sizeof(short) * 2;
// 			g_recording->lock(0, numBytes, &g_lock1, &g_lock2, &g_lock1Len, &g_lock2Len);
// 			char* buffer = new char[numBytes];
// 			memcpy(buffer, g_lock1, numBytes);
// 			g_recording->unlock(g_lock1, g_lock2, g_lock1Len, g_lock2Len);
// 			g_recording->release();
// 
// 			const int k_bytesPerPacket = 1 KB;
// 
// 			//Pieces we cut the sound data into.  Yayaayayayayay!
// 			const int numChunks = numBytes / k_bytesPerPacket + ((numBytes % k_bytesPerPacket > 0) ? 1 : 0);
// 			int bytesRemaining = numBytes;
// 			for (int i = 0; i < numChunks; i++)
// 			{
// 				//We'll create a reliable message with each of these buffers, index them, and send 'em off to pasture
// 				NetMessage msg(NETMESSAGE_AUDIO);
// 				msg.SetFlag(NETMESSAGEFLAG_RELIABLE);
// 				//We write the number of bytes for buffer allocation, then index and total number of chunks to know when we're done
// 				//After that, we write the raw data from the correct position in the audio buffer into the message buffer
// 				msg.Write<unsigned int>(numBytes);
// 				msg.Write<int>(i);
// 				msg.Write<int>(numChunks);
// 				msg.WriteBuffer(buffer + i * k_bytesPerPacket, (ushort)((bytesRemaining > k_bytesPerPacket) ? k_bytesPerPacket : bytesRemaining));
// 				bytesRemaining -= k_bytesPerPacket;
// 
// 				std::vector<NetConnection*>& conns = g_netSession->GetConnections();
// 				for (NetConnection* conn : conns)
// 				{
// 					conn->AddMessage(msg);
// 				}
// 			}
// 			delete[] buffer;
// 		}
// 	}
// }


void TheGame::FixAndClampAngles()
{

	m_camera.m_pitchDegreesAboutX = Clampf(m_camera.m_pitchDegreesAboutX, -89.f, 89.f);
	while (m_camera.m_yawDegreesAboutY >= 360.f)
		m_camera.m_yawDegreesAboutY -= 360.f;
	while (m_camera.m_yawDegreesAboutY < 0.f)
		m_camera.m_yawDegreesAboutY += 360.f;
}

extern Actor* loadedActor;
//-----------------------------------------------------------------------------------------------
void TheGame::Render(float deltaSeconds)
{
	glViewport(350, 0, 900, 900);
	g_spriteRenderer->Render();

	Matrix44 viewProj = view * proj;
	glLoadMatrixf((float*)&viewProj);
	
	The.Renderer->SetOrtho(Vector2(0.f, 0.f), Vector2(1600.f, 900.f));
	The.Renderer->UseZBuffering(false);
	
	glViewport(0, 0, 1600, 900);
	ConsoleRender(deltaSeconds);
	NetSessionRender();
}


//-----------------------------------------------------------------------------------------------
void TheGame::SetUpPerspectiveProjection() const
{
	float aspect = 16.f / 9.f;
	float fovDegreesHorizontal = 90.f;
	float fovDegreesVertical = fovDegreesHorizontal / aspect;
	float zNear = .1f;
	float zFar = 1000.f;

	The.Renderer->SetPerspective(fovDegreesVertical, aspect, zNear, zFar);
	The.Renderer->UseBackfaceCulling(true);
	The.Renderer->UseZBuffering(true);
}


//-----------------------------------------------------------------------------------------------
void TheGame::DebugRender(float deltaSeconds)
{
	for (DebugRenderCommand* drc : m_debugCommands)
	{
		drc->Draw();
		drc->Tick(deltaSeconds);

	}
	m_debugCommands.erase(std::remove_if(m_debugCommands.begin(), m_debugCommands.end(), [](DebugRenderCommand* drc) { return drc->ShouldDie(); }), m_debugCommands.end());
}


//-----------------------------------------------------------------------------------------------
void TheGame::DebugRenderInput()
{
	if (m_inConsole)
	{
		if (The.Input->GetKeyDown(KB_ESC))
		{
			if (m_workingString.empty())
				m_inConsole = false;
			else
				m_workingString.clear();
		}
		if (The.Input->GetKeyDown(KB_ENTER))
		{
			if (m_workingString.empty())
			{
				m_inConsole = false;
			}
			else
			{
				ConsolePrint(m_workingString, GREY);
				CallCommand(m_workingString);
				m_workingString.clear();
			}
		}
		if (The.Input->GetKeyDown(KB_TICKTILDE))
		{
			m_inConsole = false;
			m_workingString = "";
			The.Input->SetShouldHideMouse(true);
			The.Input->SetAutomaticMouseReset(true);
		}
	
		return;
	}
	
	if (The.Input->GetKeyDown(KB_ESC))
	{
		m_isQuitting = true;
	}
	if (The.Input->GetKeyDown('T'))
	{
		m_shouldTimeout = !m_shouldTimeout;
	}
	if (The.Input->GetKeyDown('Y'))
	{
		if (m_dtt == DEPTHTEST)
			m_dtt = NODEPTHTEST;
		else if (m_dtt == NODEPTHTEST)
			m_dtt = DUALDEPTHTEST;
		else if (m_dtt == DUALDEPTHTEST)
			m_dtt = DEPTHTEST;
	}
	if (The.Input->GetKeyDown('K'))
	{
		m_debugCommands.erase(m_debugCommands.begin(), m_debugCommands.end());
	}
	if (The.Input->GetKeyDown('P'))
	{
		DebugDrawPoint(Vector3(3.f, 3.f, 3.f), m_shouldTimeout, 3.f, m_dtt, MAGENTA);
	}
	if (The.Input->GetKeyDown('L'))
	{
		DebugDrawLine(Vector3(0.f, 0.f, 0.f), Vector3(5.f, 1.f, 10.f), m_shouldTimeout, 3.f, m_dtt, GREY);
	}
	if (The.Input->GetKeyDown('R'))
	{
		DebugDrawArrow(Vector3(1.f, 0.f, 0.f), Vector3(-5.f, 1.f, -5.f), m_shouldTimeout, 3.f, m_dtt, YELLOW);
	}
	if (The.Input->GetKeyDown('B'))
	{
		DebugDrawAABB3(Vector3(3.f, 3.f, 3.f), Vector3(10.f, 10.f, 10.f), m_shouldTimeout, 3.f, m_dtt, MAGENTA, BLACK);
	}
	if (The.Input->GetKeyDown('O'))
	{
		DebugDrawSphere(Vector3(10.f, 10.f, 10.f), 5.f, m_shouldTimeout, 3.f, m_dtt, BLUE);
	}
	if (The.Input->GetKeyDown(KB_TICKTILDE))
	{
		m_inConsole = true;
		m_workingString = "";
		The.Input->SetShouldHideMouse(false);
		The.Input->SetAutomaticMouseReset(false);
	}
}


//-----------------------------------------------------------------------------------------------
void TheGame::CallCommand(const std::string& workingString)
{
	ConsoleCommand cc(workingString);
	bool success = cc.CallFunc();
	if (!success)
	{
		ConsolePrintf(RED, "Invalid command: %s", cc.GetFuncName().c_str());
// 		std::string errString = Stringf("Invalid command: %s", cc.GetFuncName().c_str());
// 		ConsolePrint(errString, RED);
	}
}


//-----------------------------------------------------------------------------------------------
void TheGame::ConsoleRender(float deltaSeconds)
{
	static float cursorBlinkTime = 0.f;
	cursorBlinkTime += deltaSeconds * 2.f;
	if (!m_inConsole)
		return;

	The.Renderer->DrawAABB(AABB2(Vector2(0.f, 0.f), Vector2(1600.f, 900.f)), Rgba(0.f, 0.f, 0.f, .5f));
	float width;
	BitmapFont* arial = BitmapFont::CreateOrGetFont("arial");
	float cellHeight = 20.f;
	float lineSpacing = arial->m_lineHeight * cellHeight / arial->m_base;
	float startHeight = 30.f;
	float startWidth = startHeight * .5f;
	The.Renderer->DrawText2D(Vector2(startWidth, startHeight), m_workingString, cellHeight, WHITE, arial, &width);

	if (((int)(cursorBlinkTime) & 1) == 1)	//Render every other "second" (modified by multiplier)
		The.Renderer->DrawLine(Vector2(startWidth + width, startHeight), Vector2(startWidth + width, startHeight - cellHeight), WHITE, WHITE);

	for (std::tuple<std::string, Rgba>& sLog : m_consoleLog)
	{
		PROFILE_LOG_SECTION(consoleTextLine)
		startHeight += lineSpacing;
		The.Renderer->DrawText2D(Vector2(startWidth, startHeight), std::get<0>(sLog), cellHeight, std::get<1>(sLog), arial);
	}
}

#include <sstream>
//-----------------------------------------------------------------------------------------------
void TheGame::ProfileRender()
{
	PROFILE_LOG_SECTION(profileRender);
	BitmapFont* arial = BitmapFont::CreateOrGetFont("arial");
	float cellHeight = 20.f;
	float lineSpacing = arial->m_lineHeight * cellHeight / arial->m_base;
	float startHeight = 870.f;
	float startWidth = 30.f;

	std::string profileString = Profiler::CompileReport(usingListView ? PRF_HIERARCHICAL_VIEW : PRF_FLAT_VIEW);
	std::stringstream ss;
	ss << profileString;
	std::string currentLine;
	while (std::getline(ss, currentLine))
	{
		startHeight -= lineSpacing;
		The.Renderer->DrawText2D(Vector2(startWidth, startHeight), currentLine, cellHeight, WHITE, arial);
	}
}


//-----------------------------------------------------------------------------------------------
void TheGame::NetSessionRender()
{
	BitmapFont* arial = BitmapFont::CreateOrGetFont("arial");
	float cellHeight = 20.f;
	float lineSpacing = arial->m_lineHeight * cellHeight / arial->m_base;
	float startHeight = 870.f;
	float startWidth = 30.f;

	QuString netDebugString = g_netSession->GetDebugString();
	std::stringstream ss;
	ss << netDebugString.GetRaw();
	std::string currentLine;
	while (std::getline(ss, currentLine))
	{
		startHeight -= lineSpacing;
		The.Renderer->DrawText2D(Vector2(startWidth, startHeight), currentLine, cellHeight, WHITE, arial);
	}
}

extern bool g_isDebuggingMemory;
//-----------------------------------------------------------------------------------------------
void TheGame::MemoryRender(float deltaSeconds)
{
	static float secondsSinceLastMemoryUpdate = 0.f;
	const float k_timeBetweenMemoryUpdates = 1.f;
	static unsigned int allocatedBytesOverLastUpdate = 0;
	static unsigned int freedBytesOverLastUpdate = 0;
#ifdef MEMORY_DETECTION_MODE
	if (!g_isDebuggingMemory)
	{
		secondsSinceLastMemoryUpdate = 0.f;
		return;
	}
	secondsSinceLastMemoryUpdate += deltaSeconds;
	if (secondsSinceLastMemoryUpdate >= k_timeBetweenMemoryUpdates)
	{
		allocatedBytesOverLastUpdate = g_bytesAllocatedSinceLastUpdate;
		freedBytesOverLastUpdate = g_bytesFreedSinceLastUpdate;
		g_bytesFreedSinceLastUpdate = 0;
		g_bytesAllocatedSinceLastUpdate = 0;
		secondsSinceLastMemoryUpdate = 0.f;
	}
	BitmapFont* arial = BitmapFont::CreateOrGetFont("arial");
	float cellHeight = 20.f;
	float lineSpacing = arial->m_lineHeight * cellHeight / arial->m_base;
	float startHeight = 450.f;
	float startWidth = 30.f;

	The.Renderer->DrawText2D(Vector2(startWidth, startHeight), Stringf("Current allocations: %u", g_numAllocations), cellHeight, WHITE, arial);
	startHeight -= lineSpacing;
	The.Renderer->DrawText2D(Vector2(startWidth, startHeight), Stringf("Current allocated bytes: %u", g_totalAllocatedBytes), cellHeight, WHITE, arial);
	startHeight -= lineSpacing;
	The.Renderer->DrawText2D(Vector2(startWidth, startHeight), Stringf("Highwater bytes: %u", g_currentHighwaterBytes), cellHeight, WHITE, arial);
	startHeight -= lineSpacing;
	The.Renderer->DrawText2D(Vector2(startWidth, startHeight), Stringf("Bytes allocated in the last second: %u", allocatedBytesOverLastUpdate), cellHeight, WHITE, arial);
	startHeight -= lineSpacing;
	The.Renderer->DrawText2D(Vector2(startWidth, startHeight), Stringf("Bytes freed in the last second: %u", freedBytesOverLastUpdate), cellHeight, WHITE, arial);
	startHeight -= lineSpacing;
	The.Renderer->DrawText2D(Vector2(startWidth, startHeight), Stringf("Delta allocations in the last second: %i", (int)allocatedBytesOverLastUpdate - (int)freedBytesOverLastUpdate), cellHeight, WHITE, arial);

#endif
}
#define GAME_SESSION_PORT 4334
//-----------------------------------------------------------------------------------------------
void TheGame::StartSession()
{
	if (g_netSession)
	{
		delete g_netSession;
	}

	g_netSession = new NetSession();

	g_netSession->RegisterCoreMessages();
	g_netSession->RegisterMessage(NETMESSAGE_GAME_UPDATE, "gameupdate", OnUpdateReceived);
// 	g_netSession->RegisterMessage(NETMESSAGE_AUDIO, "audio", OnAudioReceived);

	g_eventSystem->RegisterEvent<TheGame, &TheGame::OnConnectionJoin>("OnConnectionJoin", this);
	g_eventSystem->RegisterEvent<TheGame, &TheGame::OnConnectionLeave>("OnConnectionLeave", this);
	g_eventSystem->RegisterEvent<TheGame, &TheGame::OnNetworkTick>("OnNetworkTick", this);

	g_netSession->Start(GAME_SESSION_PORT);

// 	ConsoleCommand cc1("nscreateconnection 0 player0 10.8.144.146:4334");
// 	ConsoleCommand cc2("nscreateconnection 1 player1 10.8.144.146:4335");
// 	cc1.CallFunc();
// 	cc2.CallFunc();

	ConsolePrintf(WHITE, "Bound to %s", g_netSession->GetAddressString().c_str());
}


//-----------------------------------------------------------------------------------------------
static void CorrectPosition(Vector3& position)
{
	const float maximumDimensionalMagnitudeForCharacter = 9.5f;
	if (position.x < -maximumDimensionalMagnitudeForCharacter)
	{
		position.x = -maximumDimensionalMagnitudeForCharacter;
	}
	if (position.x > maximumDimensionalMagnitudeForCharacter)
	{
		position.x = maximumDimensionalMagnitudeForCharacter;
	}
	if (position.y < -maximumDimensionalMagnitudeForCharacter)
	{
		position.y = -maximumDimensionalMagnitudeForCharacter;
	}
	if (position.y > maximumDimensionalMagnitudeForCharacter)
	{
		position.y = maximumDimensionalMagnitudeForCharacter;
	}
}


//-----------------------------------------------------------------------------------------------
void TheGame::GameUpdate()
{
	const float k_movementSpeed = 4.f;
	Vector3 movement(0.f);
	movement.x = The.Input->GetHorizontalXboxAxisLeft() * m_clock->GetDeltaTime() * k_movementSpeed;
	movement.y = The.Input->GetVerticalXboxAxisLeft() * m_clock->GetDeltaTime() * k_movementSpeed;
	Sprite* character = FindSprite(m_playerIndex);
	if (!character)
	{
		return;
	}
	character->m_transform.TranslateBy(movement);
	CorrectPosition(character->m_transform.position);
}


//-----------------------------------------------------------------------------------------------
void TheGame::ConcatToWorkingString(unsigned char toCat)
{
	if (toCat >= ' ' && toCat < 127)
	{
		m_workingString += toCat;
	}
	else if (toCat == '\b')
	{
		if (!m_workingString.empty())
			m_workingString.pop_back();
	}
}


//-----------------------------------------------------------------------------------------------
void TheGame::DebugDrawPoint(const Vector3& position, bool canTimeout, float timeToLive /* = 0.f */, EDepthTestType dtt /* = DEPTHTEST */, const Rgba& color /* = WHITE */)
{
	m_debugCommands.push_back(DRC::DebugPoint(position, canTimeout, timeToLive, dtt, color));
}


//-----------------------------------------------------------------------------------------------
void TheGame::DebugDrawLine(const Vector3& startPos, const Vector3& endPos, bool canTimeout, float timeToLive /* = 0.f */, EDepthTestType dtt/* = DEPTHTEST */, const Rgba& color /* = WHITE */)
{
	m_debugCommands.push_back(DRC::DebugLine(startPos, endPos, canTimeout, timeToLive, dtt, color));
}


//-----------------------------------------------------------------------------------------------
void TheGame::DebugDrawArrow(const Vector3& startPos, const Vector3& endPos, bool canTimeout, float timeToLive /* = 0.f */, EDepthTestType dtt /* = DEPTHTEST */, const Rgba& color /* = WHITE */)
{
	m_debugCommands.push_back(DRC::DebugArrow(startPos, endPos, canTimeout, timeToLive, dtt, color));
}


//-----------------------------------------------------------------------------------------------
void TheGame::DebugDrawAABB3(const Vector3& mins, const Vector3& maxs, bool canTimeout, float timeToLive /* = 0.f */, EDepthTestType dtt /* = DEPTHTEST */, const Rgba& edgeColor /* = WHITE */, const Rgba& faceColor /* = WHITE */)
{
	m_debugCommands.push_back(DRC::DebugAABB3(mins, maxs, canTimeout, timeToLive, dtt, edgeColor, faceColor));
}


//-----------------------------------------------------------------------------------------------
void TheGame::DebugDrawSphere(const Vector3& position, float radius, bool canTimeout, float timeToLive /* = 0.f */, EDepthTestType dtt /* = DEPTHTEST */, const Rgba& color /* = WHITE */)
{
	m_debugCommands.push_back(DRC::DebugSphere(position, radius, canTimeout, timeToLive, dtt, color));
}


//-----------------------------------------------------------------------------------------------
void TheGame::ClearDebugCommands()
{
	for (DebugRenderCommand* drc : m_debugCommands)
	{
		delete drc;
	}
	m_debugCommands.clear();
}


//-----------------------------------------------------------------------------------------------
void TheGame::Clear()
{
	m_consoleLog.clear();
}


//-----------------------------------------------------------------------------------------------
void TheGame::GConsolePrint(const std::string& toPrint, const Rgba& color)
{
	g_theGame->m_consoleLog.push_front(std::make_tuple(toPrint, color));
	if (g_theGame->m_consoleLog.size() > MAX_CONSOLE_LOG_ENTRIES)
	{
		g_theGame->m_consoleLog.pop_back();
	}
}
#include <stdarg.h>
const int STRINGF_STACK_LOCAL_TEMP_LENGTH = 2048;

//-----------------------------------------------------------------------------------------------
void TheGame::GConsolePrintf(const Rgba& color, const char* format, ...)
{
	char textLiteral[STRINGF_STACK_LOCAL_TEMP_LENGTH];
	va_list variableArgumentList;
	va_start(variableArgumentList, format);
	vsnprintf_s(textLiteral, STRINGF_STACK_LOCAL_TEMP_LENGTH, _TRUNCATE, format, variableArgumentList);
	va_end(variableArgumentList);
	textLiteral[STRINGF_STACK_LOCAL_TEMP_LENGTH - 1] = '\0'; // In case vsnprintf overran (doesn't auto-terminate)

	ConsolePrint(std::string(textLiteral), color);
}


//-----------------------------------------------------------------------------------------------
CONSOLE_COMMAND(Help, args)
{
	(void)args;
	std::string testInsertedString = "commands";
	ConsolePrintf(YELLOW, "Supported %s:", testInsertedString.c_str());
	ConsolePrint("help:  show this menu", YELLOW);
	ConsolePrint("clear:  clear the console log", YELLOW);
	ConsolePrint("quit:  quit the application", YELLOW);
}


//-----------------------------------------------------------------------------------------------
CONSOLE_COMMAND(Clear, args)
{
	(void)args;
	g_theGame->Clear();
}


//-----------------------------------------------------------------------------------------------
CONSOLE_COMMAND(Quit, args)
{
	(void)args;
	g_theGame->m_isQuitting = true;
}


//-----------------------------------------------------------------------------------------------
//NSCreateConnection <index> <guid> <host:port>
CONSOLE_COMMAND(NSCreateConnection, args)
{
	NetConnection* thisConn = nullptr;
	if (!g_netSession)
	{
		ConsolePrint("Session does not exist!", RED);
		return;
	}

	std::string helpMessage = "Usage: NSCreateConnection <idx> <guid> <host:port>";

	std::string indexString = args.GetNextArg();
	int intdex;
	try
	{
		intdex = std::stoi(indexString);
	}
	catch (std::invalid_argument&)
	{
		ConsolePrint("Invalid index", RED);
		ConsolePrint(helpMessage, RED);
		return;
	}
	byte index = (byte)intdex;
	std::string guid = args.GetNextArg();
	if (guid == "")
	{
		ConsolePrint("Invalid guid", RED);
		ConsolePrint(helpMessage, RED);
		return;
	}

	std::string addrString = args.GetNextArg();
	sockaddr_in addr;
	if (!Network::GetAddrFromString(addrString.c_str(), &addr))
	{
		ConsolePrint("Invalid addr string", RED);
		ConsolePrint(helpMessage, RED);
		return;
	}

	if (g_netSession->IsMe(addr))
	{
		if (g_netSession->IsIndexTaken(index))
		{
			ConsolePrint("Index is taken", RED);
			ConsolePrint(helpMessage, RED);
			return;
		}
		else if (g_netSession->IsGUIDTaken(guid))
		{
			ConsolePrint("GUID is taken", RED);
			ConsolePrint(helpMessage, RED);
			return;
		}

		g_theGame->m_playerIndex = index;
		g_netSession->SetOwnConnectionInfo(index, guid);
		thisConn = g_netSession->GetOwnConnection();
	}
	else
	{
		NetConnection* newConn = new NetConnection(index, guid, addr);
		if (g_netSession->DoesConnectionExist(newConn))
		{
			ConsolePrint("Address is already connected", RED);
			delete newConn;
			ConsolePrint(helpMessage, RED);
			return;
		}
		else if (g_netSession->IsIndexTaken(index))
		{
			ConsolePrint("Index is taken", RED);
			delete newConn;
			ConsolePrint(helpMessage, RED);
			return;
		}
		else if (g_netSession->IsGUIDTaken(guid))
		{
			ConsolePrint("GUID is taken", RED);
			delete newConn;
			ConsolePrint(helpMessage, RED);
			return;
		}
		else
		{
			g_netSession->AddConnection(newConn);
			thisConn = newConn;
		}
	}

	ConsolePrintf(WHITE, "Connection successfully set! Addr: %s, GUID: \"%s\", Index: %s", addrString.c_str(), guid.c_str(), indexString.c_str());
	
	ConnectionChangeEvent cce;
	cce.connection = thisConn;
	g_eventSystem->TriggerEvent("OnConnectionJoin", &cce);
}


//-----------------------------------------------------------------------------------------------
//NSDestroyConnection <index>
CONSOLE_COMMAND(NSDestroyConnection, args)
{
	std::string indexString = args.GetNextArg();

	int intdex;
	try
	{
		intdex = std::stoi(indexString);
	}
	catch (const std::invalid_argument&)
	{
		ConsolePrint("Invalid index", RED);
		return;
	}
	byte index = (byte)intdex;

	NetConnection* toDestroy = g_netSession->GetConnectionAtIndex(index);
	ConnectionChangeEvent cce;
	cce.connection = toDestroy;

	g_eventSystem->TriggerEvent("OnConnectionLeave", &cce);
	
	g_netSession->DestroyConnection(index);
}


CONSOLE_COMMAND(NSSendReliables, args)
{
	std::string arg = args.GetNextArg();

	int numPings = 0;
	if (arg == "")
	{
		numPings = 1;
	}
	else
	{
		try
		{
			numPings = std::stoi(arg);
		}
		catch (const std::exception&)
		{
			numPings = 1;
		}
	}

	for (int i = 0; i < numPings; i++)
	{
		NetMessage message(NETMESSAGE_PING);
		message.SetFlag(NETMESSAGEFLAG_RELIABLE);
		message.Write<std::string>(Stringf("%i", i));
		std::vector<NetConnection*>& conns = g_netSession->GetConnections();
		for (NetConnection* conn : conns)
		{
			conn->AddMessage(message);
		}
	}
}


CONSOLE_COMMAND(NSSendOOOO, args)
{
	std::string arg = args.GetNextArg();

	int numPings = 0;
	if (arg == "")
	{
		numPings = 1;
	}
	else
	{
		try
		{
			numPings = std::stoi(arg);
		}
		catch (const std::exception&)
		{
			numPings = 1;
		}
	}

	for (int i = 0; i < numPings; i++)
	{
		NetMessage message(NETMESSAGE_PING);
		message.SetFlag(NETMESSAGEFLAG_RELIABLE);
		message.SetFlag(NETMESSAGEFLAG_ORDERED);
		message.Write<std::string>(Stringf("%i", i));
		std::vector<NetConnection*>& conns = g_netSession->GetConnections();
		for (NetConnection* conn : conns)
		{
			conn->AddMessage(message);
		}
	}
}


//-----------------------------------------------------------------------------------------------
CONSOLE_COMMAND(NSHost, args)
{
	std::string arg = args.GetNextArg();

	if (arg == "")
	{
		ConsolePrint("Must supply username", RED);
		return;
	}

	if (!g_netSession)
	{
		ConsolePrint("No net session to host on", RED);
		return;
	}

	if (g_netSession->GetSessionState() != NETSESSIONSTATE_DISCONNECTED)
	{
		ConsolePrint("Session is not in disconnected state", RED);
		return;
	}
	g_netSession->Host(arg.c_str());

	ConsolePrint("Successfully hosted", WHITE);
	ConnectionChangeEvent cce;
	cce.connection = g_netSession->GetOwnConnection();
	g_eventSystem->TriggerEvent("OnConnectionJoin", &cce);
}


//-----------------------------------------------------------------------------------------------
CONSOLE_COMMAND(NSJoin, args)
{
	std::string guid = args.GetNextArg();
	if (guid == "")
	{
		ConsolePrint("Must provide username", RED);
		return;
	}

	std::string addrString = args.GetNextArg();
	sockaddr_in addr;
	if (!Network::GetAddrFromString(addrString.c_str(), &addr))
	{
		ConsolePrint("Invalid host:port combo", RED);
		return;
	}

	if (!g_netSession)
	{
		ConsolePrint("No net session with which to join", RED);
		return;
	}

	if (g_netSession->GetSessionState() != NETSESSIONSTATE_DISCONNECTED)
	{
		ConsolePrint("Session must be valid and disconnected to join", RED);
		return;
	}
	g_netSession->Join(guid.c_str(), addr);

	ConsolePrint("Attempting to join", WHITE);
}


//-----------------------------------------------------------------------------------------------
CONSOLE_COMMAND(NSLeave, args)
{
	UNUSED(args);

	if (!g_netSession)
	{
		ConsolePrint("Cannot leave non-existent session", RED);
		return;
	}

	g_netSession->Leave();
}