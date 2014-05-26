/* This file is part of the Razor AHRS Firmware */

// I2C code to read the sensors

// Sensor I2C addresses
#define ACCEL_ADDRESS ((int) 0x53) // 0x53 = 0xA6 / 2
#define MAGN_ADDRESS  ((int) 0x1E) // 0x1E = 0x3C / 2
#define GYRO_ADDRESS  ((int) 0x69)

// Arduino backward compatibility macros
#if ARDUINO >= 100
  #define WIRE_SEND(b) Wire.write((byte) b) 
  #define WIRE_RECEIVE() Wire.read() 
#else
  #define WIRE_SEND(b) Wire.send(b)
  #define WIRE_RECEIVE() Wire.receive() 
#endif


void I2C_Init()
{
  Wire.begin();
}

void Accel_Init()
{
  Wire.beginTransmission(ACCEL_ADDRESS);
  WIRE_SEND(0x2D);  // Power register
  WIRE_SEND(0x08);  // Measurement mode
  Wire.endTransmission();
  delay(5);
  Wire.beginTransmission(ACCEL_ADDRESS);
  WIRE_SEND(0x31);  // Data format register
  WIRE_SEND(0x08);  // Set to full resolution
  Wire.endTransmission();
  delay(5);
  
  // Because our main loop runs at 50Hz we adjust the output data rate to 50Hz (25Hz bandwidth)
  Wire.beginTransmission(ACCEL_ADDRESS);
  WIRE_SEND(0x2C);  // Rate
  WIRE_SEND(0x09);  // Set to 50Hz, normal operation
  Wire.endTransmission();
  delay(5);
}

// Reads x, y and z accelerometer registers
void Read_Accel()
{
  int i = 0;
  byte buff[6];
  
  Wire.beginTransmission(ACCEL_ADDRESS); 
  WIRE_SEND(0x32);  // Send address to read from
  Wire.endTransmission();
  
  Wire.beginTransmission(ACCEL_ADDRESS);
  Wire.requestFrom(ACCEL_ADDRESS, 6);  // Request 6 bytes
  while(Wire.available())  // ((Wire.available())&&(i<6))
  { 
    buff[i] = WIRE_RECEIVE();  // Read one byte
    i++;
  }
  Wire.endTransmission();
  
  if (i == 6)  // All bytes received?
  {
    // No multiply by -1 for coordinate system transformation here, because of double negation:
    // We want the gravity vector, which is negated acceleration vector.
//    accel[0] = ((((int) buff[3]) << 8) | buff[2]);  // X axis (internal sensor y axis)
//    accel[1] = ((((int) buff[1]) << 8) | buff[0]);  // Y axis (internal sensor x axis)
//    accel[2] = (((int) buff[5]) << 8) | buff[4];  // Z axis (internal sensor z axis)
//    
    accel[1] = ((((int) buff[1]) << 8) | buff[0]);
    accel[0] = ((((int) buff[3]) << 8) | buff[2]);
    accel[2] = (((int) buff[5]) << 8) | buff[4];
  }
  else
  {
    num_accel_errors++;
    if (output_errors) Serial.println("!ERR: reading accelerometer");
  }
}

void Magn_Init()
{
  Wire.beginTransmission(MAGN_ADDRESS);
  WIRE_SEND(0x02); 
  WIRE_SEND(0x00);  // Set continuous mode (default 10Hz)
  Wire.endTransmission();
  delay(5);

  Wire.beginTransmission(MAGN_ADDRESS);
  WIRE_SEND(0x00);
  WIRE_SEND(0b00011000);  // Set 50Hz
  Wire.endTransmission();
  delay(5);
}

