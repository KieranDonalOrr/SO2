#include "directorios.h"

int main(int argc, char **argv)
{
    
    bmount(argv[1]);
    mi_unlink(argv[2]);
    bumount();
    return 0;
}