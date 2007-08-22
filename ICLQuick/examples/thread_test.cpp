#include <iclQuick.h>
#include <iclMutex.h>
#include <pthread.h>

#include <QWaitCondition>
#include <QCoreApplication>
#include <QMutex>
#include <QThread>
#include <QTime>
#include <QString>
#include <QMutexLocker>
#include "iclCallbackThread.h"
#include <queue>
#include <QSemaphore>


const int DataSize = 10000;
const int BufferSize = 8192;
char buffer[BufferSize];

QSemaphore freeBytes(BufferSize);
QSemaphore usedBytes;

class Producer : public QThread {
public:
  void run(){
    qsrand(QTime(0,0,0).secsTo(QTime::currentTime()));
    for (int i = 0; i < DataSize; ++i) {
      freeBytes.acquire();
      buffer[i % BufferSize] = "ACGT"[(int)qrand() % 4];
      usedBytes.release();
    }
  }
};

class Consumer : public QThread {
public:
  void run(){
     for (int i = 0; i < DataSize/2; ++i) {
       usedBytes.acquire();
       fprintf(stderr, "%c", buffer[i % BufferSize]);
       freeBytes.release();
     }
     printf("---done \n");
     fprintf(stderr, "\n");
  }
};


int main(int argc, char *argv[]) {
  QCoreApplication app(argc, argv);
  Producer producer;
  Consumer consumer;
  Consumer consumer2;
  producer.start();
  consumer.start();
  consumer2.start();
  producer.wait();
  consumer.wait();
  consumer2.wait();
  return 0;
}

 
