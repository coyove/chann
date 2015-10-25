[English](#chann-anonymous-imageboard)
#chann（酱） 匿名版

chann是基于[mongoose](https://github.com/cesanta/mongoose)、[unqlite](http://unqlite.org)的匿名版服务器。

[Demo](http://chann.org/)托管于Conoha，配置为单核CPU和1G内存的CentOS 7 x64，chann的开发与运营均基于CentOS 7，故以下叙述全部以此版本为准。

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


#Chann Anonymous Imageboard

Chann is an anonymous imageboard written in C/C++ based on [mongoose](https://github.com/cesanta/mongoose) and [unqlite](http://unqlite.org).

[Demo](http://chann.org/) is hosted on Conoha, running on CentOS 7 x64 with 1g of RAM.

##Compile

1. Get started:
`yum group install "Development Tools"`

2. Chann supports Google RECAPTCHA, to enable it, you should do the followings first:
	1. `wget https://github.com/bagder/curl/releases/download/curl-7_45_0/curl-7.45.0.tar.gz | tar xvz`
	2. `cd curl-7.45.0.tar.gz`
	3. `./configure && make && make install`

3. `git clone git@github.com:coyove/chann.git`

4. run `make` to compile, or `make CAPTCHA=1` if you need RECAPTCHA verifications.

5. run `make test` to open a test server listening on 13739 and the admin's password is 111.

##Launch

### Create chann.conf
Run `cp chann_test.conf chann.conf` to get a fresh copy of the configuration file. Every options in the file is commented.

Note: you should set the md5 salt and the admin's password using complex combinations.

### Added as a service
Inside `/usr/lib/systemd/system/chann.service` adding:

```
[Unit]
Description=CHANN Anonymous Imageboard

[Service]
ExecStart=/path/to/chann
Restart=always

[Install]
WantedBy=multi-user.target
```

### Launch the service
Run `systemctl start chann.service` to start.


##Performance

Use Webbench to test the performance (500 clients/sec, lasting 10s):

target 		 			|pages/min
------------------------|---------
pixmicat|5,000
Demo site's front page|77,000
Demo site's 10th page|74,000

I have rebuilt chann using C++ recently and here're the new results:

target 		 			|pages/min
------------------------|---------
Demo site's front page|73,000
Demo site's 10th page|55,000