#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <inttypes.h>
#include <sys/types.h>
#include <unistd.h>


int main(int argc, char** argv){
  if (argc < 2){
    printf("Pass number of children as first parameter\n");
    exit(EXIT_FAILURE);
  } else if (argc < 3){
    printf("Pass file with data\n");
    exit(EXIT_FAILURE);
  }

  int childrenCount = atoi(argv[1]);
  pid_t* childProcessList = malloc(childrenCount * sizeof(*childProcessList) );

  FILE* inputFile = fopen(argv[2], "r");

  int vectorSize;
  if (fscanf(inputFile, "%d\n", &vectorSize) != 1){
    fclose(inputFile);
    exit(EXIT_FAILURE);
  }

  int * vector = malloc(vectorSize * sizeof(*vector));

  for(int*  i = vector; fscanf(inputFile, "%d", i) == 1; i++)
    ;

  fclose(inputFile);

  for (int i = 0; i < vectorSize; i++)
    printf("%d", vector[i]);
  printf("\n");

  for (int i = 0; i < childrenCount; i++){

    int f = fork();
    if (f == 0){
      printf("\tI am a fork pid = %d\nMy parent pid = %d\n", getpid(), getppid());
      pause();
      // it should never reach this place
    } else if (f != -1) {
      printf("I am the parent pid = %d\n", getpid());
      childProcessList[i] = f;
    }
         
  }


  return 0;
}
