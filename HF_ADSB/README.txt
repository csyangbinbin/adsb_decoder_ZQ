1. libhfdecoder.a 为解码ADS-B数据的静态库。
2. adsb_decoder_api.h 为静态库对应的函数头文件。如果你们要自己开发程序，使用这个头文件和上边的静态库即可。
   具体函数定义，参考我之前发给你们的windows版本的说明。

3.HF_ADSB_Decoder.cpp为解码数据XML发送的源码文件。
4. 在linux环境下输入make进行源码编译。

5.编译出的可执行程序名为 HF_ADSB_Decoder

6.运行程序输入 ./HF_ADSB_Decoder IP Port  , 其中两个参数为接收XML数据的接收端的IP和端口。
例如，要将XML数据发送到10.1.1.130的21560 端口，输入 ./HF_ADSB_Decoder 10.1.1.130 21560