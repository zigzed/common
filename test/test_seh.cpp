/** Copyright (C) 2013 wilburlang@gmail.com
 */
#include "common/config.h"
#include "common/sys/seh.h"

int main(int argc, char* argv[])
{

    cxx::sys::seh::filter<cxx::sys::seh::shutdown>::install();
    cxx::sys::seh::filter<cxx::sys::seh::report>::install();

    int* p = 0;
    *p = 0;

    return 0;

}
