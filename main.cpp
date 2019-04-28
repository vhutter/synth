#define _USE_MATH_DEFINES

#include "synthMain.h"

#ifdef __TEST__
#include "test/testMain.h"
#else
#include "synthMain.h"
#endif

int main(int argc, char** argv)
{
#ifdef __TEST__
	testMain(argc, argv);
#else
	synthMain(argc, argv);
#endif
}
