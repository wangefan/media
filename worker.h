#ifndef WORKER_H_
#define WORKER_H_
#include <thread>
#include "mediabase.h"
#include "utility.h"

class Worker {
public:
  Worker();
  virtual ~Worker();
  RET_CODE Start();
  void Stop();

  virtual void Work() = 0;

protected:
  bool is_running_;
private:
  std::unique_ptr<std::thread> thread_;
};

#endif // WORKER_H_
