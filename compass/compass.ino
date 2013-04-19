#include <Wire.h>

float m_avg = 3.0;
float m_avg_factor = ((float)m_avg)/((float)m_avg+1.0);
float vals[] = {0.0, 0.0, 0.0};
int axes[] = {0,0,0};

void setup()
{
  Serial.begin(9600);
  delay(100);

  pinMode(8,OUTPUT);
  pinMode(A1,INPUT);

  Wire.begin();
  delay(100);

  // read mode
  Wire.beginTransmission(0x1E);
  Wire.write(0x00);
  Wire.write(0x70);
  Wire.endTransmission();

  // guass 1.3
  Wire.beginTransmission(0x1E);
  Wire.write(0x01);
  Wire.write(0x20);
  Wire.endTransmission();

  // continuous measurement
  Wire.beginTransmission(0x1E);
  Wire.write(0x02);
  Wire.write(0x00);
  Wire.endTransmission();

  delay(8);
}

float get_val(int axisIndex){  
  float v = vals[axisIndex];
  //return v;
  return (v/m_avg);
}
  
void update_vals(){
  int i = 3;
  while (i --> 0){
    //should check here for huge / erroneous values
    vals[i] += axes[i];
    vals[i] *= m_avg_factor;
  }
}

void output(){
   float f;
   f = get_val(0);
   Serial.print(f);  
   Serial.print(",");
   f = get_val(1);
   Serial.print(f);  
   Serial.print(",");
   f = get_val(2);
   Serial.println(f);
}

void loop()
{

  Wire.requestFrom(0x1E, 6);

  int x = 0, y = 0, z = 0;

  if(Wire.available() == 6) {    // If 6 bytes available
    x = Wire.read() << 8 | Wire.read();
    y = Wire.read() << 8 | Wire.read();
    z = Wire.read() << 8 | Wire.read();
  }else{
    Serial.print(Wire.available());
    delay(2000);    
  }

  Wire.beginTransmission(0x1E);
  Wire.write(0x03);
  Wire.endTransmission();

  
  axes[0] = x;
  axes[1] = y;
  axes[2] = z;

  update_vals();
  
  output();

  delay(2);

  return;
  
  //update_vals();

  

  

}