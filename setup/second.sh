##opencv start
echo "**************Start installing OpenCV Prerequisites**************"
sudo apt-get -y install build-essential git cmake pkg-config
sudo apt-get -y install libjpeg-dev libtiff5-dev libjasper-dev libpng12-dev
sudo apt-get -y install libavcodec-dev libavformat-dev libswscale-dev libv4l-dev
sudo apt-get -y install libxvidcore-dev libx264-dev
sudo apt-get -y install libgtk2.0-dev
sudo apt-get -y install libatlas-base-dev gfortran
sudo apt-get -y install python2.7-dev python3-dev
echo "**************Cloning OpenCV and OpenCV_contrib**************"
cd ~
mkdir opencv
cd opencv
git clone https://github.com/Itseez/opencv.git
git clone https://github.com/Itseez/opencv_contrib.git

#wget -O opencv.zip https://github.com/Itseez/opencv/archive/3.0.0.zip
#unzip opencv.zip
#wget -O opencv_contrib.zip https://github.com/Itseez/opencv_contrib/archive/3.0.0.zip
#unzip opencv_contrib.zip
echo "**************cmake OpenCV**************"
cd opencv
mkdir build
cd build
cmake -D CMAKE_BUILD_TYPE=RELEASE -D CMAKE_INSTALL_PREFIX=/usr/local -D INSTALL_C_EXAMPLES=OFF -D INSTALL_PYTHON_EXAMPLES=ON -D OPENCV_EXTRA_MODULES_PATH=~/opencv_contrib-3.0.0/modules -D BUILD_EXAMPLES=ON ..
echo "**************build OpenCV**************"
make -j4
sudo make install
sudo ldconfig
##opencv end

echo "**************install wiringPi**************"
mkdir wiringPi
cd wiringPi
git clone git://git.drogon.net/wiringPi
cd wiringPi
./build

echo "**************install pico UPS**************"
sudo apt-get -y install python-rpi.gpio
sudo apt-get -y install python-smbus
sudo apt-get -y install i2c-tools
sudo sh -c "echo "i2c-bmc2708" >> /etc/modules"
mkdir ups
cd ups
wget http://www.pimodules.com/_zip/UPS_PIco_Supporting_Files.zip
unzip UPS_PIco_Supporting_Files.zip
#sudo mv rc.local /etc/
echo "sudo python /home/pi/picofssd.py &" >> /etc/rc.local
echo "echo ds1307 0x68 > /sys/class/i2c-adapter/i2c-1/new_device( sleep 4; hwclock -s ) &" >> /etc/rc.local
sudo chmod +x /etc/rc.local
echo "**************Done**************"
#sudo reboot
