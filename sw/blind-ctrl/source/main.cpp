
#include "mbed.h"
#include "sMotor.h"
#include "PinDetect.h"
#include <events/mbed_events.h>
#include <mbed.h>
#include "ble/BLE.h"
#include "BlindService.h"
#include "DataStorage.h"

DataStorage ds;
DigitalOut ledR(D9);
BLE* ble = 0;

sMotor motor(D13, D11, D12, D10); // creates new stepper motor: IN1, IN2, IN3, IN4

int step_speed = 1000; // set default motor speed
int numstep = (4076/8)/10; // defines full turn of 360 degree

#define DEFAULT_DOWN_CALIBRATE_POSITION 12000
#define DEFAULT_DOWN_POSITION 8700 // 0x21FC bottom measured
#define DEFAULT_UP_POSITION 100


static int32_t _position = 0; // This is the position where the HALL effekt magnetic sensor is triggered
static int32_t _upPosition = DEFAULT_UP_POSITION; // Slighlty below the HALL effect sensor
static int32_t _bottomPosition = DEFAULT_DOWN_POSITION; // Bottom position of the blind
static int32_t _topPosition = -DEFAULT_DOWN_POSITION*2 - DEFAULT_UP_POSITION; // Top position of the blind. Can actually not go all the way but to make calibration possible from the bottom position

void handleHallSensorFall();

DigitalOut actuatedLED(LED1, 0);

InterruptIn hallSensorIn(A5); // NINA_B1_GPIO_16

const static char     DEVICE_NAME[] = "Blinds";
static const uint16_t uuid16_list[] = { BlindService::BLIND_SERVICE_UUID };

static EventQueue eventQueue(
    /* event count */10 * /* event size */ 32
    );

BlindService *blindServicePtr;


PinDetect pin(SW1);

enum State {
    WaitForTopPos,
    IdleTop,
    IdleBottom,
    Going2Top,
    Going2Bottom,
    Failure
};


static void enterGoing2Top();
static void enterFailure();
static void exitGoing2Top();
static void stepMotorUp();
static void stepMotorDown();


static State _state = IdleBottom;

#define SET_STATE(state) set_state(state,#state)

void set_state(State state, const char *state_str)
{
    _state = state;
    printf("State change to: %s\n", state_str);
}

void stepMotorUp()
{
    motor.step(numstep*100, 1, step_speed); // number of steps, direction, speed
    _position -= numstep;
    motor.disable();
}

void stepMotorDown()
{
    motor.step(numstep*100, 0, step_speed); // number of steps, direction, speed
    _position += numstep;
    motor.disable();
}

static void decrease_position()
{
    _position -= numstep;
    ble->gattServer().write(blindServicePtr->getValueHandlePos(), (uint8_t*)&_position, 4);
    printf("Pos = %d\n", _position);
}

static void increase_position()
{
    _position += numstep;
    ble->gattServer().write(blindServicePtr->getValueHandlePos(), (uint8_t*)&_position, 4);
    printf("Pos = %d\n", _position);
}

static void motor_thread() {
    int count = 0;
    ledR = 0;
   
    //pin.mode(PullUp);
    //pin.attach_asserted( &keyPressed );
    //pin.attach_deasserted(&keyReleased);
    //pin.attach_asserted_held( &keyPressedHeld );
    //pin.attach_deasserted_held(&keyReleasedHeld);
    pin.setSampleFrequency();

    // Main motor loop
    while (1)
    {
        count++;
        switch (_state)
        {
        case Going2Top:
            if (_position > _topPosition)
            {
                motor.step(numstep, 1, step_speed); // number of steps, direction, speed
                decrease_position();
            }
            else
            {
                handleHallSensorFall(); // Emulate that we got the sensor indication
                    //SET_STATE(IdleTop);
                //enterFailure();
            }
            break;
        case WaitForTopPos:
            motor.step(numstep, 0, step_speed); // number of steps, direction, speed
            increase_position();
            if(_position > _upPosition)
            {
                SET_STATE(IdleTop);
            }
            break;
        case Going2Bottom:
            if (_position < _bottomPosition)
            {
                motor.step(numstep, 0, step_speed); // number of steps, direction, speed
                increase_position();
            }
            else
            {
                SET_STATE(IdleBottom);
            }
            break;
        default:
            break;
        }
        wait_us(step_speed);    
        motor.disable();
    }
}


void disconnectionCallback(const Gap::DisconnectionCallbackParams_t *params)
{
    (void)params;
    BLE::Instance().gap().startAdvertising();
}

