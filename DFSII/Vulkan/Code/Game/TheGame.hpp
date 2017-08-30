#pragma once
#include "Game/Camera3D.hpp"
#include "Engine/Renderer/DebugRenderCommand.hpp"
#include "Engine/Renderer/TheRenderer.hpp"
#include "Engine/Core/EventSystem.hpp"
#include "Engine/Network/NetSession.hpp"

#include <string>
#include <deque>
#include <vector>
#include <tuple>
#include <set>
#include <map>


//-----------------------------------------------------------------------------------------------
class TheGame;


//-----------------------------------------------------------------------------------------------
extern TheGame* g_theGame;


enum EParticleType
{
	NONE,
	FIREWORK,
	SMOKE,
	FOUNTAIN,
	DEBRIS,
	ESPLOAD,
	CIRCLE_CANNON,
	NUM_PARTICLE_TYPES
};


//-----------------------------------------------------------------------------------------------
enum EGameNetMessages : byte
{
	NETMESSAGE_GAME_UPDATE = NETMESSAGE_CORE_COUNT,
	NETMESSAGE_AUDIO,
	NETMESSAGE_GAME_COUNT
};


//-----------------------------------------------------------------------------------------------
class TheGame
{
	static const int MAX_PARTICLES = 200;

public:
	static const int MAX_CONSOLE_LOG_ENTRIES = 35;

public:
	void Update(class QuNamedProperties& props);
	TheGame();

	void StartMainMenu();

	~TheGame();
	void Render(float deltaSeconds);

	void FixAndClampAngles();
	void SetUpPerspectiveProjection() const;
	void DebugDrawPoint(const Vector3& position, bool canTimeout, float timeToLive = 0.f, EDepthTestType dtt = DEPTHTEST, const Rgba& color = WHITE);
	void DebugDrawLine(const Vector3& startPos, const Vector3& endPos, bool canTimeout, float timeToLive = 0.f, EDepthTestType dtt= DEPTHTEST, const Rgba& color = WHITE);
	void DebugDrawArrow(const Vector3& startPos, const Vector3& endPos, bool canTimeout, float timeToLive = 0.f, EDepthTestType dtt = DEPTHTEST, const Rgba& color = WHITE);
	void DebugDrawAABB3(const Vector3& mins, const Vector3& maxs, bool canTimeout, float timeToLive = 0.f, EDepthTestType dtt = DEPTHTEST, const Rgba& edgeColor = WHITE, const Rgba& faceColor = WHITE);
	void DebugDrawSphere(const Vector3& position, float radius, bool canTimeout, float timeToLive = 0.f, EDepthTestType dtt = DEPTHTEST, const Rgba& color = WHITE);
	static void GConsolePrint(const std::string& toPrint, const Rgba& color);
	static void GConsolePrintf(const Rgba& color, const char* format, ...);
	void DebugRender(float deltaSeconds);
	void DebugRenderInput();
	void CallCommand(const std::string& m_workingString);
	void ConsoleRender(float deltaSeconds);
	void ConcatToWorkingString(unsigned char toCat);
	void Clear();
	void ClearDebugCommands();
	void ProfileRender();
	void NetSessionRender();
	void MemoryRender(float deltaSeconds);
	void StartPlayState();
	void StartSession();
	void OnConnectionJoin(Event* e);
	void OnConnectionLeave(Event* e);
	void OnNetworkTick(Event* e);
public:
	bool			m_isQuitting;
	std::deque<std::tuple<std::string, Rgba>>			m_consoleLog;
	byte												m_playerIndex;

private:
	void Tick(float deltaSeconds);
	void UpdateRecordSound(float deltaSeconds);
	friend void OnUpdateReceived(NetSender& sender, NetMessage& msg);
	friend void OnAudioReceived(NetSender& sender, NetMessage& msg);
	class Sprite* FindSprite(byte playerIndex);
	Camera3D											m_camera;
	float												m_timeElapsedSeconds;
	float												m_bgRGB;
	float												m_objAlpha;
	std::vector<DebugRenderCommand*>					m_debugCommands;
	bool												m_shouldTimeout;
	EDepthTestType										m_dtt;
	bool												m_inConsole;
	std::string											m_workingString;
	class Clock*										m_clock;
	void GameUpdate();
	void SpawnBullets();
	std::set<class Bullet*>								m_bullets;
	std::set<class Bullet*>								m_enemyBullets;
	void BurstFrom(class Enemy* enemy);
	void CheckCollision(Enemy* enemy1, Bullet* bullet);
	void StartWinState();
	void StartLoseState();
	std::map<byte, Sprite*>								m_sprites;
};