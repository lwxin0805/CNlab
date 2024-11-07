Checkpoint 1 Writeup
====================

My name: 赖炜欣 

My Student ID: 205220026

I did attend the lab session.

![alt text](image.png)
buffer_是用来存储未组装的数据片段。
unassembled_bytes是用来记录buffer_中所有数据片段的总未组装字节的节数。
current_index_是用来记录已组装并输出的数据的结束索引。

![alt text](image-1.png)
capacity用来记录能获取ByteStream的可用容量。
left_bound和right_bound在这里的作用就是一个边界，对于left_bound是first_index和current_index_中的最大值，但是不能早于current_index_，right_bound取current_index_ + capacity和first_index+data.size()中的最小值，但不能超出ByStream的可用容量或数据的末尾。

![alt text](image-2.png)
创建新的数据片段。

![alt text](image-3.png)
更新未组装的字节数。

![alt text](image-6.png)
找到新片段插入的位置。

![alt text](image-5.png)
处理重叠的片段，可以分成两个情况部分重叠或者完全重叠。
对于部分重叠，利用if判断，如果新片段部分覆盖了当前片段，首先将当前片段中新片段未覆盖的数据追加到新片段的末尾，然后减少unassembled_bytes，更新新片段的所以就行。
对于完全覆盖，直接更新未组装的字节数。
对于这两个情况都需要删除已经处理的片段。

![alt text](image-7.png)
这个部分是处理合并阶段，有两种情况，一个是前一个片段完全覆盖了新片段或新片段和前一个片段部分重叠，需要合并。
对于第一个情况，如果完全覆盖了，新片段的数据已经被包含在前一个片段里了，所以只需要减少unassembled_bytes就可以了
对于第二个情况，因为只有部分重叠所以需要合并，将新片段未被前一个片段所覆盖的树蕨加到前一个片段里，然后更新数据且减少unassembled_bytes.

![alt text](image-8.png)
插入新片段。

![alt text](image-9.png)
用于输出。

![alt text](image-3.png)
用于返回当前未组装的总结字数。

测试结果:
![alt text](image-10.png)

Implementation Challenges:
一直卡在时间复杂度上面，使用了vector以后就减少了。