void blinkCallback(void)
{
    ledR = !ledR.read();
    printf("ledR = %d\n",ledR.read());
    
}
/*
void onDataReadCallback(const GattReadCallbackParams *params) {
    if ((params->handle == blindServicePtr->getValueHandlePos())) {
        
    }
}
*/
/**
* This callback allows the LEDService to receive updates to the ledState Characteristic.
*
* @param[in] params
*     Information about the characterisitc being updated.
*/
void onDataWrittenCallback(const GattWriteCallbackParams *params) {
    if ((params->handle == blindServicePtr->getValueHandleCmd()) && (params->len == 1)) {
        switch (*(params->data))
        {
        case BlindService::BLIND_CMD_UP:
            switch (_state)
            {
            case IdleBottom:
            case Going2Bottom:
                enterGoing2Top();
                break;
            default:
                break;
            }
            break;
        case BlindService::BLIND_CMD_DOWN:
            switch (_state)
            {
            case IdleTop:
            case Going2Top:
                exitGoing2Top();
                SET_STATE(Going2Bottom);
                break;
            default:
                break;
            }
            break;
        case BlindService::BLIND_CMD_TOGGLE:
            switch (_state)
            {
            case IdleTop:
                SET_STATE(Going2Bottom);
                break;
            case IdleBottom:
                enterGoing2Top();
                break;
            case Going2Bottom:
                SET_STATE(IdleBottom);
                break;
            case Going2Top:
                exitGoing2Top();
                SET_STATE(IdleTop);
                break;
            }
            break;
        case BlindService::BLIND_CMD_UP_CALIBRATE:
            _upPosition = DEFAULT_UP_POSITION; // Stop at default up position
            enterGoing2Top();
            break;
        case BlindService::BLIND_CMD_DOWN_CALIBRATE:
            _bottomPosition = DEFAULT_DOWN_CALIBRATE_POSITION; // Big enough value to make it go down all the way
            SET_STATE(Going2Bottom);
            break;
        case BlindService::BLIND_CMD_SET_UP_POS:
            _upPosition = _position;
            SET_STATE(IdleTop);
            break;
        case BlindService::BLIND_CMD_SET_BOTTOM_POS:
            _bottomPosition = _position;
            SET_STATE(IdleBottom);
            break;
            
        default:
            // Ignore
            break;
        }
    }
}

/**
* This function is called when the ble initialization process has failled
*/
void onBleInitError(BLE &ble, ble_error_t error)
{
    /* Initialization error handling should go here */
}

/**
* Callback triggered when the ble initialization process has finished
*/
void bleInitComplete(BLE::InitializationCompleteCallbackContext *params)
{
    BLE&        ble = params->ble;
    ble_error_t error = params->error;

    if (error != BLE_ERROR_NONE) {
        /* In case of error, forward the error handling to onBleInitError */
        onBleInitError(ble, error);
        return;
    }

    /* Ensure that it is the default instance of BLE */
    if (ble.getInstanceID() != BLE::DEFAULT_INSTANCE) {
        return;
    }

    ble.gap().onDisconnection(disconnectionCallback);
    ble.gattServer().onDataWritten(onDataWrittenCallback);
   // ble.gattServer().onDataRead(onDataReadCallback);

    bool initialValueForBlindCharacteristic = false;
    uint32_t initialValueForBlindCharacteristicPos = 0;
    blindServicePtr = new BlindService(ble, initialValueForBlindCharacteristic, initialValueForBlindCharacteristicPos);

    /* setup advertising */
    ble.gap().accumulateAdvertisingPayload(GapAdvertisingData::BREDR_NOT_SUPPORTED | GapAdvertisingData::LE_GENERAL_DISCOVERABLE);
    ble.gap().accumulateAdvertisingPayload(GapAdvertisingData::COMPLETE_LIST_16BIT_SERVICE_IDS, (uint8_t *)uuid16_list, sizeof(uuid16_list));
    ble.gap().accumulateAdvertisingPayload(GapAdvertisingData::COMPLETE_LOCAL_NAME, (uint8_t *)DEVICE_NAME, sizeof(DEVICE_NAME));
    ble.gap().setAdvertisingType(GapAdvertisingParams::ADV_CONNECTABLE_UNDIRECTED);
    ble.gap().setAdvertisingInterval(1000); /* 1000ms. */
    ble.gap().startAdvertising();
    
}

void scheduleBleEventsProcessing(BLE::OnEventsToProcessCallbackContext* context) {
    BLE &ble = BLE::Instance();
    eventQueue.call(Callback<void()>(&ble, &BLE::processEvents));
}

void enterGoing2Top()
{
    SET_STATE(Going2Top);
    // TODO start timer
}

void exitGoing2Top()
{
    // TODO stop timer
}

void enterFailure()
{
    eventQueue.call_every(1000, blinkCallback);
}

void handleHallSensorFall()
{
    switch (_state)
    {
    case Going2Top:
        {
        _position = 0; // Calibrate position
        SET_STATE(WaitForTopPos);
        exitGoing2Top();
        }
        break;
    default:
        // Ignore
        break;
    }
}


Thread _motor_thread;


int main()
{
    printf("Start\n");


    ble = &(BLE::Instance());
    ble->onEventsToProcess(scheduleBleEventsProcessing);
    ble->init(bleInitComplete);

    hallSensorIn.mode(PullUp);
    hallSensorIn.fall(&handleHallSensorFall);

    SET_STATE(Going2Top);
    _motor_thread.start(motor_thread);
    //printf(" Motor thread started\n");

    //ds.write_up_position(500);
    eventQueue.dispatch_forever();

    return 0;
}
