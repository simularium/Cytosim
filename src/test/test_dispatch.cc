#include <dispatch/dispatch.h>
#include <stdio.h>

int main() {
  dispatch_queue_t queue = dispatch_queue_create(NULL, NULL); 

  dispatch_sync(queue, ^{
    printf("Hello, world from a dispatch queue!\n");
  });

  dispatch_release(queue);

  return 0;
}