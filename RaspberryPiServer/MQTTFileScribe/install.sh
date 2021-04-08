#!/bin/sh

echo "Installing MQTTFileScribe.py as a system service"

echo "Stopping old scribe"
systemctl stop MQTTFileScribe

echo "Making bin dir"
mkdir -p /usr/local/skimon/bin

echo "Copying new script"
cp ./MQTTFileScribe.py /usr/local/skimon/bin/

echo "Copying new service file"
cp ./MQTTFileScribe.service /lib/systemd/system

echo "Enabling MQTTFileScribe on boot"
systemctl enable MQTTFileScribe

echo "Starting MQTTFileScribe with new files"
systemctl start MQTTFileScribe

echo "Installed"