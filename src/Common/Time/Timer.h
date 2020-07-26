#pragma once

#include <chrono>

// Basic timer class
class Timer
{
public:
	void Reset();
	
	double GetDelta();

private:
	std::chrono::high_resolution_clock::time_point mStart;

};

