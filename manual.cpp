#include "manual.h"
#include <iostream>
#include "servo.h"
Manual::Manual() : motor(nullptr) {}

void Manual::manual_init()
{
    // 初始化 Motor
    MotorPins pins = {17, 18, 22, 23};  // 替换为你的实际 GPIO 引脚
    motor = new Motor(pins);

    // 初始化 Servo
    servo = new Servo(18);  // 替换为你的实际 GPIO 引脚
}

void Manual::manual_forward()
{
    if (motor) {
        std::cout << "Moving forward" << std::endl;
        motor->forward(50);
    }
}

void Manual::manual_backward()
{
    if (motor) {
        std::cout << "Moving backward" << std::endl;
        motor->backward(50);
    }
}

void Manual::manual_left()
{
    if (motor) {
        std::cout << "Turning left" << std::endl;
        
        servo->turn(5, 45);  // 向左转45度
    }
}

void Manual::manual_right()
{
    if (motor) {
        std::cout << "Turning right" << std::endl;
        
        servo->turn(5, 45);
    }
}

void Manual::manual_stop()
{
    if (motor) {
        std::cout << "Stopping" << std::endl;
        motor->stop();
    }
}