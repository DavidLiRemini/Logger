# Logger
简单的日志管理工具
主要是为了方便非控制台类程序调试和追踪程序问题。此工具在主应用程序运行期间，始终处于后台静默运行。
应用程序中的调试信息都会在后台写到日志文件。单个日志文件最大为4M，达到文件上限会自动滚动，日志文
件名及其日志消息格式需要在程序初始化时指定，当然也可以以json形式写为本地配置文件，通过在初始化的
时候读取配置文件来决定格式。如格式可以为" %TimeStamp%_N %Message% "。
问题：如果在密集写入的情况下有时会出现空字符，以及丢失少量数据。个人预测是双缓冲切换和CPU线程调度
和文件交互时处理不当导致的，目前尚未解决。但对于一般的GUI应用程序来言是可以使用的，因为GUI程序一般
整个生命周期内，占用CPU的是事件的消息循环。用户代码一般不会在一个循环当中写入大量的日志。当然写入
大量的日志也是可以的，只是会存在可能多个日志文件的大小并不是非常准确的平均为4M，实践当中并不是完全
分布平均的，但不会丢失数据。密集写入情况下，可以6秒内写入50000行数据。
# Logger
