#include <math.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>


unsigned short min_fan_speed;
unsigned short max_fan_speed;
unsigned short low_temp=35;
unsigned short high_temp=50;

/* temperature polling interval */
unsigned short polling_interval=15;


/* Controls the speed of the fan */
void set_fan_speed(unsigned short speed)
{
  FILE *file;

  file=fopen("/sys/devices/platform/applesmc.768/fan2_output", "w");
  fprintf(file, "%d", speed);
  fclose(file);
}


/* Takes "manual" control of fan and read max and min fan speed */
void prepare_fan(int argc)
{
  FILE *file;

  file=fopen("/sys/devices/platform/applesmc.768/fan2_manual", "w");
  fprintf(file, "%d", 1);
  fclose(file);

  file=fopen("/sys/devices/platform/applesmc.768/fan2_min","r");
  fscanf(file, "%hd", &min_fan_speed);
  fclose(file);

  file=fopen("/sys/devices/platform/applesmc.768/fan2_max","r");
  fscanf(file, "%hd", &max_fan_speed);
  fclose(file);

  if (argc > 1)
  {
    printf("min_fan_speed: %d\n", min_fan_speed);
    printf("max_fan_speed: %d\n", max_fan_speed);
  }
}


/* Returns temp in degrees */
unsigned short get_temp()
{
  FILE *fp;
  unsigned short temp;
  char path[1035];

  fp = popen("hddtemp /dev/sda -n", "r");
  fgets(path, sizeof(path)-1, fp);
  temp = (unsigned short)atoi(path);

  return temp;
}


int main(int argc, char *argv[])
{
  unsigned short old_temp, new_temp, fan_speed, steps;
  short temp_change;
  float step_up, step_down;

  unsigned short step;

  prepare_fan(argc);

  step = (unsigned short)(max_fan_speed - min_fan_speed)/
                         (high_temp-low_temp);

  if (argc > 1)
    printf("step: %d\n", step);

  while(1)
  {
    old_temp = new_temp;
    new_temp = get_temp();

    if (argc > 1)
      printf("new_temp: %d  ", new_temp);

    if (new_temp > high_temp)
    {
      fan_speed = max_fan_speed;
    }
    else if (new_temp <= low_temp)
    {
      fan_speed = min_fan_speed;
    }
    else
    {
      fan_speed = step * (new_temp - low_temp) + min_fan_speed;
    }

    set_fan_speed(fan_speed);

    if (argc > 1)
      printf("set_fan: %d\n", fan_speed);

    sleep(polling_interval);
  }

  return 0;
}
