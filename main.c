#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <signal.h>
#include <inttypes.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <unistd.h>
#include <stdbool.h>
#include <pthread.h>
#include <sys/types.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/time.h>
#include <sys/wait.h>

void on_usr1(int signal);

void handle_input(int argc, char **argv);

int sum(int *vector, int begin, int end);

int main(int argc, char **argv)
{
  struct timeval stopTime, startTime;
  gettimeofday(&startTime, NULL);
  // printf("after main");
  handle_input(argc, argv);

  int children_count = atoi(argv[1]);
  pid_t *child_process_list = malloc(children_count * sizeof(*child_process_list));

  FILE *inputFile = fopen(argv[2], "r");

  int data_vector_size;
  if (fscanf(inputFile, "%d\n", &data_vector_size) != 1)
  {
    fclose(inputFile);
    exit(EXIT_FAILURE);
  }

  key_t key_ranges = IPC_PRIVATE;
  key_t key_resaults = IPC_PRIVATE;
  key_t key_data = IPC_PRIVATE;

  int shared_memory_data_id = shmget(key_data, data_vector_size * sizeof(int), 0664 | IPC_CREAT);
  int *data_vector = shmat(shared_memory_data_id, NULL, 0);

  for (int *i = data_vector; fscanf(inputFile, "%d", i) == 1; i++)
    ;

  fclose(inputFile);

  // for (int i = 0; i < data_vector_size; i++)
    // printf("%d", data_vector[i]);
  // printf("\n");

  int shared_memory_result_id = shmget(key_resaults, children_count * sizeof(*data_vector), 0664 | IPC_CREAT);
  int shared_memory_ranges_id = shmget(key_ranges, (children_count + 1) * sizeof(*child_process_list), 0664 | IPC_CREAT);

  sigset_t mask;
  sigemptyset(&mask);
  struct sigaction usr1;
  usr1.sa_handler = (&on_usr1);
  usr1.sa_mask = mask;
  usr1.sa_flags = SA_SIGINFO;
  sigaction(SIGUSR1, &usr1, NULL);
  sigprocmask(SIG_BLOCK, &mask, NULL);

  for (int i = 0; i < children_count; i++)
  {
    int f = fork();
    if (f == 0)
    {
      pause();
      const int my_child_id = i;

      int *shared_ranges = shmat(shared_memory_ranges_id, NULL, 0);
      int *result_vector = shmat(shared_memory_result_id, NULL, 0);
      int *my_data_vector = shmat(shared_memory_data_id, NULL, 0);
      // USR1
      int partial_sum = sum(data_vector, shared_ranges[my_child_id], shared_ranges[my_child_id + 1]);
      result_vector[my_child_id] = partial_sum;
      shmdt(shared_ranges);
      shmdt(result_vector);
      shmdt(my_data_vector);
      exit(EXIT_SUCCESS);
    }
    else if (f != -1)
    {
      child_process_list[i] = f;
    }
  }
  // printf("I am the parent pid = %d\n", getpid());
  for (int i = 0; i < children_count; i++)
  {
    // printf("\tChild %d\n", child_process_list[i]);
  }
  int *ranges_vector = shmat(shared_memory_ranges_id, NULL, 0);
  // printf("after ranges");
  int batch = data_vector_size / children_count;
  ranges_vector[0] = 0;
  for (int i = 1; i <= children_count; i++)
  {
    ranges_vector[i] = ranges_vector[i - 1] + batch;
  }
  // printf("before sleep");
  // sleep(1);
  for (int i = 0; i < children_count; i++)
  {
    kill(child_process_list[i], SIGUSR1);
  }
  for (int i = 0; i < children_count; i++)
  {
    kill(child_process_list[i], SIGUSR1);
  }
  // printf("before wait");
  wait(NULL);
  // printf("After wait");
  int first_index_not_read_by_child = ranges_vector[children_count];
  int *result_vector = shmat(shared_memory_result_id, NULL, 0);
  int64_t partial_sum = sum(data_vector, first_index_not_read_by_child, data_vector_size);
  int64_t sum_from_children = sum(result_vector, 0, first_index_not_read_by_child);

  int64_t result = partial_sum + sum_from_children;

  printf("Sum of all elements in %s is %ld\n", argv[2], result);
  // printf("Executed using %d processes\n", children_count);

  shmdt(data_vector);
  shmdt(ranges_vector);
  shmdt(result_vector);
  free(child_process_list);
  gettimeofday(&stopTime, NULL);
  double execution_time = (stopTime.tv_sec - startTime.tv_sec) * 1000000 + stopTime.tv_usec - startTime.tv_usec;
  printf("time = %lf", execution_time);
  return 0;
}

void on_usr1(int signal)
{
  const char *message = "USR1 was recived\n";
  write(1, message, strlen(message));
}

void handle_input(int argc, char **argv)
{
  if (argc < 2)
  {
    printf("Pass number of children as first parameter\n");
    exit(EXIT_FAILURE);
  }
  else if (argc < 3)
  {
    printf("Pass file with data\n");
    exit(EXIT_FAILURE);
  }
}
//Sum from begin to end -1
int sum(int *vector, int begin, int end)
{
  int sum = 0;
  for (int i = begin; i < end; i++)
  {
    sum += vector[i];
  }
  return sum;
}