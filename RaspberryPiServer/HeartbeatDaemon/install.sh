#!/bin/sh

echo "Installing HeartbeatDaemon.py as a system service"

echo "Stopping old deamon"
systemctl stop HeartbeatDaemon

echo "Making bin dir"
mkdir -p /usr/local/skimon/bin

echo "Copying new script"
cp ./HeartbeatDaemon.py /usr/local/skimon/bin/

echo "Copying new service file"
cp ./HeartbeatDaemon.service /lib/systemd/system

echo "Enabling Heartbeat Daemon on boot"
systemctl enable HeartbeatDaemon

echo "Starting Heartbeat Daemon with new files"
systemctl start HeartbeatDaemon

echo "Installed"