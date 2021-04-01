#!/bin/sh

echo "Installing GPSLogger.py as a system service"

echo "Stopping old GPS Logger"
systemctl stop GPSLogger

echo "Making bin dir"
mkdir -p /usr/local/skimon/bin

echo "Copying new script"
cp ./GPSLogger.py /usr/local/skimon/bin/

echo "Copying new service file"
cp ./GPSLogger.service /lib/systemd/system

echo "Enabling GPSLogger on boot"
systemctl enable GPSLogger

echo "Starting GPSLogger with new files"
systemctl start GPSLogger

echo "Installed"