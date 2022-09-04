#include <ConfigFile.h>
#include <Gpio.h>
#include <Hardware.h>
#include <Log.h>
#include <limero.h>
#include <stdio.h>
#include <unistd.h>

#include "Redis.h"

Log logger;

std::string loadFile(const char *name);
bool loadConfig(JsonObject cfg, int argc, char **argv);

/*
+-----+-----+---------+------+---+---Pi 3B--+---+------+---------+-----+-----+
 | BCM | wPi |   Name  | Mode | V | Physical | V | Mode | Name    | wPi | BCM |
 +-----+-----+---------+------+---+----++----+---+------+---------+-----+-----+
 |     |     |    3.3v |      |   |  1 || 2  |   |      | 5v      |     |     |
 |   2 |   8 |   SDA.1 |  OUT | 1 |  3 || 4  |   |      | 5v      |     |     |
 |   3 |   9 |   SCL.1 |   IN | 1 |  5 || 6  |   |      | 0v      |     |     |
 |   4 |   7 | GPIO. 7 |   IN | 1 |  7 || 8  | 0 | IN   | TxD     | 15  | 14  |
 |     |     |      0v |      |   |  9 || 10 | 1 | IN   | RxD     | 16  | 15  |
 |  17 |   0 | GPIO. 0 |   IN | 0 | 11 || 12 | 0 | IN   | GPIO. 1 | 1   | 18  |
 |  27 |   2 | GPIO. 2 |  OUT | 1 | 13 || 14 |   |      | 0v      |     |     |
 |  22 |   3 | GPIO. 3 |  OUT | 0 | 15 || 16 | 0 | IN   | GPIO. 4 | 4   | 23  |
 |     |     |    3.3v |      |   | 17 || 18 | 0 | IN   | GPIO. 5 | 5   | 24  |
 |  10 |  12 |    MOSI |   IN | 0 | 19 || 20 |   |      | 0v      |     |     |
 |   9 |  13 |    MISO |   IN | 0 | 21 || 22 | 1 | IN   | GPIO. 6 | 6   | 25  |
 |  11 |  14 |    SCLK | ALT0 | 0 | 23 || 24 | 1 | OUT  | CE0     | 10  | 8   |
 |     |     |      0v |      |   | 25 || 26 | 1 | OUT  | CE1     | 11  | 7   |
 |   0 |  30 |   SDA.0 |   IN | 1 | 27 || 28 | 1 | IN   | SCL.0   | 31  | 1   |
 |   5 |  21 | GPIO.21 |   IN | 1 | 29 || 30 |   |      | 0v      |     |     |
 |   6 |  22 | GPIO.22 |  OUT | 0 | 31 || 32 | 0 | OUT  | GPIO.26 | 26  | 12  |
 |  13 |  23 | GPIO.23 |   IN | 0 | 33 || 34 |   |      | 0v      |     |     |
 |  19 |  24 | GPIO.24 |   IN | 0 | 35 || 36 | 0 | IN   | GPIO.27 | 27  | 16  |
 |  26 |  25 | GPIO.25 |  OUT | 1 | 37 || 38 | 0 | IN   | GPIO.28 | 28  | 20  |
 |     |     |      0v |      |   | 39 || 40 | 0 | IN   | GPIO.29 | 29  | 21  |
 +-----+-----+---------+------+---+----++----+---+------+---------+-----+-----+
 | BCM | wPi |   Name  | Mode | V | Physical | V | Mode | Name    | wPi | BCM |
 +-----+-----+---------+------+---+---Pi 3B--+---+------+---------+-----+-----+
*/

// uint8_t gpioRaspberry[] = {4, 9, 10, 17, 18, 22, 23, 24, 25, 27}; //
// RAspberry Pi 1
// uint8_t gpioRaspberry[] = {0,1,2,3,4,5,6,7,21,22,23,24,25,26,27,28,29}; //
// wiringPi convention

int main(int argc, char **argv)
{
  INFO(" gpio2redis started. Build : %s ", __DATE__ " " __TIME__);

  INFO("Loading configuration.");
  Json config;
  config["redis"]["host"] = "localhost";
  config["redis"]["port"] = 6379;
  configurator(config, argc, argv);
  Thread workerThread("worker");
  Sys::init();
  Gpio::init();
  Redis redis(workerThread, config["redis"].as<JsonObject>());
  TimerSource timer(workerThread, 1000,true);

  redis.connect();

  Json helloCmd(1024);
  helloCmd[0] = "hello";
  helloCmd[1] = "3";
  redis.request().on(helloCmd);
  std::string srcPrefix = "src/raspi/";
  std::string dstPrefix = "dst/raspi/";

  timer >> [&](const TimerMsg &)
  {
    redis.publish(srcPrefix + "system/alive", "true");
  };

  for (uint32_t i = 0; i < Gpio::raspberryGpio.size(); i++)
  {
    uint32_t gpioIdx = Gpio::raspberryGpio[i];
    Gpio *gpio = new Gpio(workerThread, gpioIdx);
    std::string gpioValue = "gpio" + std::to_string(gpioIdx) + "/value";
    std::string gpioMode = "gpio" + std::to_string(gpioIdx) + "/mode";

    redis.subscriber<int>((dstPrefix + gpioValue).c_str()) >> gpio->value;
    redis.subscriber<std::string>((dstPrefix + gpioMode).c_str()) >> gpio->mode;

    gpio->value >> Cache<int>::nw(workerThread, 100, 1000) >>
        redis.publisher<int>((srcPrefix + gpioValue).c_str());

    gpio->mode >> Cache<std::string>::nw(workerThread, 100, 10000) >>
        redis.publisher<std::string>((srcPrefix + gpioMode).c_str());

    JsonArray gpioConfig = config["gpio"][std::to_string(gpioIdx)];
    if (gpioConfig)
    {
      gpio->mode.on(gpioConfig[0]);
      gpio->value.on(gpioConfig[1]);
    }
    else
    {
      gpio->mode.on("INPUT");
    }
  }

  workerThread.run();
}
