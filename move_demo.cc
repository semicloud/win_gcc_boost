#include <iostream>
#include <string>
#include <utility>

std::string edit(std::string &&);

int main(int argc, char **argv)
{
    std::string s("b");
    std::cout << s << std::endl;
    std::string s1(std::move(s)); // s的状态取决于std::string的移动构造函数的定义，而非std::move就真正移动了s的内部对象和状态
    edit(std::move(s1));          // 
    std::cout << s1 << std::endl;
    return 0;
}

std::string edit(std::string &&s)
{
    s.push_back('a');
    return s;
}