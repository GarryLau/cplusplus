
创建API的主要原因是隐藏所有的实现细节，以免将来修改API对已有客户造成影响。任何内部实现细节（那些很可能变更的部分）必须对该API的客户隐藏。主要有两种技巧可以达到此目标：<font color=blue>物理隐藏</font>和<font color=blue>逻辑隐藏</font>。
>物理隐藏：不让用户获得私有源代码；
>逻辑隐藏：使用语言特性限制用户访问API的某些元素。
## 物理隐藏
**物理隐藏表示将内部实现细节（.cpp）与公共接口（.h）分离，存储在不同的文件中。**
一般来说，声明包含在.h文件中，相关的定义包含在.cpp文件中。当然也可以在.h文件中声明方法的位置给出方法的定义。例如：
```C++
class MyClass
{
public:
    void MyMethod()
    {
        printf("Im MyMethod() of MyClass.\n");
    }
};
```
该技巧<font color=red >隐式地要求编译器在任何调用<font color=Teal face="黑体">MyMethod()</font>的地方内联此成员函数</font>。从API设计的角度来看，因为它不仅暴露 此方法的实现代码，而且将代码直接内联到客户程序中，所以这是很拙劣的做法。因此，应该尽量做到，在API的头部只提供声明。不过，为了支持模板和有意的内联，这个规则也会有例外。
## 逻辑隐藏：封装
**逻辑隐藏指的是使用C++语言中受保护和私有的访问控制特征从而限制访问的内部细节。**
封装（面向对象中的一个概念）提供了限制访问对象成员的机制。在C++中，此机制通过对类使用public、protected、private关键字来实现。
>+ public（公有的）：能从类的外部访问这些成员；
>+ protected（受保护的）：只能在该类或该类的派生类中访问这些成员；
>+ private（私有的） ：只能在定义这些成员的类中访问它们。

## Pimpl
Pimpl是“pointer to implementation”的缩写，意为指向实现的指针。Pimpl是针对C++的设计模式，它将所有的私有数据成员、私有成员函数隔离到一个.cpp文件中独立实现的类或结构体内，在.h中仅需要包含指向该类实例的不透明指针（opaque pointer）即可。这样可以将所有实现细节完全和公有头文件分开。Pimpl设计模式利用了C++的一个特点，即可以将类的数据成员定义为指向某个已经声明过的类型的指针。我们把该声明过的类的定义隐藏在.cpp中，而用户是无法通过不透明指针看到其所指向的对象的细节的。本质上，<font color=red >Pimpl是一种同时在逻辑上和物理上隐藏私有数据成员和私有成员函数的方法</font>。
下面举例说明，设计一个“自动定时器”API。创建对象后，当对象被销毁时打印其生存时间。
>不好的API写法：
```C++
/* autotimer.h */
#ifdef _WIN32
#include <windows.h>
#else 
#include <sys/time.h>
#endif
#include <string>
class AutoTimer
{
public:
    /* 定时器构造函数 */
	explicit AutoTimer(const std::string &name);
	/* 析构函数进行对象销毁时定时器报告生存时间 */
	~AutoTimer();
private:
    /* 返回对象已经存在了多长时间 */
	double GetElapsed()const;
	std::string mName;
#ifdef _WIN32
    DWORD mStartTime;
#else
    #struct timeval mStartTime;
#endif
};
```
上面API违反了许多良好的API设计思想，比如它在.h中包含了与平台相关的定义，暴露了定时器在不同平台上的实现细节，任何人都可以从头文件中看到这些平台定义。
设计者的真正目的是将所有的私有成员隐藏在.cpp中，这样就不再需要包含任何繁琐的平台相关项了。Pimpl是将私有成员放置在一个类（结构体）中，这个类在.h中前置声明，在.cpp中定义。例如，改变上述API设计方法为下面良好的API设计方法：
>良好的API写法：
```C++
/* autotimer.h */
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
```
现在的API何其简洁！不再需要在.h中写与平台相关的预处理指令，别人也不能通过.h文件来了解类的任何私有成员了。
在具体实现方面，AutoTimer的构造函数需要分配AutoTimer::pmImpl的内存，并在析构函数中销毁它。所有私有成员必须通过pmImpl指针访问。在大部分实际案例中，使用这种简洁的、不包含实现的API利远大于弊。
下面是设计良好的API的.h文件对应的.cpp文件完整代码：
```C++
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

#endif
```
由于与平台相关的代码的#ifdef语句存在使得.cpp文件看起来有些凌乱，但重要的是，这种凌乱完全包含在.cpp中。原先暴露的成员变量、函数都包在了.cpp中。将Impl类声明为AutoTimer类的私有内嵌类，避免了与该实现相关的符号污染全局命名空间，而且只有AutoTimer的方法可以访问Impl的成员，.cpp中其它类或函数不能访问Impl。
## 参考文献
Martin Reddy 著, 刘晓娜、臧秀涛、林建 译. API Design for C++ [D]. 人民邮电出版社，2017.
<div align=center><img src="http://img.blog.csdn.net/2018030517011018?watermark/2/text/aHR0cDovL2Jsb2cuY3Nkbi5uZXQvbGl1Z2FuNTI4/font/5a6L5L2T/fontsize/400/fill/I0JBQkFCMA==/dissolve/70" width = 384 height = 384 alt="ellipse" align=center /></div><div align=center></div>
更多资料请移步github： 
https://github.com/GarryLau/cplusplus
