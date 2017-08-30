#include "Engine/Core/EngineSystemManager.hpp"
#include "Engine/Core/Audio.hpp"
#include "Engine/Input/TheInput.hpp"
#include "Engine/Renderer/DXRenderer.hpp"
#include "Engine/Renderer/GLRenderer.hpp"


//-----------------------------------------------------------------------------------------------
EngineSystemManager The;


//-----------------------------------------------------------------------------------------------
EngineSystemManager::EngineSystemManager()
#ifdef __USING_UWP
	: Renderer(new DXRenderer())
#else
	: Renderer(new GLRenderer())
#endif
	, Input(new TheInput())
//	, Audio(new AudioSystem())
{

}


//-----------------------------------------------------------------------------------------------
EngineSystemManager::~EngineSystemManager()
{
	if (Renderer)
		delete Renderer;
	if (Input)
		delete Input;
	if (Audio)
		delete Audio;
}