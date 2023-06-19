#ifndef PUSHWORKER_H
#define PUSHWORKER_H

#include "mediabase.h"
#include "utility.h"
class PushWorker {
public:
  PushWorker();
  ~PushWorker();

  RET_CODE Init(const Properties &properties);
};

#endif // PUSHWORKER_H
