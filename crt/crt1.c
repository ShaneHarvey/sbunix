#include <stdlib.h>

void _start(int argc, char *argv[], char *envp[]) {
	exit(main(argc, argv, envp));
}
