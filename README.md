# Windows平台MinGW GCC编译并使用C++ Boost

## 前提

操作系统：Windows 10，安装了MinGW，并将gcc.exe等程序添加到Path变量中。

在下载好的Boost目录中运行`booststrap.bat`，该脚本自动生成编译程序*b2.exe*。

## 编译Boost为动态链接库

动态链接的编译脚本如下：

> .\b2.exe stage --toolset=gcc --stagedir="C:\SDK\boost_1_69_0\bin\gcc" --link=shard link=shared runtime-link=shared threading=multi debug release 

该命令说明，使用的编译工具（`--toolset`）为gcc，生成的静态、动态链接库目录（`--stageddir`）为C:\SDK\boost_1_69_0\bin\gcc，Boost库采用动态链接的方式（`--link=shared`），C/C++的运行库也采用动态链接的方式（`--runtime-link=shared`），编译多线程版本（`threading=multi`），生成目标为debug和release模式的库文件。

运行上述编译脚本（需要较长时间，大概10~15分钟左右），编译完成后，可以在C:\SDK\boost_1_69_0\bin\gcc\lib下看到编译完成的动态链接库文件(.dll)和静态链接库文件(.dll.a)。下面以boost的filesystem库为例，说明库文件的组成。

编译为动态链接库的一个Boost Library，编译为4个文件，以filesystem库为例：

libboost_filesystem-mgw82-mt-d-x32-1_69.dll
libboost_filesystem-mgw82-mt-d-x32-1_69.dll.a
libboost_filesystem-mgw82-mt-x32-1_69.dll
libboost_filesystem-mgw82-mt-x32-1_69.dll.a

* mt：代表多线程版本
* d：代表debug模式，无d则代表release模式
* x32：代表32位版本，因为我本机上只安装了MinGW的32位版本，因此只编译出了32位版本的库文件
* 1_69：Boost版本号
* dll：**部署后**使用的动态链接库文件
* .a：**链接时**使用的静态链接库文件

## 使用编译的Boost动态链接库

下面，使用一个简单的程序来使用Boost的动态链接库，该程序打印Boost的版本号，并使用filesystem库来检查Boost安装目录下的b2.exe文件是否存在：

```C++
#include <boost/filesystem.hpp>
#include <boost/version.hpp>
#include <boost/config.hpp>
#include <iostream>

int main(int argc, char** argv){
    std::cout << BOOST_VERSION << std::endl;
    boost::filesystem::path root("C:\\SDK\\boost_1_69_0\\b2.exe");
    std::cout << boost::filesystem::exists(root) << std::endl;
    return 0;
    
}
```

测试程序写好后，下一步就是编译并生成目标文件，我们使用MinGW提供的make工具：*mingw32-make.exe*，来完成编译和链接工作，链接时，使用Boost在release模式下编译生成的库。make脚本如下：

```makefile
TestBoost_dynamic.exe: TestBoost.o copy_dependency
	g++ TestBoost.o -Wall -LC:\SDK\boost_1_69_0\bin\gcc\lib -lboost_filesystem-mgw82-mt-x32-1_69.dll -o TestBoost_dynamic.exe
	
TestBoost.o: TestBoost.cpp
	g++ -c TestBoost.cpp -IC:\SDK\boost_1_69_0
	
copy_dependency: 
	copy C:\SDK\boost_1_69_0\bin\gcc\lib\libboost_filesystem-mgw82-mt-x32-1_69.dll .\libboost_filesystem-mgw82-mt-x32-1_69.dll
	
clean: 
	del TestBoost.o
	del TestBoost_dynamic.exe
	del libboost_filesystem-mgw82-mt-x32-1_69.dll
```

g++的相关参数说明如下（可参考：<https://www.rapidtables.com/code/linux/gcc.html>）：

