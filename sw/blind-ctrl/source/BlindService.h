#ifndef __BLE_BLIND_SERVICE_H__
#define __BLE_BLIND_SERVICE_H__

class BlindService {
public:
    const static uint16_t BLIND_SERVICE_UUID              = 0xA000;
    const static uint16_t BLIND_CMD_CHARACTERISTIC_UUID   = 0xA001;
    const static uint16_t BLIND_POS_CHARACTERISTIC_UUID   = 0xA002;
    
    const static uint16_t BLIND_CMD_UP =              0;
    const static uint16_t BLIND_CMD_DOWN =            1;
    const static uint16_t BLIND_CMD_TOGGLE =          2;
    const static uint16_t BLIND_CMD_UP_CALIBRATE =    3;
    const static uint16_t BLIND_CMD_DOWN_CALIBRATE =  4;
    const static uint16_t BLIND_CMD_SET_UP_POS =      5;
    const static uint16_t BLIND_CMD_SET_BOTTOM_POS =  6;

    BlindService(BLEDevice &_ble, uint8_t initialValueForBlindCharacteristicCmd, uint32_t initialValueForBlindCharacteristicPos)
        :
        ble(_ble), 
        blindCmd(BLIND_CMD_CHARACTERISTIC_UUID, &initialValueForBlindCharacteristicCmd),
        blindPos(BLIND_POS_CHARACTERISTIC_UUID, &initialValueForBlindCharacteristicPos)
        
    {
        GattCharacteristic *charTable[] = { &blindCmd, &blindPos };
        GattService         blindService(BLIND_SERVICE_UUID, charTable, sizeof(charTable) / sizeof(GattCharacteristic *));
        ble.addService(blindService);
    }

    GattAttribute::Handle_t getValueHandleCmd() const
    {
        return blindCmd.getValueHandle();
    }

    GattAttribute::Handle_t getValueHandlePos() const
    {
        return blindPos.getValueHandle();
    }
    
private:
    BLEDevice                         &ble;
    WriteOnlyGattCharacteristic<uint8_t> blindCmd;
    ReadOnlyGattCharacteristic<uint32_t> blindPos;
};

#endif /* #ifndef __BLE_BLIND_SERVICE_H__ */
