#!/bin/sh

echo "Installing StatusDaemon.py as a system service"

echo "Stopping old daemon"
systemctl stop StatusDaemon

echo "Making bin dir"
mkdir -p /usr/local/skimon/bin

echo "Copying new script"
cp ./StatusDaemon.py /usr/local/skimon/bin/

echo "Copying new service file"
cp ./StatusDaemon.service /lib/systemd/system

echo "Enabling Status Daemon on boot"
systemctl enable StatusDaemon

echo "Starting Status Daemon with new files"
systemctl start StatusDaemon

echo "Installed"