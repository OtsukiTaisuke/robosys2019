# robosys2019

実装したこと
デバイスファイルに書き込んだ数字でLEDを点滅させる。  
  ０の場合１０回の場合、LEDが点滅後、消灯。
  １の場合１０回の場合、LEDが点滅後、点灯。
  数字以外の文字が書き込まれた場合は動作しない。  　　

# 実行方法
raspberrypi B の　GPIO２５にLEDをつなぐ
 ```bash
git clone https://github.com/OtsukiTaisuke/robosys2019.git
make
sudo insmod kadai.ko
sudo chmod 666 /dev/myled0
echo 1 > /dev/myled0
echo 0 > /dev/myled0
sudo rmmod kadai.ko
```
# demo video
https://youtu.be/cgRfnZJb3D8
