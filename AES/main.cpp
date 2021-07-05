#define _CRTDBG_MAP_ALLOC
#include <stdlib.h>
#include <crtdbg.h>


void test();

int main(int argc, char* argv[])
{
	{
		test();
	}
	system("pause");

	_CrtDumpMemoryLeaks();
	return 0;
}