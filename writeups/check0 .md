Checkpoint 0 Writeup
====================

My name: [Loa Wei Xin]

This lab took me about [3] hours to do. I [did] attend the lab session.

1. Program Structure and Design:

创建了一个TCPSocket，通过host和service --"http"构建Address，通过socket1向服务器发送http请求报文。利用while循环，一直输出从服务器里收到的所有内容。

code:
![alt text](35d9bd35f90ee83bf963f72b4df5639.png)

outcome1:
![alt text](534543570799f799511e3002c257c7c.png)

capacity表示最大容量，error_表示是否发生错误，close表示ByteStream是否关闭，buffer_是存储数据的地方，bytes_pushed_表示写入了多少数据，bytes_popped表示读取了多少数据.

code:
![alt text](67e070b8326f589f4b0f641bfb7290a.png)
![alt text](282300034a9cb60ed5bdc710afeb478.png)
![alt text](8e74b3807ce693cb6479662ac760da7.png)

outcome2:
![alt text](4cf553c327f84d5c404715adbe45cd8.png)

2. Implementation Challenges
在配置环境中出现了问题，导致实现这句命令行时"Cmake --build build"一直在报错，问了助教后发现是因为g++和gcc的版本太低，升级了以后就解决了。