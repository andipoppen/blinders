#ifndef __BLE_BLINDER_SERVICE_H__
#define __BLE_BLINDER_SERVICE_H__

class BlinderService {
public:
    const static uint16_t BLINDER_SERVICE_UUID              = 0xA000;
    const static uint16_t BLINDER_CMD_CHARACTERISTIC_UUID   = 0xA001;
    const static uint16_t BLINDER_POS_CHARACTERISTIC_UUID   = 0xA002;
    
    const static uint16_t BLINDER_CMD_UP =              0;
    const static uint16_t BLINDER_CMD_DOWN =            1;
    const static uint16_t BLINDER_CMD_TOGGLE =          2;
    const static uint16_t BLINDER_CMD_UP_CALIBRATE =    3;
    const static uint16_t BLINDER_CMD_DOWN_CALIBRATE =  4;
    const static uint16_t BLINDER_CMD_SET_UP_POS =      5;
    const static uint16_t BLINDER_CMD_SET_BOTTOM_POS =  6;

    BlinderService(BLEDevice &_ble, uint8_t initialValueForBlinderCharacteristicCmd, uint32_t initialValueForBlinderCharacteristicPos)
        :
        ble(_ble), 
        blinderCmd(BLINDER_CMD_CHARACTERISTIC_UUID, &initialValueForBlinderCharacteristicCmd),
        blinderPos(BLINDER_POS_CHARACTERISTIC_UUID, &initialValueForBlinderCharacteristicPos)
        
    {
        GattCharacteristic *charTable[] = { &blinderCmd, &blinderPos };
        GattService         blinderService(BLINDER_SERVICE_UUID, charTable, sizeof(charTable) / sizeof(GattCharacteristic *));
        ble.addService(blinderService);
    }

    GattAttribute::Handle_t getValueHandleCmd() const
    {
        return blinderCmd.getValueHandle();
    }

    GattAttribute::Handle_t getValueHandlePos() const
    {
        return blinderPos.getValueHandle();
    }
    
private:
    BLEDevice                         &ble;
    WriteOnlyGattCharacteristic<uint8_t> blinderCmd;
    ReadOnlyGattCharacteristic<uint32_t> blinderPos;
};

#endif /* #ifndef __BLE_BLINDER_SERVICE_H__ */
