
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
#include <map>
#include <chrono>
#include <unordered_map>

#define BT_BUF_SIZE 100

using std::chrono::duration;
using std::chrono::duration_cast;
using std::chrono::high_resolution_clock;
using std::chrono::milliseconds;

static void RecordAlloc(const void *ptr, size_t bytes, int skip_count);
static void RecordFree(const void *ptr);
static void MyNewHook(const void *ptr, size_t size);
static void MyDeleteHook(const void *ptr);
void myfunc(int ncalls);
int GetStackTrace(void **result, int max_depth,
                  int skip_count);

struct AllocRecord
{
  size_t size;
  high_resolution_clock::time_point time;
};

size_t allocSize = 0;
size_t *allocSizePtr = &allocSize;
high_resolution_clock::time_point startTime;
static std::unordered_map<std::string, struct AllocRecord> allocRecordMap;

static size_t sizeArr[100];
static high_resolution_clock::time_point timeArr[100];
int sizeIndex = 0;
int *sizeIndexPtr = &sizeIndex;
int timeIndex = 0;
int *timeIndexPtr = &timeIndex;

// Longest stack trace we record.
static const int kMaxStackDepth = 32;
static const int kStripFrames = 3;

int main(int argc, char *argv[])
{
  void *buffer[BT_BUF_SIZE];
  // init backtrace
  int nptrs = backtrace(buffer, BT_BUF_SIZE);

  allocRecordMap.reserve(1000);

  startTime = high_resolution_clock::now();
  MallocHook::AddNewHook(&MyNewHook);

  // if (argc != 2)
  // {
  //   fprintf(stderr, "%s num-calls %ld size\n", argv[0], allocSize);
  //   exit(EXIT_FAILURE);
  // }

  myfunc(2);

  std::cout << "finish running" << std::endl;
  // for (int i = 0; i < 1000; i++)
  // {
  //   std::cout << "symbol: "
  //             << "size: " << sizeArr[i] << "time: "
  //             << duration_cast<milliseconds>(timeArr[i] - startTime).count() << std::endl;
  // }

  for (const auto &pair : allocRecordMap)
  {
    std::cout << "symbol: " << pair.first << ", size: " << pair.second.size << ", time: " << duration_cast<milliseconds>(pair.second.time - startTime).count() << std::endl;
  }

  exit(EXIT_SUCCESS);
}

void myfunc(int ncalls)
{
  // int nptrs;
  // void *buffer[BT_BUF_SIZE];
  // char **strings;

  // nptrs = backtrace(buffer, BT_BUF_SIZE);
  // printf("%d backtrace() returned %d addresses\n", ncalls, nptrs);

  // /* The call backtrace_symbols_fd(buffer, nptrs, STDOUT_FILENO)
  //    would produce similar output to the following: */

  // strings = backtrace_symbols(buffer, nptrs);
  // if (strings == NULL)
  // {
  //   perror("backtrace_symbols");
  //   exit(EXIT_FAILURE);
  // }

  // for (int j = 0; j < nptrs; j++)
  //   printf("%d %s, size: %ld\n", ncalls, strings[j], allocSize);

  // free(strings);
  // printf("%d\n", ncalls);
  // std::cout << ncalls << std::endl;
  double *a = new double;
  // if (ncalls > 1)
  //   myfunc(ncalls - 1);
  delete a;
}

int GetStackTrace(void **result, int max_depth,
                  int skip_count)
{
  static const int kStackLength = 64;
  void *stack[kStackLength];
  int size;

  size = backtrace(stack, kStackLength);
  skip_count += 2; // we want to skip the current and it's parent frame as well
  int result_count = size - skip_count;
  if (result_count < 0)
    result_count = 0;
  if (result_count > max_depth)
    result_count = max_depth;
  for (int i = 0; i < result_count; i++)
    result[i] = stack[i + skip_count];
}

// Record an allocation in the profile.
static void RecordAlloc(const void *ptr, size_t bytes, int skip_count)
{
  void *stack[kMaxStackDepth];
  int depth = GetStackTrace(stack, skip_count + 1, skip_count + kStripFrames + 1);
  SpinLockHolder l(&heap_lock);
  if (is_on)
  {
    heap_profile->RecordAlloc(ptr, bytes, depth, stack);
    MaybeDumpProfileLocked();
  }
}

// Record a deallocation in the profile.
static void RecordFree(const void *ptr)
{
}

static void MyNewHook(const void *ptr, size_t size)
{
  if (ptr != NULL)
    RecordAlloc(ptr, size, 0);
}

static void MyDeleteHook(const void *ptr)
{
  if (ptr != NULL)
    RecordFree(ptr);
};