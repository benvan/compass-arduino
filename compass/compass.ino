#include <Wire.h>

#include <HMC5883L.h> 
int input = 0;
int f = 0;
float summ = 0;
float m_avg = 6.0;
float m_avg_factor = ((float)m_avg)/((float)m_avg+1.0);
const float limit = 4000.0; //yeah, I know. whatever
float range[] = {0.0,0.0,0.0,0.0,0.0,0.0};
float vals[] = {0.0, 0.0, 0.0};
int axes[] = {0,0,0};

// Add this outside the methods as a global variable.
HMC5883L compass;

void setup(){
  Wire.begin();
  pinMode(8,OUTPUT);
  pinMode(A1,INPUT);
  Serial.begin(9600);
  

  compass = HMC5883L();
  int error = compass.SetScale(1.3f); // Set the scale of the compass.
  if(error != 0) // If there is an error, print it out.
    Serial.println(compass.GetErrorText(error));

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

int get_val(int axisIndex){  
  float v = vals[axisIndex];
  axisIndex <<= 1;
  return (int) map((long) (v / m_avg), (range[axisIndex]/m_avg), (range[axisIndex+1]/m_avg), -100, 100);
}
  
void setup_axes(){
  MagnetometerRaw raw = compass.ReadRawAxis();
  axes[0] = raw.XAxis;
  axes[1] = raw.YAxis;
  axes[2] = raw.ZAxis;
}


void update_vals(){
  int i = 3;
  while (i --> 0){
    vals[i] += axes[i];
    vals[i] *= m_avg_factor;
  }
}

void output(){
   Serial.print(vals[0] / m_avg);  
   Serial.print(",");
   Serial.print(vals[1] / m_avg);
   Serial.print(",");
   float val = vals[2] / m_avg;
   Serial.println(val);
   if (val > 2000.0){
     int i = 5;
     while(i --> 0){
       Serial.print("o");
     }
     Serial.print("h snap!   -   ");
     Serial.print(axes[0]);  
     Serial.print(",");
     Serial.print(axes[1]);
     Serial.print(",");
     Serial.println(axes[2]);
     delay(1000);
   }
   
}

void loop()
{

  setup_axes();
  
  update_vals();

//  update_bounds();  

  output();

  delay(1);

}

void update_bounds(){

  for (int i = 0; i < 6; i+=2){
      float v = axes[i]*m_avg;
      if (v < range[i]){
        range[i] += v;
        range[i] *= m_avg_factor;
      }
      if (v > range[i+1]){
        range[i+1] += v; 
        range[i+1] *= m_avg_factor;
      }     
  }
} 


