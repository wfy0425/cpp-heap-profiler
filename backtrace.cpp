
#include <execinfo.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <iostream>
#include <string>
#include <chrono>
#include <fstream>
#include <iostream>
#include <gperftools/malloc_hook.h>
using namespace std;
#define BT_BUF_SIZE 100
#define ARRAY_SIZE 100

using std::chrono::duration;
using std::chrono::duration_cast;
using std::chrono::high_resolution_clock;
using std::chrono::milliseconds;

struct AllocRecord
{
  size_t size;
  int64_t time;
};

static void RecordAlloc(const void *ptr, size_t bytes, int skip_count);
static void RecordFree(const void *ptr);
static void MyNewHook(const void *ptr, size_t size);
static void MyDeleteHook(const void *ptr);

size_t allocSize = 0;
size_t *allocSizePtr = &allocSize;

high_resolution_clock::time_point startTime;
static size_t sizeArr[ARRAY_SIZE];
static high_resolution_clock::time_point timeArr[ARRAY_SIZE];
int sizeIndex = 0;
int *sizeIndexPtr = &sizeIndex;
int timeIndex = 0;
int *timeIndexPtr = &timeIndex;

void myfunc2(int iterations){
  const char* x[iterations];
  for(int i=0;i<iterations;i++){
    x[i]="ABCDEFGHIJKLMNOPQRSTUVXYZ"+char(i);
  }
}

void myfunc(int ncalls)
{
  int nptrs;
  void *buffer[BT_BUF_SIZE];
  char **strings;

  nptrs = backtrace(buffer, BT_BUF_SIZE);
  printf("%d backtrace() returned %d addresses\n", ncalls, nptrs);

  /* The call backtrace_symbols_fd(buffer, nptrs, STDOUT_FILENO)
     would produce similar output to the following: */

  strings = backtrace_symbols(buffer, nptrs);
  if (strings == NULL)
  {
    perror("backtrace_symbols");
    exit(EXIT_FAILURE);
  }

  for (int j = 0; j < nptrs; j++)
    printf("%d %s, size: %ld\n", ncalls, strings[j], allocSize);

  free(strings);

  if (ncalls > 1)
    myfunc(ncalls - 1);

  // for(int i=0;i<ncalls;i++){
  //   myfunc2(i*10+1);
  // }
}

static void MyNewHook(const void *ptr, size_t size)
{
  // printf("in my new hook %zu\n",size);
  if (ptr != NULL)
    RecordAlloc(ptr, size, 0);
}

static void MyDeleteHook(const void *ptr)
{
  if (ptr != NULL)
    RecordFree(ptr);
};

// Record an allocation in the profile.
static void RecordAlloc(const void *ptr, size_t bytes, int skip_count)
{
  // // Take the stack trace outside the critical section.
  // void* stack[HeapProfileTable::kMaxStackDepth];
  // int depth = HeapProfileTable::GetCallerStackTrace(skip_count + 1, stack);
  // SpinLockHolder l(&heap_lock);
  // if (is_on) {
  //   heap_profile->RecordAlloc(ptr, bytes, depth, stack);
  //   MaybeDumpProfileLocked();
  // }
  // printf("record alloc %ld num-calls\n", bytes);
  // printf(allocStr, bytes);
  sizeArr[*sizeIndexPtr] = bytes;
  timeArr[*timeIndexPtr] =  high_resolution_clock::now();
  *sizeIndexPtr += 1;
  *timeIndexPtr += 1;
  *allocSizePtr += bytes;
}

// Record a deallocation in the profile.
static void RecordFree(const void *ptr)
{
  // SpinLockHolder l(&heap_lock);
  // if (is_on) {
  //   heap_profile->RecordFree(ptr);
  //   MaybeDumpProfileLocked();
  // }
}

int main(int argc, char *argv[])
{

  MallocHook::AddNewHook(&MyNewHook);
  if (argc != 2)
  {
    fprintf(stderr, "%s num-calls %ld size\n", argv[0], allocSize);
    exit(EXIT_FAILURE);
  }

  myfunc(atoi(argv[1]));

  std::cout << "finish running" << std::endl;
  std::cout << "Allocations:" <<std::endl;
  
  for (int i = 0; i < *sizeIndexPtr && i<ARRAY_SIZE; i++)
  {
    if(sizeArr[i]>0)
      std::cout << "symbol: "
              << "size: " << sizeArr[i] << " - time: "
              << duration_cast<milliseconds>(timeArr[i] - startTime).count() << std::endl;
  }


  exit(EXIT_SUCCESS);
}