#!/bin/bash
sudo apt-get update
sudo apt-get upgrade
sudo rpi-update
sudo apt-get install screen
sudo raspi-config --expand-rootfs
sudo echo "dtparam=i2c_arm=on" >> /boot/config.txt
sudo echo "gpu_mem=384" >> /boot/config.txt
sudo echo "start_x=1" >> /boot/config.txt
sudo reboot
