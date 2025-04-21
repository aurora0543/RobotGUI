#ifndef MANUAL_H
#define MANUAL_H
#include "motor.h"
#include "servo.h"
class Manual
{
public:
    Manual();
    void manual_init();
    void manual_forward();
    void manual_backward();
    void manual_left();
    void manual_right();
    void manual_stop();

private:
    Motor* motor;
    Servo* servo;
};

#endif // MANUAL_H
