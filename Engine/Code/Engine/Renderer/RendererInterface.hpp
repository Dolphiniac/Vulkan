#pragma once


//-----------------------------------------------------------------------------------------------
class RendererInterface
{
public:
	virtual void Update(float deltaSeconds) = 0;
	virtual void Render() const = 0;
	virtual bool ShouldDie() const = 0;
	virtual ~RendererInterface() {}
};