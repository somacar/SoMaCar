# 통신 제어

## master 아두이노 <-> slave 아두이노 통신
> 마스터 아두이노와 slave 아두이노와 통신을 하는 부분이며, 두 아두이노 중 하나가 전기 신호를 주고 받는 쪽 핀에서 인식을 하면 동작을 수행하는 부분입니다. (이미지 추가 예정)
##### 해당 폴더
- CommunicationSend
- CommunicationRecv

## Raspberry Pi -> Arduino 통신 (시리얼)
> 아두이노에서 라즈베리파이로 부터 시리얼 통신을 통해 받은 값을 아두이노에서 인식하는 코드 입니다.
##### 해당 폴더
- CommunicationSerialRecv
- RPiArduinoCommunication

## 트래킹 시 사용 되는 파일 
>
- CommunicationSerialRecv/for-tracking/GPIO_Receiver.py - GPIO 4번 핀 값을 읽어들여 OpenCV On/Off 기능을 담당하는 파일
- CommunicationSerialRecv/for-tracking/Sender.ino - 라즈베리파이와 아두이노와 시리얼 통신을 하여 모터 담당 아두이노 쪽으로 신호 전달


## 아두이노 <-> RPI I2C 통신 (deprecated)
> 핀 연결을 통해 I2C 방식으로 통신을 할 수 있습니다.
##### 해당 폴더
- CommunicationI2C


## Raspberry Pi, Arduino Wi-Fi AP Switch (deprecated)
[해당 문서로 이동](https://github.com/somacar/somacar/tree/rainc/communication_control/arduino-raspberry-wifi-connect)
