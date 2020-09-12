#pragma once
#include "Modules/Module.h"

class CMovement : public CModule
{
public:
	void StartListening() override;

private:
	int OnCreateMove();
};

inline CMovement gMove;