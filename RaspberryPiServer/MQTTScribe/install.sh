#!/bin/sh

echo "Installing MQTTScribe.py as a system service"

echo "Stopping old scribe"
systemctl stop MQTTScribe

echo "Making bin dir"
mkdir -p /usr/local/skimon/bin

echo "Copying new script"
cp ./MQTTScribe.py /usr/local/skimon/bin/

echo "Copying new service file"
cp ./MQTTScribe.service /lib/systemd/system

echo "Enabling MQTTScribe on boot"
systemctl enable MQTTScribe

echo "Starting MQTTScribe with new files"
systemctl start MQTTScribe

echo "Installed"