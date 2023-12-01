#pragma once
#include <assert.h>
#include "commonmacros.h"

#define Warning(...)
#define Error(...)
#define Msg(...)
#define ConMsg(...)
#define ConColorMsg(...)
#define AssertMsg(expr, ...) assert(expr)
#define AssertMsg3(expr, ...) assert(expr)
#define AssertValidWritePtr(...)
#define AssertValidReadPtr(...)
#define AssertValidStringPtr(...)
#define COMPILE_TIME_ASSERT(expr) static_assert(expr)
#define VerifyEquals(...)
#define DBGFLAG_ASSERT