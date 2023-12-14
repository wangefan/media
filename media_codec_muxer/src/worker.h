#ifndef WORKER_H_
#define WORKER_H_
#include <mutex>
#include <thread>

class Worker {
public:
  Worker();
  virtual ~Worker();
  virtual bool Start();
  virtual void Stop();
  virtual void Work() = 0;

protected:
  bool is_running_;
  std::mutex is_running_mutex_;

private:
  std::unique_ptr<std::thread> thread_;
};

#endif // WORKER_H_
