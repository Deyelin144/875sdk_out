# AT通讯协议

## 协议框架

### 协议描述

AT命令最⼤⻓度⼩于64个字符

#### 发送协议：

固定"AT+"开头，"\r\n"结尾
查询命令
AT+<X>?\r\n
该命令用于返回参数的当前值
设置命令
AT+<X>=<param1,param2...>\r\n
该命令用于设置用户自定义的参数值

#### 响应协议：

如果有需要回复的内容，则先返回":+<RES>:"，然后回复内容。 正确响应以"OK\r\n"结尾，错误响应以"ERROR\r\n"结尾
+<RES>:param1,param2...

OK\r\n

举例1：
REQ:
AT+TEST?\r\n
RES:
+RES:12,45

举例2：
REQ:
AT+TEST=1,2,3\r\n
RES:
OK\r\n

举例2：
REQ:
AT+TEST=4,5,6\r\n
RES:
+RES:wrong param\r\n
ERROR\r\n