#include "Timer.h"

void Timer::Reset()
{
	mStart = std::chrono::high_resolution_clock::now();
}

double Timer::GetDelta()
{
	auto mCurrentTime = std::chrono::high_resolution_clock::now();
	std::chrono::duration<double> elapsed = mCurrentTime - mStart;
	double delta = std::chrono::duration_cast<std::chrono::milliseconds>(elapsed).count();
	mStart = mCurrentTime;

	return delta;
}
