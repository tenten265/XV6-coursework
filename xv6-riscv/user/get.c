#include "kernel/types.h"
#include "user/user.h"

int main(void) {
  int year = getyear();
  printf("Current year: %d\n", year);
  exit(0);
}