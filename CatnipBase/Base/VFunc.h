#pragma once

template <class T>
T GetVFunc(void* Inst, size_t Index) {
	return (T)(*(void***)Inst)[Index];
}