* -I：程序引用的头文件所在目录，如以上Makefile所示，在编译时用到了该参数，将TestBoost.cpp文件编译成了.obj文件TestBoost.o
* -L: 链接库查找目录，该参数一般应用在链接阶段，将.obj文件和相关第三方库链接在一起，生成目标文件TestBoost_dynamic.exe
* -l（小写的L）：指定待链接的**静态**库文件，因为我们的程序中使用的filesystem库，因此需要指定该库作为链接目标，这里需要注意，此时传给-l参数的是filesystem的静态链接文件，也就是libboost_filesystem-mgw82-mt-x32-1_69.dll.a文件，**gcc规定传给-l的参数必须去掉相关文件前面的lib以及后面的.a**，将中间的文件名传给-l参数，例如：将libmath.a传给-l参数，则为-lmath，因此上述Makefile中传给-l的参数是~~lib~~boost_filesystem-mgw82-mt-x32-1_69.dll~~.a~~
* -o：指定了最终输出文件
* -Wall：打开所有警报信息

使用下面的命令运行该Makefile：

> mingw32-make.exe -f Makefile_dynamic

运行上述命令后，当前目录下应该包括如下文件：

Makefile_dynamic
TestBoost.cpp
libboost_filesystem-mgw82-mt-x32-1_69.dll
TestBoost_dynamic.exe
TestBoost.o

需要特别说明的是，因为我们要以动态链接的形式使用Boost库，因此需要将相关的动态链接库文件（也就是libboost_filesystem-mgw82-mt-x32-1_69.dll）复制到当前目录下，否则，虽然可以成功编译出BoostText_dynamic.exe文件，却无法运行。此时，双击BoostText_dynamic.exe时，Windows会报错，提示缺少动态链接库文件。

另外，可以看到，由于使用动态链接生成目标文件，生成的目标文件TestBoost_dynamic.exe只有387KB，相关依赖都在动态链接库文件中，因此目标文件就相对小了，与此相比，稍后介绍的使用静态链接生成的目标文件TestBoost_static.exe，其体积将大于TestBoost_dynamic.exe，这是因为由于使用静态链接，相关函数都编译到目标文件本身了。

运行BoostTest_dynamic.exe可以得到输出结果：

> 106900
> 1

## 编译Boost为静态链接库

有了编译为动态链接库作为基础，编译静态链接库就比较简单了，使用b2.exe编译静态链接的脚本如下：

> .\b2.exe stage --toolset=gcc --stagedir="C:\SDK\boost_1_69_0\bin\gcc_all_static" --link=static link=static runtime-link=static threading=multi debug release 

为了与动态链接库相区别，我们将编译得到的静态链接库保存到C:\SDK\boost_1_69_0\bin\gcc_all_static目录下。与动态链接库不同，一个Boost Library编译得到了两个.a文件，以filesystem为例：

libboost_filesystem-mgw82-mt-sd-x32-1_69.a
libboost_filesystem-mgw82-mt-s-x32-1_69.a

结合动态链接的例子，容易知道这两个文件分别是debug模式、release模式下的静态链接库文件。

## 使用Boost静态链接库

下面来使用编译得到的Boost静态库，测试程序与上面的相同，但这次使用Boost静态库来生成目标文件TestBoost_static.exe，相关Makefile如下所示：

```makefile
TestBoost_static.exe: TestBoost.o
	g++ TestBoost.o -Wall -LC:\SDK\boost_1_69_0\bin\gcc_all_static\lib -lboost_filesystem-mgw82-mt-s-x32-1_69 -o TestBoost_static.exe
	
TestBoost.o: TestBoost.cpp
	g++ -g -c TestBoost.cpp -IC:\SDK\boost_1_69_0
	
clean: 
	del TestBoost.o
	del TestBoost_static.exe
```

将TestBoost.cpp编译为.obj文件的过程相同，链接时使用的-L参数与-l参数都与动态链接时不同，此时-L参数指定的是前面编译得到的Boost静态链接库的目录，而-l指定的是使用的静态链接库文件libboost_filesystem-mgw82-mt-s-x32-1_69.a，去掉了前面的lib以及后面的.a文件

运行该Makefile后，当前目录生成如下文件：

Makefile_static
TestBoost.cpp
TestBoost_static.exe
TestBoost.o

TestBoost_static.exe即为生成的目标文件，由于使用了静态链接方式，此时的TestBoost_static.exe不再需要动态链接库文件了，因此可以直接运行，得到输出：

> 106900
> 1

值得注意的是，使用静态链接生成的目标文件大小是867KB，与使用动态链接生成的目标文件相比，足足大了500KB，因为相关依赖都编译到目标文件内部了，因此得到的目标文件会比较大。

