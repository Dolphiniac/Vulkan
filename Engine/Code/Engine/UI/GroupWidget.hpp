#pragma once

#include "Engine/UI/BaseWidget.hpp"

#include <vector>


//-----------------------------------------------------------------------------------------------
class GroupWidget : public BaseWidget
{
public:
	GroupWidget() : BaseWidget() { InitializeRendering(); }
	virtual void Update(float deltaSeconds) override;
	virtual void Render() const override;
	virtual bool IsGroup() const override { return true; }
	void AddChild(BaseWidget* widget) { m_children.push_back(widget); }

private:
	std::vector<BaseWidget*> m_children;

private:
	static WidgetGeneratorRegistration s_registration;
};