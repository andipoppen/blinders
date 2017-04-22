
#include "mbed.h"
#include "sMotor.h"
#include "PinDetect.h"

DigitalOut ledR(LED1);
DigitalOut ledG(LED2);
DigitalOut ledB(LED3);

sMotor motor(D10, D11, D12, D13); // creates new stepper motor: IN1, IN2, IN3, IN4

int step_speed = 1000; // set default motor speed
int numstep = (4076/8)/10; // defines full turn of 360 degree
//you might want to calibrate this value according to your motor

//static int 57 = 2000;
static int _position = 0;
static int _topPosition = 0;
static int _bottomPosition = 0;

PinDetect pin(D9);

enum State {
    CalibrationIdleOn,
    CalibrationIdleOff,
    IdleTop,
    IdleBottom,
    Going2Top,
    Going2Bottom,
    CalibrateBottom,
    CalibrateTop,
};

static State _state = CalibrationIdleOff;

/*void keyPressed( void ) {
   ledR = 1;
   ledG = 1;
   ledB = 0;
}*/

void keyReleased( void ) {
   switch(_state)
   {
       case CalibrationIdleOn:
       case CalibrationIdleOff:
           _state = CalibrateBottom;
           break;
       case CalibrateBottom:
           _bottomPosition = _position;
           _state = CalibrateTop;
           printf("CalibrateBottom - position=%d\n",_position);
           break;
       case CalibrateTop:
           _topPosition = _position;
           _state = IdleTop;
           motor.disable();
           printf("CalibrateTop - position=%d\n",_position);
           break;
       case IdleTop:
           _state = Going2Bottom;
           break;
       case IdleBottom:
           _state = Going2Top;
           break;
       case Going2Bottom:
           _state = IdleBottom;
           break;
       case Going2Top:
           _state = IdleTop;
           break;
       default:
           ledR = 0;
           ledG = 1;
           ledB = 1;
           break;
   }
}

/*void keyPressedHeld( void ) {
   ledR = 1;
   ledG = 0;
   ledB = 1;
}*/

void keyReleasedHeld( void ) {
   ledR = 0;
   ledG = 1;
   ledB = 0;
   _state = CalibrationIdleOn;
}

int main() {
   int count = 0;
   ledR = 0;
   ledG = 1;
   ledB = 0;
   
    pin.mode( PullUp );
    //pin.attach_asserted( &keyPressed );
    pin.attach_deasserted( &keyReleased );
    //pin.attach_asserted_held( &keyPressedHeld );
    pin.attach_deasserted_held( &keyReleasedHeld );
    pin.setSampleFrequency();

        
    // Main motor loop
    while(1)
    {
       count++;
       switch(_state)
       {
           case CalibrationIdleOff:
               if(count % 100 == 0)
               {
                   ledR = 0;
                   ledG = 1;
                   ledB = 0;
                   _state = CalibrationIdleOn;
               }
               break;
           case CalibrationIdleOn:
               if(count % 100 == 0)
               {
                   ledR = 1;
                   ledG = 1;
                   ledB = 1;
                   _state = CalibrationIdleOff;
               }
               break;
           case Going2Top:
               if(_position > _topPosition)
               {
                   motor.step(numstep,1,step_speed); // number of steps, direction, speed
                   _position -= numstep;
               }
               else
               {
                   _state = IdleTop;
               }
               break;
           case Going2Bottom:
               if(_position < _bottomPosition)
               {
                   motor.step(numstep,0,step_speed); // number of steps, direction, speed
                   _position += numstep;
               }
               else
               {
                   _state = IdleBottom;
               }
               break;
           case CalibrateTop:
               //if(_position > 0)
               {
                   motor.step(numstep,1,step_speed); // number of steps, direction, speed
                   _position -= numstep;
               }
               /*else
               {
                   _state = IdleTop;
               }*/
               break;
           case CalibrateBottom:
               motor.step(numstep,0,step_speed); // number of steps, direction, speed
               _position += numstep;
               break;
           default:
               break;
       }
       wait_us(step_speed);    
       motor.disable();
    }
}


