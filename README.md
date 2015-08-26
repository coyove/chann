cchan 匿名版
============

cchan是基于[mongoose](https://github.com/cesanta/mongoose)、[unqlite](http://unqlite.org)的匿名版服务器。

[Demo](https://waifu.cc/)，托管于Linode，配置：单核、1G内存、CentOS 7 x64，由于使用的是SSLS的证书，故在一些手机浏览器上会提示证书错误。

编译cchan
---------

目前windows平台上的开发已不再继续，但以往版本中的VS工程文件依旧可以打开。

在Linux平台下编译，请使用64位系统和GCC 4.8以上的版本，若要使用HTTPS，请安装OpenSSL。

运行`make & make clean`进行编译。

运行`make test`打开测试服务器，默认为监听`13739`端口，管理员密码`111`。

启动cchan
---------
首次启动cchan时，请务必遵循以下步骤：

1. 请首先在运行目录中新建`images`目录。
2. 使用命令`--salt XXX`设置MD5盐，设置之后每次启动cchan请使用同一个值。XXX不超过64个ASCII字符，默认为`coyove`。
请至少使用16位以上的盐。
4. 使用命令`--admin-spell XXX`设置管理员密码，默认为随机生成字符串。
5. 使用命令`--port XXX`设置监听端口。
6. 其他命令请参考源码。

安全性
-----
本质上讲，cchan除了设置发言间隔时间和ban ID之外，安全性是很低的，尤其是update、slogan、delete、sage、ban、state等操作完全是明文传输，
为了提高安全性，推荐使用反向代理。

性能
----
使用jMeter对[Demo](https://waifu.cc/)站点进行远程测试，本地环境为中国电信10Mbps。
使用Apache Benchmark对[Demo](https://waifu.cc/)站点进行本地测试。