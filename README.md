# ardDelayer
a async delayer for arduino, 一个arduino下使用的非阻塞式异步延时器



### 需解决的问题
1. 延迟某段时间才执行某段代码，但是又不阻塞整个代码的执行。
2. 解决某些代码执行的先后顺序问题(解决多个延时器按顺序延迟执行多段代码问题)
3. 解决多个延时器按顺序**循环**延迟执行多段代码问题
4. 尽量保留原阻塞式延时器的使用习惯。


### 设计
#### 需求1很好解决，不做讨论
#### 需求2,3的设计 
1. Delayer加入一个nextDelayer字段，标示自己执行完成后，可以启用的下一个Delayer。
2. 在Delayer的构造函数中允许初始化延时时间
3. Delayer的void isDelayed(回调函数);执行业务代码
4. 重载Delayer的isDelayed为bool isDelayed();
5. 延时器加入一个状态字AllowStart，用于标示是否可以启用该延时器，这个标志位不对外公开，让Delayer类自己用于控制，一般是上一个延时器执行完成后开下一个延时器的AllowStart
6. 默认情况下AllowStart是开启的，所以要在前延时器的setNextDelayer函数里判断后延时器是否正在运行，若否置后延时器的AllowStart为false，普通不需要串联的定义器可以直接使用。对于循环延时，所有的延时器都被关了，需要提供一个函数开其中一个延时器，以免所有延时器都被关闭。


#### 还有没有更简单的用法？
+ 可以有如：Delayer.delay(300){}的用法吗？
+ 没有！！！异步编程和普通编程本来就不一样。

### 使用方法
#### 独立延时器使用方法
+ 以检测机械按键去抖为例
```
#include <Arduino.h>
#include "delayer.h"

Delayer k1Delayer=Delayer(20);  //第一个按键对应的延时器
Delayer k2Delayer=Delayer(20);  //第二个按键对应的延时器
void setup(){
  Serial.begin(9600);

  pinMode(13,INPUT);  //第一个按键在13引脚
  pinMode(12,INPUT);  //第二个按键在12引脚

  k1Delayer.setAllowStart(false);//没检测到按键前不进行延时计时
  k2Delayer.setAllowStart(false);//没检测到按键前不进行延时计时

}

void loop(){

  //检测到按键产生的电平信号，开启延时计时。
  if(digitalRead(13)==HIGH && k1Delayer.getAllowStart()==false){
    k1Delayer.setAllowStart(true);
  }  

  //如果延时时长到了，电平和最开始还是一样的，则确认按键已稳定按下
  if(k1Delayer.isDelayed()){
    if(digitalRead(13)==HIGH){
      Serial.println("确认K1按下");
      k1Delayer.setAllowStart(false); //完成一次延时计时后，重置延时器不再延时
    }
  }
  
  if(digitalRead(12)==HIGH && k2Delayer.getAllowStart()==false){
    k2Delayer.setAllowStart(true);
  }  
  if(k2Delayer.isDelayed()){
    if(digitalRead(12)==HIGH){
      Serial.println("确认K2按下");
      k2Delayer.setAllowStart(false);//完成一次延时计时后，重置延时器不再延时
    }
  }

}

```


#### 延时器"串联"的典型使用场景和典型用法
+ 以blink为例使用例子一：

```


// the setup function runs once when you press reset or power the board
#include <Arduino.h>
#include "delayer.h"

void onLed();
void offLed();
Delayer onDelayer=Delayer(1000);
Delayer offDelayer=Delayer(1000);

void setup() {
  // initialize digital pin 13 as an output.
  pinMode(13, OUTPUT);
  onDelayer.setNextDelayer(&offDelayer);
  offDelayer.setNextDelayer(&onDelayer);
  
  onDelayer.setAllowEnable();

}


// the loop function runs over and over again forever
void loop() {

  onDelayer.isDelayed(onLed);

  offDelayer.isDelayed(offLed);

}
void onLed(){
    digitalWrite(13, HIGH); 
}
void offLed(){
  digitalWrite(13,LOW);
}

```
+ 以blink为例使用例子二：
```

#include <Arduino.h>
#include "delayer.h"


  Delayer onDelayer=Delayer(1000);
  Delayer offDelayer=Delayer(1000);
  
void setup() {
  // initialize digital pin 13 as an output.
  pinMode(13, OUTPUT);
  
  onDelayer.setNextDelayer(&offDelayer);
  offDelayer.setNextDelayer(&onDelayer);
  onDelayer.setAllowEnable();
  
  Serial.begin(9600);
}

  
void loop() {


  if(onDelayer.isDelayed(500)){
    digitalWrite(13,LOW);

  }

  if(offDelayer.isDelayed(300)){
    digitalWrite(13,HIGH);

  }
}
```
#### 不使用延时器串联方式控制两个LED灯以不同的频率闪烁
```

#include <Arduino.h>
#include "delayer.h"


Delayer k1onDelayer=Delayer(2000);
Delayer k1offDelayer=Delayer(1000);
Delayer k2onDelayer=Delayer(1000);
Delayer k2offDelayer=Delayer(1000);

void setup(){

pinMode(42,OUTPUT);
pinMode(46,OUTPUT);

k1offDelayer.setAllowStart(false);
k2offDelayer.setAllowStart(false);

}

void loop(){
  if(k1onDelayer.isStarted()==false && k1onDelayer.getAllowStart()==true){
    digitalWrite(42,HIGH);
  }
  if(k1onDelayer.isDelayed()){
    digitalWrite(42,LOW);
    k1offDelayer.setAllowStart(true);
    k1onDelayer.setAllowStart(false);
  }

  if(k1offDelayer.isDelayed()){
    digitalWrite(42,HIGH);
    k1onDelayer.setAllowStart(true);
    k1offDelayer.setAllowStart(false);
  }

  if(k2onDelayer.isStarted()==false && k2onDelayer.getAllowStart()==true){
    digitalWrite(46,HIGH);
  }
  if(k2onDelayer.isDelayed()){
    digitalWrite(46,LOW);
    k2offDelayer.setAllowStart(true);
    k2onDelayer.setAllowStart(false);
  }

  if(k2offDelayer.isDelayed()){
    digitalWrite(46,HIGH);
    k2onDelayer.setAllowStart(true);
    k2offDelayer.setAllowStart(false);
  }
}

```
