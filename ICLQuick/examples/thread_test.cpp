#include <iclQuick.h>
#include <iclMutex.h>
#include <pthread.h>

#include <QWaitCondition>
#include <QCoreApplication>
#include <QMutex>
#include <QThread>
#include <QTime>
#include <QString>

static QWaitCondition work;
static QWaitCondition workdone;
static const int NTHREADS = 2;
static QMutex m[NTHREADS];


struct Worker : public QThread {

  Worker (int idx):idx(idx){
    start();
    msleep(100);
  }
  int idx;
  

  static void execThreaded(){
    printf("[execThreaded] called\n");
    for(int i=0;i<NTHREADS;i++){
      m[i].lock();
    }
    work.wakeOne();
    work.wakeOne();
    printf("[execThreaded] wating\n");
    for(int i=0;i<NTHREADS;i++){
      workdone.wait(&m[i]);
    }
    printf("[execThreaded] waited\n");
    for(int i=0;i<NTHREADS;i++){
      m[i].unlock();
    }
    printf("[execThreaded] done\n");
  }
  
  void message(const std::string &func, const std::string &txt) {
    printf("[%s] thread %d: %s \n",func.c_str(),idx,txt.c_str());
  }
  
  void run(){
    m[idx].lock();
    while(true){

      message("run","waiting");
      work.wait(&m[idx]);
      //m.unlock();

      message("run","waited starting to work");

      for(int i=0;i<10;i++){
        ImgQ x = scale(create("parrot"),2);
        msleep(10);
        
        message("run",(QString("working ")+QString::number(i*10)+"%").toLatin1().data());
      }
      message("run","work done");
      
      workdone.wakeAll();

    }
    m[idx].unlock();
  }
};

int main(int argc, char *argv[]){
  QCoreApplication app(argc, argv);
  
  Worker w1(0);
  Worker w2(1);
  
  for(int i=0;i<10;i++){
    w1.execThreaded();
  }
  return 0;
}
