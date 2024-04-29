#pragma once
#include <Modules/BaseModule.h>
#include <SDK/vector.h>
#include <unordered_set>
#include <Base/fnv1a.h>
#include <assert.h>
#include <math.h>

typedef class CGameTrace trace_t;
class ITraceFilter;

enum EPathEdit
{
	PathEdit_None,
	PathEdit_Start,
	PathEdit_End,
	_PathEdit_EnumEnd,
};

class TestMovement : public CModule
{
public:
	void StartListening() override;

	// TODO: Grab straight from entlist/engine interface if not in freecam
	const Vector& GetCameraPos() { return m_freecamVec; }
	const QAngle& GetCameraAng() { return m_freecamAng; }

private:
	int OnWindow();
	int OnDraw();
	int OnCreateMove(bool& result, float, class CUserCmd* cmd);
	int OnOverrideView(class CViewSetup* pViewSetup);

	bool IsInFreecam() { return m_freecam; }
	bool ShouldDrawPaths();

	// Returns false if the trace didn't land within the map.
	// GoInsideMap will ignore walls if the trace hits a side facing the void.
	bool SurfaceUnderCrosshair(trace_t* out_Trace, bool GoInsideMap = true, ITraceFilter* Filter = nullptr);

	bool TraceTiles();
	bool RecurseTraceTiles();

	bool m_fakelag = false;

	bool m_freecam = false;
	float m_freecam_speed = 1;
	QAngle m_lastView = QAngle(0, 0, 0);
	Vector m_freecamVec;
	QAngle m_freecamAng;

	bool m_pathDraw = false;
	EPathEdit m_pathEdit = PathEdit_None;
	Vector m_pathEnd = Vector(0);
	Vector m_pathStart = Vector(0);
};

inline TestMovement g_movement;