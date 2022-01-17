#pragma once
#include "Modules/Module.h"

class CMovement : public CModule
{
public:
	void StartListening() override;

private:
	int OnMenu();
	int OnCreateMove();

	bool m_fakelag;
};

inline CMovement gMove;