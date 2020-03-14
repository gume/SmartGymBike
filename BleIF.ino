#include <BLEDevice.h>
#include <BLEUtils.h>
#include <BLEServer.h>
#include <BLE2902.h>

//https://os.mbed.com/users/tenfoot/code/BLE_CycleSpeedCadence/file/7e334e81da21/CyclingSpeedAndCadenceService.h/

bool BleIF_deviceConnected;

class GattService {
public:
  enum {
     UUID_CYCLING_SPEED_AND_CADENCE = 0x1816
  };
};

class GattCharacteristic {
public:
  enum {
    UUID_CSC_MEASUREMENT_CHAR = 0x2A5B,
    UUID_CSC_FEATURE_CHAR = 0x2A5C,
    UUID_SENSOR_LOCATION_CHAR = 0x2A5D,
    UUID_SC_CONTROL_POINT_CHAR = 0x2A55
  };
};

class BleIF_ServerCallbacks: public BLEServerCallbacks {
    void onConnect(BLEServer* pServer) {
      BleIF_deviceConnected = true;
    };

    void onDisconnect(BLEServer* pServer) {
      BleIF_deviceConnected = false;
    }
};

BLECharacteristic *pCsc, *pCscFeat, *pScLocation, *pControlPoint;

void BleIF_setup() {
  BLEDevice::init("Szmardzsimbájk");
  BLEServer *pServer = BLEDevice::createServer();
  pServer->setCallbacks(new BleIF_ServerCallbacks());

  BLEService *pService = pServer->createService((uint16_t)GattService::UUID_CYCLING_SPEED_AND_CADENCE);
  BLECharacteristic *pCsc = pService->createCharacteristic(
                 (uint16_t)GattCharacteristic::UUID_CSC_MEASUREMENT_CHAR,
                 BLECharacteristic::PROPERTY_READ | BLECharacteristic::PROPERTY_NOTIFY);
  pCsc->addDescriptor(new BLE2902());
  BLECharacteristic *pCscFeat = pService->createCharacteristic(
                 (uint16_t)GattCharacteristic::UUID_CSC_FEATURE_CHAR,
                 BLECharacteristic::PROPERTY_READ);
  uint16_t vf = 2;  // Crank counter only
  pCscFeat->setValue((uint8_t*)&vf, 2);
  BLECharacteristic *pScLocation = pService->createCharacteristic(
                 (uint16_t)GattCharacteristic::UUID_SENSOR_LOCATION_CHAR,
                 BLECharacteristic::PROPERTY_READ);
  uint16_t vl = 6;  // LOCATION_RIGHT_CRANK
  pScLocation->setValue((uint8_t*)&vl, 1);
  BLECharacteristic *pControlPoint = pService->createCharacteristic(
                 (uint16_t)GattCharacteristic::UUID_SC_CONTROL_POINT_CHAR,
                 BLECharacteristic::PROPERTY_WRITE);
  pService->start();               

  BLEAdvertising *pAdvertising = BLEDevice::getAdvertising();
  pAdvertising->addServiceUUID((uint16_t)GattService::UUID_CYCLING_SPEED_AND_CADENCE);
  pAdvertising->setScanResponse(true);
  pAdvertising->setMinPreferred(0x0);  // set value to 0x00 to not advertise this parameter
  BLEDevice::startAdvertising();
}

void BleIF_update(uint16_t revs) {

  uint16_t v[2];
  v[0] = revs;
  v[1] = (uint16_t) millis();
  
  pCsc->setValue((uint8_t*)&v, 4);
  pCsc->notify();
}
