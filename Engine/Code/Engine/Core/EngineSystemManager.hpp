#pragma once



//-----------------------------------------------------------------------------------------------
class EngineSystemManager;


//-----------------------------------------------------------------------------------------------
extern EngineSystemManager The;


//-----------------------------------------------------------------------------------------------
class EngineSystemManager
{
public:
	class GLRenderer*	Renderer;
	class TheInput*		Input;
	class AudioSystem*	Audio;

public:
	EngineSystemManager();
	~EngineSystemManager();
};