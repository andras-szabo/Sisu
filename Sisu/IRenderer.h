#pragma once

struct IRenderer
{
	virtual bool IsSetup() const = 0;
	virtual bool Init() = 0;
};
