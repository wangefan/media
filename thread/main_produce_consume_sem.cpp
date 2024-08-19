#include <mutex>
#include <semaphore.h>
#include <thread>
#include <time.h>
#include <unistd.h>

#define THREAD_NUM 8
#define ELEMS_NUM 10

sem_t g_no_elem_sem;
sem_t g_has_elem_sem;

std::mutex g_elems_buffer_mutex;

int elems_buffer[ELEMS_NUM];
int elems_cur_count = 0;

void print_current_time() {
    auto now = std::chrono::system_clock::now();
    auto in_time_t = std::chrono::system_clock::to_time_t(now);
    std::tm *timeinfo = std::localtime(&in_time_t);

    // Move cursor to the first line and clear the line
    printf("\033[H\033[K");
    printf("Current time: %02d-%02d %02d:%02d:%02d\n",
           timeinfo->tm_mon + 1, // Month (0-11, so add 1)
           timeinfo->tm_mday,    // Day
           timeinfo->tm_hour,    // Hour
           timeinfo->tm_min,     // Minute
           timeinfo->tm_sec      // Second
    );

    // Flush the output to ensure it's immediately printed
    fflush(stdout);
}

void producer() {
  while (true) {
    // produce an element
    // Simulate time taken to produce an element (random sleep up to 500ms)
    usleep(rand() % 500000);
    int num_produced = rand() % 100;

    sem_wait(&g_no_elem_sem);
    g_elems_buffer_mutex.lock();
    elems_buffer[elems_cur_count++] = num_produced;
    printf("Produced %d\n", num_produced);
    g_elems_buffer_mutex.unlock();
    sem_post(&g_has_elem_sem);
  }
}

void consumer() {
  while (true) {
    int num_consumed = -1;
    sem_wait(&g_has_elem_sem);
    g_elems_buffer_mutex.lock();
    num_consumed = elems_buffer[--elems_cur_count];
    printf("Consumed %d\n", num_consumed);
    print_current_time(); // Print the current time after consuming
    g_elems_buffer_mutex.unlock();
    sem_post(&g_no_elem_sem);

    // consume an element
    // Simulate time taken to consume an element (e.g., 100-500ms)
    usleep(100000 + (rand() % 400000));
  }
}

int main(int argc, char *argv[]) {
  srand(time(NULL));
  std::thread threads[THREAD_NUM];

  // Initialize semaphores, there are ELEMS_NUM empty slots
  // at beginning, so the initial value is ELEMS_NUM
  sem_init(&g_no_elem_sem, 0, ELEMS_NUM);
  sem_init(&g_has_elem_sem, 0, 0); // there are no elements at beginning

  // Create producer/consumer threads
  int i = 0;
  for (int i = 0; i < THREAD_NUM; i++) {
    if (i % 2 == 0) {
      threads[i] = std::thread(producer);
    } else {
      threads[i] = std::thread(consumer);
    }
  }

  // join threads
  for (i = 0; i < THREAD_NUM; i++) {
    threads[i].join();
  }

  sem_destroy(&g_no_elem_sem);
  sem_destroy(&g_has_elem_sem);

  return 0;
}
