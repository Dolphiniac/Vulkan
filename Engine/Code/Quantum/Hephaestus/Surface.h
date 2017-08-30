#pragma once

#include "Quantum/Hephaestus/Declarations.h"


//-----------------------------------------------------------------------------------------------
class HSurface
{
	friend class HManager;

private:
	HSurface(class HInstance* instance, HephHINSTANCE applicationHandle, HephHWND windowHandle);

public:
	HephSurfaceKHR m_surface;
};