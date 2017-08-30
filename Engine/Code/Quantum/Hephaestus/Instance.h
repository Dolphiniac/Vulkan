#pragma once

#include "Quantum/Hephaestus/Declarations.h"


//-----------------------------------------------------------------------------------------------
class HInstance
{
	friend class HManager;

private:
	HInstance(bool enableValidation);
	~HInstance();

public:
	HephInstance m_instance = nullptr;
};