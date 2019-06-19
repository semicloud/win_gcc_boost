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