#include "perfbench.h"


int main(int argc, char* argv[]) {
	Perfbench* pb = new Perfbench;

    // Parse command line
    pb->argparser->parse(argc, argv);

    // Start Perfbench
    pb->start();

	return 0;
}
