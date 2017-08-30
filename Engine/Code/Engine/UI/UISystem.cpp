#include "Engine/UI/UISystem.hpp"
#include "Engine/Renderer/SpriteRenderer.hpp"
#include "Engine/UI/GroupWidget.hpp"
#include "Quantum/FileSystem/FileUtils.h"
#include "Engine/Core/XMLUtils.hpp"
#include "Engine/Core/ErrorWarningAssert.hpp"
#include "Engine/Math/MathUtils.hpp"

#define WIN32_LEAN_AND_MEAN
#include <windows.h>


//-----------------------------------------------------------------------------------------------
UISystem* g_uiSystem = nullptr;
STATIC std::map<QuHash, WidgetGenerationFunc>* UISystem::s_generators = nullptr;
HWND g_hWnd = nullptr;


//-----------------------------------------------------------------------------------------------
//CTORS AND DTOR
//-----------------------------------------------------------------------------------------------


//-----------------------------------------------------------------------------------------------
UISystem::UISystem()
	: m_currentHoveredWidget(nullptr)
{
	LoadSystemFromXML();
}


//-----------------------------------------------------------------------------------------------
//END CTORS AND DTOR
//-----------------------------------------------------------------------------------------------


//-----------------------------------------------------------------------------------------------
void UISystem::Update(float deltaSeconds)
{
	POINT p;
	GetCursorPos(&p);
	ScreenToClient(g_hWnd, &p);
	QuVector2 virtualPos;
	virtualPos.x = RangeMap((float)p.x - 350.f, 0.f, 900.f, -10.f, 10.f);
	virtualPos.y = RangeMap((float)p.y, 0.f, 900.f, -10.f, 10.f);
	virtualPos.y *= -1.f;
	//The above is some finagling to get my cursor in virtual coordinates, which is where everything else is

	for (auto riter = m_rootWidgets.rbegin(); riter != m_rootWidgets.rend(); riter++)
	{
		GroupWidget* gw = *riter;
		BaseWidget* calcHighlighted = gw->GetHighlightedWidget(virtualPos);
		if (calcHighlighted != m_currentHoveredWidget)
		{
			if (m_currentHoveredWidget)
			{
				m_currentHoveredWidget->m_currentState = WIDGETSTATE_ACTIVE;
			}
			if (calcHighlighted)
			{
				calcHighlighted->m_currentState = WIDGETSTATE_HIGHLIGHTED;
			}
			m_currentHoveredWidget = calcHighlighted;
		}
	}
	for (GroupWidget* gw : m_rootWidgets)
	{
		gw->Update(deltaSeconds);
	}
}


//-----------------------------------------------------------------------------------------------
void UISystem::Render() const
{
	for (GroupWidget* gw : m_rootWidgets)
	{
		gw->Render();
	}
}


//-----------------------------------------------------------------------------------------------
STATIC BaseWidget* UISystem::Generate(const QuString& name)
{
	GUARANTEE_OR_DIE(s_generators, "No generators found!");

	auto mapIter = s_generators->find(name);
	GUARANTEE_OR_DIE(mapIter != s_generators->end(), "Specified generator not found!");

	BaseWidget* result = mapIter->second();

	return result;
}


//-----------------------------------------------------------------------------------------------
static BaseWidget* ConstructWidgetFromXMLNode(const XMLNode& node)
{
	BaseWidget* result = UISystem::Generate(node.getName());
	for (int i = 0; i < node.nAttribute(); i++)
	{
		result->SetProperty(node.getAttributeName(i), node.getAttributeValue(i));
	}
	if (result->IsGroup())
	{
		GroupWidget* group = (GroupWidget*)result;
		for (int i = 0; i < node.nChildNode(); i++)
		{
			BaseWidget* childWidget = ConstructWidgetFromXMLNode(node.getChildNode(i));
			group->AddChild(childWidget);
		}
	}

	return result;
}


//-----------------------------------------------------------------------------------------------
static GroupWidget* GatherWidgetsFromXMLPath(const QuString& path)
{
	XMLNode root = XMLUtils::GetRootNode(path.GetRaw());
	BaseWidget* tentativeResult = ConstructWidgetFromXMLNode(root);
	GUARANTEE_OR_DIE(tentativeResult->IsGroup(), "All XML files must start with a group widget");
	GroupWidget* result = (GroupWidget*)tentativeResult;

	return result;
}


//-----------------------------------------------------------------------------------------------
void UISystem::LoadSystemFromXML()
{
	std::vector<QuString> xmlPaths = QuFile::GetPaths("Data/UI/Widgets/", "*.xml");

	for (const QuString& path : xmlPaths)
	{
		GroupWidget* root = GatherWidgetsFromXMLPath(path);
		m_rootWidgets.push_back(root);
	}
}