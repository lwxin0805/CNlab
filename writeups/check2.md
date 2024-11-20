Checkpoint 2 Writeup
====================

My name: 赖炜欣

My student ID : 205220026

I did attend the lab session.
这个实验修改了wrapping_integers和tcp_receiver的文件。

wrapping_integers的主要功能是32位序列号与64位序列号之间的转换。

![alt text](image-14.png)
计算n的低32位+zero_point.raw_value_的出来的结果，自动从64无符号对象变成wrap32对象

![alt text](image-13.png)
在unwrap函数里实现的是将32位的值拆解为靠近checkpoint的64位整数。
首先，先计算offset = (raw_value+ 2^32- zero_point.raw_value) mod 2^32
然后计算base = checkpoint ~ (2^32-1)
计算cand=base+offset，选一个cand和checkpoint的差值最小的数，所以利用了if判断，如果cand比checkpoint小很多就需要加2^32，反之大太多的话就需要减 2^32。
最后得到的cand是最接近checkpoint的。

tcp_receiver的主要功能是负责接受来自TCP发送端的信息和向发送端提供反馈。

![alt text](image-15.png)
这个函数处理收到的信息。
首先先判断RST，如果收到了RST信号，表示发送端请求重置连接，所以需要标记错误并停止处理。
如果收到SYN信号，且zero_point没被设置，表示连接建立，所以需要初始化zero_point，zero_point设置成message.seqno，message.seqno将移动到下一个位置，然后把is_zero_point_set已设置。
如果还没设置好zero_point，消息会无法处理所以需要等待收到SYN信号。
first_index设置需要使用Unwrap函数将循环序列号变成绝对序列号。
如果序列号为0表示消息无效可以直接忽略，如果大于0就减1。
更新next_acknum，next_acknum是下一个希望接收到的序列号。

![alt text](image-16.png)
如果zero_point被设置，就将当前的next_acknum赋值给ReceiverMessage.ackno，否则就会保持默认值。
调用reassembler_.reader().has_error()检查是否存在错误，如果存在错误就设置ReceiverMessage.RST为true。
要计算window_size的大小，先获取当前接受缓冲区的剩余容量，如果剩余容量小于UINT16_MAX就直接使用剩余容量作为窗口大小，但是如果大于UINT16_MAX，就将窗口大小设置为UINT_16MAX。

测试结果:
![alt text](image-12.png)
