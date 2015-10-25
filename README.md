#chann（酱） 匿名版

chann是基于[mongoose](https://github.com/cesanta/mongoose)、[unqlite](http://unqlite.org)的匿名版服务器。

[Demo](https://waifu.cc/)托管于Conoha，配置为单核CPU和1G内存的CentOS 7 x64，chann的开发与运营均基于CentOS 7，故以下叙述全部以此版本为准。

##编译cchan

1. 准备工作：
`yum group install "Development Tools"`

2. chann支持Google的Recaptcha验证服务，若要使用此服务，请先配置libcurl库：
	1. `wget https://github.com/bagder/curl/releases/download/curl-7_45_0/curl-7.45.0.tar.gz | tar xvz`
	2. `cd curl-7.45.0.tar.gz`
	3. `./configure && make && make install`

3. `git clone git@github.com:coyove/chann.git`

4. 运行`make`进行编译。若要使用Recaptcha，运行`make CAPTCHA=1`。

5. 运行`make test`打开测试服务器，默认监听13739端口，管理员密码111。

##启动chann

### chann.conf配置文件
运行`cp chann_test.conf chann.conf`获得一份新的默认配置文件，具体配置在文件中有详细的描述。

需要注意的是请至少设置长度大于10位的MD5盐和管理员密码。

### 添加为服务
在`/usr/lib/systemd/system/chann.service`中添加以下内容：

```
[Unit]
Description=CHANN Anonymous Imageboard

[Service]
ExecStart=/path/to/chann
Restart=always

[Install]
WantedBy=multi-user.target
```

### 启动
使用`systemctl start chann.service`启动服务器。


##性能

使用Webbench对站点进行本地测试，设置为访问同一目标持续10s，500个并发连接，结果如下：

target 		 			|pages/min
------------------------|---------
pixmicat|5,000
Demo站首页|77,000
Demo站第十页|74,000

新版本的chann在用C++进行重构后，数据如下：
target 		 			|pages/min
------------------------|---------
Demo站首页|73,000
Demo站第十页|55,000