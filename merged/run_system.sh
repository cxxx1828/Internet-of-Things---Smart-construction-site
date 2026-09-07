#!/bin/bash

echo "[1/6] Mosquitto broker on"
mosquitto -d
sleep 2

echo "[2/6]  environment simulator on"
gnome-terminal --title="ENVIRONMENT" -- bash -c "./environment; exec bash"
sleep 1

echo "[3/6]  controller on"
gnome-terminal --title="CONTROLLER" -- bash -c "./controller_merged; exec bash"
sleep 1

echo "[4/6]  heart_rate sensor on"
gnome-terminal --title="HEART_RATE_SENSOR" -- bash -c "./heart_rate; exec bash"
sleep 1

echo "[5/6]  temperature sensor on"
gnome-terminal --title="TEMPERATURE_SENSOR" -- bash -c "./temperature; exec bash"
sleep 1

echo "[6/6]  actuators (emergency i shutdown) on"
gnome-terminal --title="EMERGENCY_ACTUATOR" -- bash -c "./emergency; exec bash"
gnome-terminal --title="SHUTDOWN_ACTUATOR" -- bash -c "./shutdown; exec bash"
sleep 1

echo "system is on"
