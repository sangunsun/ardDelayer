#ifndef delayer_h
#define delayer_h
#include "Arduino.h"
//这是一个“异步非阻塞延时器”，需要延时执行的代码放入if (isDelayed)｛｝里，在多次进入程序中如果时间到了，if里的代码会被执行。
//或者使用的时候 如果延时没到直接return返回，时间到了再扫行后面代码，这对编程就有要求，所以一般放在类的成员函数中使用比较方便，可以使用类成员变量保存现场，

//本类提供了第二种调用方法是使用回调函数，比较推荐使用这种方法，定时的逻辑上和阻塞式延迟是一致的。
//这是延时器，不是定时器

//在多次重入中为了防止重复执行延时器前面的代码，可以把延时器前面的代码放在if(isStarted()==false)里
//即延时器还没有启动时执行，如果延时器已经启动了，证明延时器前面的代码已经执行过一次了。
class Delayer{
    
    bool started=false;
    bool allowStart=true;
    unsigned long delayValue;
    unsigned long preTime;    //millis()返回类型为unsigned long，这里定义preTime为unsigned long，可避免millis()溢出导致的计算错误，因为在无符号整数计算中0-0xFF=1
    Delayer *nextDelayer;
    //bool isDelayed(int delayTime);
    public:
    //初始化的时候要给出毫秒计算的延时值
    //followByThisDelayer为本延时器需要跟随的上一个延时器
    Delayer(unsigned long delayTime=0,Delayer * followByThisDelayer=NULL){
        this->started=false;
        this->allowStart=true;
        this->preTime=0;
        delayValue=delayTime;
        
        //如果给出了本延时器需要跟随的延时器，要进行跟随处理
        if(followByThisDelayer!=NULL){
            followByThisDelayer->setNextDelayer(this);
            if (!followByThisDelayer->getAllowStart() && !followByThisDelayer->isStarted())
            {
                this->allowStart=true;
            }
            
        }

        //无论如何当前延时器的下一个延时器都还没有产生，所以当前延时器的下一延时器为空
        this->nextDelayer=NULL;
    }

    //延时器核心函数，延时计时未到返回false，设定的延时时长已到返回true。
    //如果该延时器还不允许被启用，则不允许进行延时计时。
    bool isDelayed(int delayTime){
        delayValue=delayTime;
        if (allowStart==false){
            started=false;
            return false;
        }
        if (started==false){
            started=true;
            preTime=millis();
            return false;
        }else{
            if(millis()-preTime<delayValue){
                return false;
            }else{
                started=false;
                if(nextDelayer!=NULL){
                    nextDelayer->allowStart=true;
                    allowStart=false;
                }
 
                return true;
            }
        }
    }
    bool isDelayed(){
        return isDelayed(delayValue);
    }


    //如果该延时器还不允许被启用，则不允许进行延时计时。
    //回调方式使用延时器，给一个参数为空，返回为空的回调函数作为参数
 bool isDelayed(unsigned long delayTime, void doAction()){
        this->delayValue =delayTime;
         if (allowStart==false){
            started=false;
            return false;
        }
        if (started==false){
            started=true;
            preTime=millis();
            return false;
        }else{
            if(millis()-preTime<delayValue){
                return false;
            }else{
                started=false;

                doAction();
                if(nextDelayer!=NULL){
                    nextDelayer->allowStart=true;
                    allowStart=false;
                }
                return true;
            }
        }
    }

    bool isDelayed(void doAction()){
        return isDelayed(this->delayValue,doAction);
    }

    //设置本延时器的下一延时器指针值。该值须是一个延时器对象地址
    void setNextDelayer(Delayer* nDelayer){
        this->nextDelayer=nDelayer;
        if (nDelayer==NULL){
            return;
        }
        if(!nDelayer->isStarted()){
            nDelayer->allowStart=false;
        }
    }

    //取本延时器指向的下一个延时器。
    Delayer * getNextDelayer(){
        return this->nextDelayer;
    }

    //设置允许本延时器进行延时。
    void setAllowStart(bool startStat){
        this->allowStart=startStat;
    }
   //读取时是否允许本延时器进行延时
    bool getAllowStart(){
        return allowStart;
    }

    //读取本延时器是否已经开始延时
    bool isStarted(){
        return started;
    }

    //读取本延时器的延时时长，以毫秒计
    unsigned long getDelayValue(){
        return delayValue;
    }
};

#endif
