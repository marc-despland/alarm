# Capturing Image

Using Raspicam API : https://sourceforge.net/projects/raspicam/files/?

```
tar xvzf raspicamxx.tgz
cd raspicamxx
mkdir build
cd build
cmake .. 
make
sudo make install
```

Save in PNG : http://zarb.org/~gc/html/libpng.html
Save in JPEG : https://github.com/LuaDist/libjpeg/blob/master/example.c

To use raspicam_still I modify private_still_impl.c
line 360 : sem_t * mutex=(sem_t *) malloc(sizeof(sem_t));
and changing reference to mutex

```
gcc raspicam_test.cpp -I ../src -o rpicamtest -lraspicam -lpng -lstdc++ -ljpeg
```