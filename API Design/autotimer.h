#ifndef _AUTOTIMER_H_
#define _AUTOTIMER_H_

#include <string>

class AutoTimer
{
public:
	explicit AutoTimer(const std::string &name);
	~AutoTimer();
private:
	class Impl;
	Impl *pmImpl;
};

#endif
