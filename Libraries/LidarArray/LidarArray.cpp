#include <LidarArray.h>
#include <Arduino.h>
#include <Wire.h>
//#include <TFLI2C.h> // TFLuna Library


//LidarArray::LidarArray(){}

LidarArray::LidarArray(int numSensors, uint8_t *addresses, int16_t *distances) {
	_numSensors = numSensors;
	_addresses = addresses;
	_distances = distances;
}

uint8_t LidarArray::readDistances() {
	return(_addresses[1]);
	//for(int i = 0; i < _numSensors; i++) {
	 //	_distances[i] = readSensor(_addresses[i]);
	 //}
}

// Adapted from TFLI2C library by Bud Ryerson
// https://www.arduino.cc/reference/en/libraries/tfli2c/
uint8_t LidarArray::readSensor(uint8_t addr) {
    tfStatus = 0;    // clear status of any error condition

    // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
    // Step 1 - Use the function 'regCommand' to fill the table 'frame'
    // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
    for (uint8_t reg = 0x00; reg <= 0x05; reg++)
    {
      if( !readReg( reg, addr)) return false;
          else dataArray[ reg] = regReply;
    }

    // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
    // Step 2 - Interpret the frame data
    // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

    uint8_t dist = dataArray[0] + (dataArray[1] << 8);

    return addr;
}


// Adapted from TFLI2C library by Bud Ryerson
// https://www.arduino.cc/reference/en/libraries/tfli2c/
// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
//       READ OR WRITE A GIVEN REGISTER OF THE SLAVE DEVICE
// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
bool LidarArray::readReg( uint8_t nmbr, uint8_t addr)
{
	//return true;

  Wire.beginTransmission( addr);
  Wire.write( nmbr);

  // if( Wire.endTransmission() != 0)  // If write error...
  // {
  //   tfStatus = 1;        // then set satus code...
  //   return false;                   // and return "false."
  // }
  // Request 1 byte from the device
  // and release bus when finished.
  Wire.requestFrom( ( int)addr, 1);
    // if( Wire.peek() == -1)            // If read error...
    // {
    //   tfStatus = 1;         // then set satus code...
    //   return false;
    // }
  regReply = ( uint8_t)Wire.read(); // Read the recieved data...
  return true;
}
