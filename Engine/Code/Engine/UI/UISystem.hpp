#pragma once

#include "Engine/Renderer/RendererInterface.hpp"
#include "Quantum/Core/String.h"
#include "Quantum/Core/EventSystem.h"

#include <vector>
#include <map>


//-----------------------------------------------------------------------------------------------
extern class UISystem* g_uiSystem;
extern struct HWND__* g_hWnd;


//-----------------------------------------------------------------------------------------------
typedef class BaseWidget*(*WidgetGenerationFunc)();


//-----------------------------------------------------------------------------------------------
class UISystem : public RendererInterface
{
	friend class WidgetGeneratorRegistration;
public:
	//-----------------------------------------------------------------------------------------------
	//CTORS AND DTOR
	//-----------------------------------------------------------------------------------------------
	UISystem();
	//-----------------------------------------------------------------------------------------------
	//END CTORS AND DTOR
	//-----------------------------------------------------------------------------------------------

public:
	virtual void Update(float deltaSeconds) override;
	virtual void Render() const override;
	virtual bool ShouldDie() const override { return false; }
	static class BaseWidget* Generate(const QuString& name);

private:
	void LoadSystemFromXML();

private:
	std::vector<class GroupWidget*> m_rootWidgets;
	BaseWidget* m_currentHoveredWidget;

private:
	static std::map<QuHash, WidgetGenerationFunc>* s_generators;
};


//-----------------------------------------------------------------------------------------------
class WidgetGeneratorRegistration
{
public:
	WidgetGeneratorRegistration(const QuString& name, WidgetGenerationFunc func)
	{
		if (!UISystem::s_generators)
		{
			UISystem::s_generators = new std::map<QuHash, WidgetGenerationFunc>();
		}
		UISystem::s_generators->insert(std::make_pair(name, func));
	}
	WidgetGeneratorRegistration() = delete;
	WidgetGeneratorRegistration(const WidgetGeneratorRegistration& other) = delete;
	WidgetGeneratorRegistration(WidgetGeneratorRegistration&& other) = delete;
};