#pragma once

#include "Engine/Renderer/TheRenderer.hpp"


//-----------------------------------------------------------------------------------------------
class DXRenderer : public TheRenderer
{
	virtual void UseZBuffering(bool shouldUse) override { UNUSED(shouldUse); }
};