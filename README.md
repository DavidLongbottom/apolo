# apolo
ctp , quant, trade <br>

# 项目介绍
这个项目目的是测试ctp 的接口， 并且寻找一种可以水平拓展低耦合的架构。 <br>

# 结果
跑通了ctp， 使用zmq的ipc pub/sub实现了水平拓展<br>

# 依赖
c++11以上，boost, zmq, talib， 仅在centos 上测试过

# 说明
此项目将不在维护和更新， 我以后有时间会重启一个项目，完全设计架构以及完全重写代码。<br>
该项目不在维护的原因是：<br>
1、项目需要的zmq的pub/sub 是不可靠的通信方式。这种不可靠通信的特点让我放弃在这个代码上采取进一步修改。<br>
    为什么项目开始采用了zmq? <br>
    因为zmq使用简单便捷， 可以轻松的实现跨线程、跨进程、不同主机之间的通信。 其次，zmq不需要共享内存那样需要精心维护，带来极大便捷。 最后zmq的速度很快，几乎比同类所有的通信中间件要快<br>
    为什么我不采取措施修改zmq pub/sub 让其可靠？<br>
    因为可靠的通信带来延迟，速度要慢一倍（虽然即时慢一倍仍然非常的快）， 而且这样使用违背了zmq的设计哲学。 <br>
    
2、 zmq 占据了每一个进程的主循环， 这是分布带来的代价，让整个程序不优雅。<br>
    为什么没有采用epoll 之类的？<br>
    zmq类似epoll<br>
3、 我对ctp这种自动交易软件的设计的观念发生了变化<br>
    我认为一个好的交易软件应该做到的是在简单易用的情况下尽可能的保持性能。 简单易用是指可以轻松的修改，甚至重构代码，可以灵活的根据想法来修改程序。<br>
    我认为单纯的追求极致的性能而不能按照自己的想法来改动代码，这只会让程序成为一个黑箱，而且我本能的讨厌这样， 如果不能按照自己的想法来修改， 这真是非常讨厌的一件事情<br>
4、 代码没有进行review <br>

# 新项目计划
新项目将优先考虑易用性，考虑修改的简单方便， 考虑对不同接口的封装， 完成分析用数据库的建设，并且增加一些常用的功能。 
