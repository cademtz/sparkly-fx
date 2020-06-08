#pragma once

#include <CatnipBase/Modules/Module.h>

class CESP : public CModule
{
	virtual void Callback(HookTypes Type)
	{
		assert("ESP unimplemented!");

		switch (Type)
		{
			
		}
	};
};

CESP* gESP = new CESP();
