#include <stdio.h>
#include <sys/types.h>
#include <unistd.h>


int main(int argc, char** argv){
  int childrenCount = 4;

  for (int i = 0; i < childrenCount; i++){
         int f = fork();
         if (f == 0){
                 printf("\tI am a fork pid = %d\nMy parent pid = %d\n", getpid(), getppid());
                 pause();
         } else if (f != -1) {
                 printf("I am the parent pid = %d\n", getpid());
         }
         
  }
  pause();
  return 0;
}
