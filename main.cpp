#include <iostream>

#include "unique_word_counter.h"

int main(int argc, char *argv[])
{
    if (argc == 2)
    {
        try
        {
            std::cout << unique_word_counter::count(argv[1]) << std::endl;
            return 0;
        }
        catch (std::exception &e)
        {
            std::cout << e.what() << std::endl;
        }
    }

    return 1;
}