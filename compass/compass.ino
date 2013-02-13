#include <Wire.h>

#include <HMC5883L.h>
int f = 0;
float summ = 0;
int m_avg = 6;
float m_avg_factor = ((float)m_avg)/(m_avg+1);

// Add this outside the methods as a global variable.
HMC5883L compass;

void setup(){
  Wire.begin();
  pinMode(8,OUTPUT);
  pinMode(A1,INPUT);
  Serial.begin(9600);

  compass = HMC5883L();
  Serial.println("Setting scale to +/- 1.3 Ga");
  int error = compass.SetScale(1.3f); // Set the scale of the compass.
  if(error != 0) // If there is an error, print it out.
    Serial.println(compass.GetErrorText(error));

  Serial.println("Setting measurement mode to continuous.");
  error = compass.SetMeasurementMode(Measurement_Continuous); // Set the measurement mode to Continuous
  if(error != 0) // If there is an error, print it out.
    Serial.println(compass.GetErrorText(error));
}

char* addNum(char* str, int x){
  bool neg = x < 0;
  x = neg ? -x : x;
  while (x) {
    *str = '0' + x % 10;
    x /= 10;
    --str;
  }
  if (neg) {
    *str = '-';
    --str;
  }
  return str;
}


void loop()
{
  // Retrive the raw values from the compass (not scaled).
  MagnetometerScaled raw = compass.ReadScaledAxis();

  int axis = raw.XAxis;

  // Retrived the scaled values from the compass (scaled to the configured scale).
  MagnetometerScaled scaled = compass.ReadScaledAxis();
  
  // Calculate heading when the magnetometer is level, then correct for signs of axis.
  float heading = atan2(raw.YAxis, raw.XAxis);
   
  // Correct for when signs are reversed.
  if(heading < 0)
    heading += 2*PI;
   
  // Convert radians to degrees for readability.
  float headingDegrees = heading * 180/M_PI; 

  // Output the data via the serial port.
//  Output(raw, scaled, heading, headingDegrees);
  
  int val = abs(scaled.XAxis * 4);// + abs(scaled.YAxis) + abs(scaled.ZAxis));

//  Serial.println(raw.XAxis);
  
  if (abs(axis) < (summ+2000)){
    
    summ += axis;
    summ *= m_avg_factor;
  }
  
  char out[30];
  out[29] = 0;
  char* str = addNum(&out[28],raw.ZAxis);
  *str = ',';
  --str;
  str = addNum(str,raw.YAxis);
  *str = ',';
  --str;
  str = addNum(str,raw.XAxis);

  
   Serial.println(++str);

  
//  int m = 10000;
//  int t = m - val;
  
//  int pitch = map((summ/m_avg), -600,600,0,2000);
  
  delay(1);

}

// Output the data down the serial port.
void Output(MagnetometerRaw raw, MagnetometerScaled scaled, float heading, float headingDegrees)
{
   Serial.print("Raw:\t");
   Serial.print(raw.XAxis);
   Serial.print("   ");   
   Serial.print(raw.YAxis);
   Serial.print("   ");   
   Serial.print(raw.ZAxis);
   Serial.print("   \tScaled:\t");
   
   Serial.print(scaled.XAxis);
   Serial.print("   ");   
   Serial.print(scaled.YAxis);
   Serial.print("   ");   
   Serial.print(scaled.ZAxis);

   Serial.print("   \tHeading:\t");
   Serial.print(heading);
   Serial.print(" Radians   \t");
   Serial.print(headingDegrees);
   Serial.println(" Degrees   \t");
}

