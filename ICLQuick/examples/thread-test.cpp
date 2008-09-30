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
#include <queue>
#include <QSemaphore>

bool echo = false;
const int PACKAGE_COUNT = 10000;
const int WORK_PACKAGES = 2;
const ImgQ IMAGE = create("parrot");
float *DATA = new float[300000];
struct Package{

  Package(int id=0):id(id){}
  void operator()(){
    //    ImgQ x = scale(IMAGE,3);
    for(int i=0;i<1;i++){
      for(int j=0;j<100000;j++){
        DATA[j] = 100.43242*j+i*34324+5;
      }
    }
  }
  int id;
};

QMutex workPackageMutex;
Package work[WORK_PACKAGES];

QSemaphore provided(WORK_PACKAGES);
QSemaphore done(WORK_PACKAGES);

QMutex printfMutex;
bool doEnd = false;



class Worker : public QThread{
  int idx;
public:
  Worker(int idx):idx(idx){
    start();
  }
  
  void run(){
    while(!doEnd){
      provided.acquire();
      workPackageMutex.lock();
      Package p = work[idx];
      workPackageMutex.unlock();
      if(echo) printf("worker %d: executing package %d \n",idx,p.id);
      p();
      done.release();
      //      usedBytes.acquire();
      // ImgQ _x = scale(Image,2); 
       //       fprintf(stderr, "%c", buffer[i % BufferSize]);
       //freeBytes.release();
     }
    // printf("---done \n");
     //     fprintf(stderr, "\n");
  }
};


int main(int argc, char *argv[]) {
  QCoreApplication app(argc, argv);

  provided.acquire(WORK_PACKAGES);
  
  Worker *workers[WORK_PACKAGES];
  for(int i=0;i<WORK_PACKAGES;i++){
    workers[i] = new Worker(i);
  }
  
  tic();
  int packagesDone = 0;
  while(packagesDone < PACKAGE_COUNT){
    done.acquire(WORK_PACKAGES);

    workPackageMutex.lock();
    for(int i=0;i<WORK_PACKAGES;i++){
      work[i] = Package(packagesDone+i); // create new work
    }
    workPackageMutex.unlock();

    provided.release(WORK_PACKAGES);

    
    if(echo) printf("provided work packages : {");
    for(int i=0;echo && i<WORK_PACKAGES;i++){
      if(i<WORK_PACKAGES-1){
        printf("%d,",packagesDone+i);
      }else{
        printf("%d}\n",packagesDone+i);
      }
    }
    packagesDone += WORK_PACKAGES;
  }
  
  doEnd = true;
  toc();
  for(int i=0;i<WORK_PACKAGES;i++){
    workers[i]->wait();
    delete workers[i];
  }
  
  tic();
  for(int i=0;i<PACKAGE_COUNT;i++){
    Package p;
    p();
  }
  toc();
  return 0;
}

 
