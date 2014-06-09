/* This file is part of the Razor AHRS Firmware */

unsigned long next;
unsigned long last;
unsigned long duration = 10l;

float *tick_boundaries;
int n_tick_boundaries;

void init_tick_boundaries(){
  float deg = 0.0f;
  float d = 1.3f;
  float dd = d;

  int n = 0;
  while(deg < 180.0f){
    n++;
    deg += dd;
    dd += d;
  }
  n_tick_boundaries = n+1;
  tick_boundaries = (float*)malloc(sizeof(float)*n_tick_boundaries);
  float prev = 0.0f;
  float rd = TO_RAD(d);
  float rdd = rd;
  for (int i = 0; i < n; i++){
    tick_boundaries[i] = prev+rdd;
    rdd += rd;
    prev = tick_boundaries[i];
  }
}

void output_tick(){
  //todo: no consideration paid to 180d boundary here - fix it
  int space = 0;
  int sign = (yaw > 0) ? 1 : -1;
  float absOYaw = abs(oyaw);
  float absCYaw = abs(yaw);
  int oBound = -1;
  int cBound = -1;
  //where's the nearest lower tick bound?
  for (int i = 0; i < n_tick_boundaries; i++){
    if (tick_boundaries[i] > absOYaw){
      oBound = i;
      break;
    }
  }
  //Serial.print(oBound);Serial.print(" - ");Serial.print(n_tick_boundaries);Serial.print(" - ");
  //Serial.print(TO_DEG(tick_boundaries[oBound]));Serial.print("->");Serial.print(TO_DEG(tick_boundaries[oBound+1]));
  //Serial.print(" : ");Serial.print(TO_DEG(absCYaw)); Serial.print(" ---- "); Serial.println(TO_DEG(absOYaw));
  if (absCYaw < tick_boundaries[oBound - 1]){
    // we've ticked towards target
    analogWrite(9,220);
    delay(5);
    analogWrite(9,0);
  } else if (absCYaw > tick_boundaries[oBound]){
    if (oBound < 2){
      analogWrite(9,190);
      delay(5);
      analogWrite(9,0);
    }else{
      // we've ticked away from target
      analogWrite(9,150);
      delay(3);
      analogWrite(9,0);
    }
  }


}

void output_pulse(){
  return;
  float dir[3];
  float xzLen = cos(pitch);
  dir[0] = xzLen * cos(yaw);
  dir[1] = sin(pitch);
  dir[2] = xzLen * sin(-yaw);
  
  float target[3] = {1.0f,0.0f,0.0f};
  float diff = Vector_Dot_Product(dir,target);
  
  float intensity = 0.5f + (diff/2.0f);
  int wut = ((int)(intensity * 250.f));
  int del = 265 - wut;
  
  long time = millis();
  if (last + duration < time){
    analogWrite(9,0);
  }
  if (time > next){
    analogWrite(9,255);
    last = time;
    next = time + del;
  }
}

// Output angles: yaw, pitch, roll
void output_angles()
{
  if (output_format == OUTPUT__FORMAT_BINARY)
  {
    float ypr[3];  
    ypr[0] = TO_DEG(yaw);
    ypr[1] = TO_DEG(pitch);
    ypr[2] = TO_DEG(roll);
    Serial.write((byte*) ypr, 12);  // No new-line
  }
  else if (output_format == OUTPUT__FORMAT_TEXT)
  {
    Serial.print("#YPR=");
    Serial.print(TO_DEG(yaw)); Serial.print(",");
    Serial.print(TO_DEG(pitch)); Serial.print(",");
    Serial.print(TO_DEG(roll)); Serial.println();
  }
}

void output_calibration(int calibration_sensor)
{
  if (calibration_sensor == 0)  // Accelerometer
  {
    // Output MIN/MAX values
    Serial.print("accel x,y,z (min/max) = ");
    for (int i = 0; i < 3; i++) {
      if (accel[i] < accel_min[i]) accel_min[i] = accel[i];
      if (accel[i] > accel_max[i]) accel_max[i] = accel[i];
      Serial.print(accel_min[i]);
      Serial.print("/");
      Serial.print(accel_max[i]);
      if (i < 2) Serial.print("  ");
      else Serial.println();
    }
  }
  else if (calibration_sensor == 1)  // Magnetometer
  {
    // Output MIN/MAX values
    Serial.print("magn x,y,z (min/max) = ");
    for (int i = 0; i < 3; i++) {
      if (magnetom[i] < magnetom_min[i]) magnetom_min[i] = magnetom[i];
      if (magnetom[i] > magnetom_max[i]) magnetom_max[i] = magnetom[i];
      Serial.print(magnetom_min[i]);
      Serial.print("/");
      Serial.print(magnetom_max[i]);
      if (i < 2) Serial.print("  ");
      else Serial.println();
    }
  }
  else if (calibration_sensor == 2)  // Gyroscope
  {
    // Average gyro values
    for (int i = 0; i < 3; i++)
      gyro_average[i] += gyro[i];
    gyro_num_samples++;
      
    // Output current and averaged gyroscope values
    Serial.print("gyro x,y,z (current/average) = ");
    for (int i = 0; i < 3; i++) {
      Serial.print(gyro[i]);
      Serial.print("/");
      Serial.print(gyro_average[i] / (float) gyro_num_samples);
      if (i < 2) Serial.print("  ");
      else Serial.println();
    }
  }
}

void output_sensors_text(char raw_or_calibrated)
{
  Serial.print("#A-"); Serial.print(raw_or_calibrated); Serial.print('=');
  Serial.print(accel[0]); Serial.print(",");
  Serial.print(accel[1]); Serial.print(",");
  Serial.print(accel[2]); Serial.println();

  Serial.print("#M-"); Serial.print(raw_or_calibrated); Serial.print('=');
  Serial.print(magnetom[0]); Serial.print(",");
  Serial.print(magnetom[1]); Serial.print(",");
  Serial.print(magnetom[2]); Serial.println();

  Serial.print("#G-"); Serial.print(raw_or_calibrated); Serial.print('=');
  Serial.print(gyro[0]); Serial.print(",");
  Serial.print(gyro[1]); Serial.print(",");
  Serial.print(gyro[2]); Serial.println();
}

void output_sensors_binary()
{
  Serial.write((byte*) accel, 12);
  Serial.write((byte*) magnetom, 12);
  Serial.write((byte*) gyro, 12);
}

void output_sensors()
{
  if (output_mode == OUTPUT__MODE_SENSORS_RAW)
  {
    if (output_format == OUTPUT__FORMAT_BINARY)
      output_sensors_binary();
    else if (output_format == OUTPUT__FORMAT_TEXT)
      output_sensors_text('R');
  }
  else if (output_mode == OUTPUT__MODE_SENSORS_CALIB)
  {
    // Apply sensor calibration
    compensate_sensor_errors();
    
    if (output_format == OUTPUT__FORMAT_BINARY)
      output_sensors_binary();
    else if (output_format == OUTPUT__FORMAT_TEXT)
      output_sensors_text('C');
  }
  else if (output_mode == OUTPUT__MODE_SENSORS_BOTH)
  {
    if (output_format == OUTPUT__FORMAT_BINARY)
    {
      output_sensors_binary();
      compensate_sensor_errors();
      output_sensors_binary();
    }
    else if (output_format == OUTPUT__FORMAT_TEXT)
    {
      output_sensors_text('R');
      compensate_sensor_errors();
      output_sensors_text('C');
    }
  }
}

