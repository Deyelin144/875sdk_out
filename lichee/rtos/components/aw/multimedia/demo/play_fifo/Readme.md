# play_fifo 示例工程

## 使用方法
1. 和播放其他音频相同，在播放fifo音频前需要调用`player_init`创建播放器。
2. 调用`audio_fifo_create`，这代码的意思实际上是把应用层需要用到的行为给抽象出来，可以不用理解。
3. 调用`AudioFifoSetPlayer`,这代码的作用就是把第一步创建的播放器和第二步创建的fifo建立连接，让他们成为朋友。
4. 调用`AudioFifoStart`,开始播放。
5. 调用`AudioFifoPutData`,源源不断的把数据放到fifo中即可。

## 解析
fifo stream和其他stream不相同，在cedarx里面的fifo stream是不完全的，他没办法脱离play_fifo里面的文件单独运行，所以该demo本身就属于fifo stream的一部分，这也导致客户阅读起来比较难理解的原因。
总结就是，fifo有以下接口可以调用：
```
audio_fifo_create：创建fifo
AudioFifoSetPlayer：播放器播放这个fifo
AudioFifoStart：创建的fifo开始运行
AudioFifoPutData：把流数据存进fifo
AudioFifoStop：fifo停止播放
audio_fifo_destroy：销毁fifo
```