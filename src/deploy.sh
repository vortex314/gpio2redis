sudo cp gpio2redis.service /etc/systemd/system/
sudo systemctl daemon-reload
sudo systemctl restart gpio2redis
sudo systemctl status gpio2redis

