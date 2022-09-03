sudo cp gpio2mqtt.service /etc/systemd/system/
sudo systemctl daemon-reload
sudo systemctl restart gpio2mqtt
sudo systemctl status gpio2mqtt

