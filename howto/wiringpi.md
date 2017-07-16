# Test connecting relais on RPI

I use wiringPI : http://wiringpi.com/download-and-install/

```
git clone git://git.drogon.net/wiringPi
./build
```

```
gcc pinon.c -o pinon -lwiringPi
gcc pinoff.c -o pinoff -lwiringPi

```



#Faire fonctionner le DHT22 avec RPI

http://www.uugear.com/portfolio/read-dht1122-temperature-humidity-sensor-from-raspberry-pi/

```
gcc dht.c -o dht -lwiringPi
```