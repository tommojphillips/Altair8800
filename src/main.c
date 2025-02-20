/* main.c
 * Github: https:\\github.com\tommojphillips
 */

#include "stdio.h"

#include "i8080.h"
#include "altair8800.h"

int main(int argc, char** argv) {

	altair8800_init();
	while (1) {		
		altair8800_update();
	}
	altair8800_destroy();
	return 0;
}
