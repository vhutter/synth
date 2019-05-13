#define _USE_MATH_DEFINES

#ifdef __TEST__
#include "test/test.h"
#else
#include <exception>
#include "utility.h"
#include "synthMain.h"
#endif

int main(int argc, char** argv)
{
	try {
		log("======================= Program started =======================");
#ifdef __TEST__
		testMain(argc, argv);
#else
		synthMain(argc, argv);
#endif
		log("Program terminated gracefully");
	}
	catch (const std::exception& e) {
		log(std::string("Program terminated due to the following error: ") + e.what());
	}
	catch (...) {
		log("Program terminated due to an unknown error");
	}
	return 0;
}
