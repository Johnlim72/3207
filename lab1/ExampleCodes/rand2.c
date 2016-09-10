#include <stdio.h>

int main(void) {

int i;
srand(time(NULL));
for (i =0; i <10; ++i){
	printf("%d\n", rand() % 100);
}

return 0;

}
