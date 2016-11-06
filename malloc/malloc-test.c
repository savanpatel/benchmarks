#include <stdio.h>
#include <pthread.h>
#include <time.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <math.h>
#include <string.h>

struct thread_args
{
    int allocation_size;
    int array_size;
    int thread_count;
};

// call this function to start a nanosecond-resolution timer
struct timespec timer_start()
{
    struct timespec start_time;
    clock_gettime(CLOCK_PROCESS_CPUTIME_ID, &start_time);
    return start_time;
}



// call this function to end a timer, returning nanoseconds elapsed as a long
long timer_end(struct timespec start_time)
{
    struct timespec end_time;
    clock_gettime(CLOCK_PROCESS_CPUTIME_ID, &end_time);
    long diffInNanos =
    (round(end_time.tv_nsec/ 1.0e6) -
                        round(start_time.tv_nsec/ 1.0e6));
    return diffInNanos;
}

/*
 *  max memory allocation in testing.
 *  testing will start with size = 1 KB upto  2^n < MAX-SIZE.
 *  it is recommended to have MAX-SIZE in power of 2.
 */
int MAX_SIZE = 1024 * 1024;



/*
 *  Test thread will start for allocated array size = 1 and iteratvely
 *  allocate n number of size of array pointers followed by that many
 *  free() call till MAX_ALLOCATION_ARRAY_SIZE.
 */
int MAX_ALLOCATION_ARRAY_SIZE = 500;


/*
 * default test result output file.
 */
char TEST_OUT_FILE[] = "result.out";



/*
 *  Max number of threads to test with.
 */
int MAX_THREADS = 500;



/*
 * out file to store stats.
 */

FILE *result_file;
pthread_mutex_t result_file_mutex = PTHREAD_MUTEX_INITIALIZER;


/*
 * thread will execute this run function which has parameter for its task
 * it measures time take to run task and outputs it in TEST_OUT_FILE.
 */
void * nice_thread_run(void *args)
{
    int allocation_size, array_size, thread_count;
    allocation_size = ((struct thread_args *)args)->allocation_size;
    array_size = ((struct thread_args *)args)->array_size;
    thread_count = ((struct thread_args *)args)->thread_count;

    int i = 0;
    void **test_arr;
    long time_elapsed_ms;
    struct timespec starttime = timer_start();

    test_arr = malloc(array_size * sizeof(void*));

    for(i = 0; i < array_size; i++)
    {
        test_arr[i] = malloc(allocation_size);
        if(NULL == test_arr)
        {
           break;
        }
    }

    time_elapsed_ms = timer_end(starttime);

    for(i = 0; i < array_size; i++)
    {
        if(NULL != test_arr[i])
        {
          free(test_arr[i]);
        }
    }

    free(test_arr);
    pthread_mutex_lock(&result_file_mutex);
    fprintf(result_file,
            "%d\t%d\t%d\t%ld\n",
            thread_count,
            allocation_size,
            array_size,
            time_elapsed_ms);
    pthread_mutex_unlock(&result_file_mutex);
    return NULL;
    //printf("\n %ld", time_elapsed_nanos);
}


int main()
{
    result_file = fopen(TEST_OUT_FILE, "w+");

    int thread_count = 50;
    int i = 0;
    pthread_t tid[10000];
    int array_size = 1;
    int allocation_size = 8;
    struct thread_args arg;

    while (thread_count <= MAX_THREADS)
    {
      printf(" Thread count, n = %d \n", thread_count);
      while(allocation_size < MAX_SIZE)
      {
            array_size = 1;
            while (array_size < MAX_ALLOCATION_ARRAY_SIZE)
            {
                arg.allocation_size = allocation_size;
                arg.array_size = array_size;
                arg.thread_count = thread_count;
                for(i = 0; i < thread_count; i++)
                {
                    pthread_create(&tid[i], NULL, nice_thread_run, (void *)&arg);
                }

                for(i = 0; i < thread_count; i++)
                {
                    pthread_join(tid[i], NULL);
                }
                array_size *= 10;
            }
            allocation_size *= 2;
      }
      allocation_size = 8;
      thread_count += 50;
    }

    fclose(result_file);
    return 0;
}
