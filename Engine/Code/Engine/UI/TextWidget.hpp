#pragma once

#include "Engine/UI/BaseWidget.hpp"
#include "Engine/Text/TextBox.hpp"


//-----------------------------------------------------------------------------------------------
class TextWidget : public BaseWidget
{
public:
	TextWidget();
	virtual void Update(float deltaSeconds) override;
	virtual void Render() const override;
	virtual bool ShouldDie() const override;

private:
};