void Read_Magn()
{
  int i = 0;
  byte buff[6];
 
  Wire.beginTransmission(MAGN_ADDRESS); 
  WIRE_SEND(0x03);  // Send address to read from
  Wire.endTransmission();
  
  Wire.beginTransmission(MAGN_ADDRESS); 
  Wire.requestFrom(MAGN_ADDRESS, 6);  // Request 6 bytes
  while(Wire.available())  // ((Wire.available())&&(i<6))
  { 
    buff[i] = WIRE_RECEIVE();  // Read one byte
    i++;
  }
  Wire.endTransmission();
  
  if (i == 6)  // All bytes received?
  {
    
//    magnetom[1] = (((int) buff[0]) << 8) | buff[1];         // X axis (internal sensor x axis) -- negative y
//    magnetom[0] = ((((int) buff[2]) << 8) | buff[3]);  // Y axis (internal sensor -y axis) a
//    magnetom[2] = ((((int) buff[4]) << 8) | buff[5]);  // Z axis (internal sensor -z axis)
    
    // MSB byte first, then LSB; Y and Z reversed: X, Z, Y
    magnetom[1] = (((int) buff[0]) << 8) | buff[1];         // X axis (internal sensor x axis)
    magnetom[0] = ((((int) buff[4]) << 8) | buff[5]);  // Y axis (internal sensor -y axis)
    magnetom[2] = ((((int) buff[2]) << 8) | buff[3]);  // Z axis (internal sensor -z axis)
  }
  else
  {
    num_magn_errors++;
    if (output_errors) Serial.println("!ERR: reading magnetometer");
  }
}

void writeRegister(int deviceAddress, byte address, byte val) {
    Wire.beginTransmission(deviceAddress); // start transmission to device 
    Wire.write(address);       // send register address
    Wire.write(val);         // send value to write
    Wire.endTransmission();     // end transmission
}

void Gyro_Init()
{
  // reg1: 0000 - Data rate 100Hz, Cut-off 12.5
  // reg1: 1 - power down mode off, 111 - enable X,Y,Z readings
  writeRegister(GYRO_ADDRESS,0x20,0b00001111);

  // reg2: high pass normal mode, cutoff 8hz (?? wtf)
  writeRegister(GYRO_ADDRESS, 0x21,0b00000000);
 
  // reg3: data ready interrupt on INT2 - I have no idea why :| .. find out
  writeRegister(GYRO_ADDRESS, 0x22, 0b00001000);
  
  // reg4: 0 - continuous update, 0 - Big Endian,
  // x - ignored
  // 11 - full resolution, 00 - self test disabled
  // 0 - 4-wire SPI interface. Don't think this matters
  writeRegister(GYRO_ADDRESS, 0x23, 0b00110000);
  
  // reg5: high pass filter disabled
  writeRegister(GYRO_ADDRESS, 0x24, 0b00000000);

}

int readRegister(int deviceAddress, byte address){

    int v;
    Wire.beginTransmission(deviceAddress);
    Wire.write(address); // register to read
    Wire.endTransmission();

    Wire.requestFrom(deviceAddress, 1); // read a byte

    while(!Wire.available()) {
        // waiting
    }

    v = Wire.read();
    return v;
}

// Reads x, y and z gyroscope registers
void Read_Gyro()
{
  
//  gyro[0] = -1 * ((((int) buff[2]) << 8) | buff[3]);    // X axis (internal sensor -y axis)
//  gyro[1] = -1 * ((((int) buff[0]) << 8) | buff[1]);    // Y axis (internal sensor -x axis)
//  gyro[2] = -1 * ((((int) buff[4]) << 8) | buff[5]);    // Z axis (internal sensor -z axis)
  
  // Get instantaneous roll, pitch and yaw values from gyro
  byte rollGyroValMSB = readRegister(GYRO_ADDRESS, 0x29);
  byte rollGyroValLSB = readRegister(GYRO_ADDRESS, 0x28);

  byte pitchGyroValMSB = readRegister(GYRO_ADDRESS, 0x2B);
  byte pitchGyroValLSB = readRegister(GYRO_ADDRESS, 0x2A);

  byte yawGyroValMSB = readRegister(GYRO_ADDRESS, 0x2D);
  byte yawGyroValLSB = readRegister(GYRO_ADDRESS, 0x2C);

  gyro[1] = -1 * ((((int)rollGyroValMSB) << 8) | rollGyroValLSB);
  gyro[0] = -1 * ((((int)pitchGyroValMSB) << 8) | pitchGyroValLSB);
  gyro[2] = -1 * ((((int)yawGyroValMSB) << 8) | yawGyroValLSB);
  
}
