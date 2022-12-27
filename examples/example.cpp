#include "../dasmig/namegen.hpp"
#include <iostream>


int main()
{
    std::wcout << dasmig::ng::instance().get_name().append_surname()
               << std::endl;

    return 0;
}