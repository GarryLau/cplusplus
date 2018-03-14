#include "autotimer.h"
#include <iostream>
#ifdef _WIN32
#include <windows.h>
#else 
#include <sys/time.h>
#endif

class AutoTimer::Impl
{
public:
	double GetElapsed() const
	{
#ifdef _WIN32
		return (GetTickCount() - mStartTime) / 1e3;
#else
		struct timeval end_time;
		gettimeofday(&end_time, NULL);
		double t1 = mStartTime.tv_usec / 1e6 + mStartTime.tv_sec;
		double t2 = end_time.tv_usec / 1e6 + end_time.tv_sec;
		return t2 - t1;
#endif
	}
	std::string mName;
#ifdef _WIN32
	DWORD mStartTime;
#else 
	struct timeval mStartTime;
#endif
};

AutoTimer::AutoTimer(const std::string &name) :pmImpl(new AutoTimer::Impl())
{
	pmImpl->mName = name;
#ifdef _WIN32
	pmImpl->mStartTime = GetTickCount();
#else
	gettimeofday(&pmImpl->mStartTime, NULL);
#endif
}

AutoTimer::~AutoTimer()
{
	std::cout << pmImpl->mName << ": took " << pmImpl->GetElapsed() << " seconds." << std::endl;
	delete pmImpl;
	pmImpl = NULL;
}
