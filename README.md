[English](#cchan-anonymous-imageboard)

#cchan 匿名版

cchan是基于[mongoose](https://github.com/cesanta/mongoose)、[unqlite](http://unqlite.org)的匿名版服务器。

[Demo](https://waifu.cc/)，托管于Linode，配置：单核、1G内存、CentOS 7 x64，由于使用的是SSLS的证书，故在一些手机浏览器上会提示证书错误。

编译cchan
---------

目前windows平台上的开发已不再继续，但以往版本中的VS工程文件依旧可以打开。

在Linux平台下编译，请使用64位系统和GCC 4.8以上的版本，若要使用HTTPS，请安装OpenSSL。

运行`make clean && make`进行编译。

运行`make test`打开测试服务器，默认为监听`13739`端口，管理员密码`111`。

在mingw64平台下编译，请使用CLion导入源码即可。

启动cchan
---------
首次启动cchan时，请务必遵循以下步骤：

1. 请首先在运行目录中新建`images`目录。
2. 使用命令`--salt XXX`设置MD5盐，设置之后每次启动cchan请使用同一个值。XXX不超过64个ASCII字符，默认为`coyove`。
请至少使用16位以上的盐。
3. 使用命令`--database XXX`设置数据库位置。
4. 使用命令`--admin-spell XXX`设置管理员密码，默认为随机生成字符串。
5. 使用命令`--port XXX`设置监听端口。
6. 其他命令请参考源码。

性能
---------
使用Webbench对站点进行测试，统一设置为持续10s的500并发连接，结果如下：

目标	 		 			|pages/min
------------------------|---------
waifu.cc/h/pixmicat.php	|5000
waifu.cc/page/1			|77000
waifu.cc/page/9			|74000
make test(1000回复)		|5000

#CCHAN Anonymous Imageboard

CCHAN is an anonymous imageboard based on [mongoose](https://github.com/cesanta/mongoose) and [unqlite](http://unqlite.org).

[Demo](https://waifu.cc/) is hosted on Linode, CentOS 7 x64 running on a single core CPU with 1GB of RAM, the site is using a certificate issued by ssls.com and some browsers may raise SSL warnings.

Compile the Server
------------------

Currently the development on Windows has been deprecated.

To compile it on Linux, you need a 64bit system with GCC > 4.8.

Run `make clean && make` to compile.

Run `make test` to open a test server listening on 13739 and the admin's password is `111`.

To compile it on mingw64, import all the codes into CLion and it should work.

Start the Server
----------------
Follow these steps:

1. `mkdir images` if not created.
2. Use `--salt XXX` to set a MD5 salt, its value shall not be changed since then. The length of XXX is 64 at most and you should NEVER make it less than 16 characters.
3. Use `--database XXX` to set the location of your database.
4. Use `--admin-spell XXX` to set a admin's password, it would be a random string if you left it blank.
5. Use `--port XXX` to set the listening port.
6. For other commands please refer the source code.

Note on i18n
---------
CCHAN is a small server and it compiles fast, inside `./src/lang.h` is the Chinese translation, feel free to add yours and recompile it.

Performance
---------
Use WebBench for benchmarking, 500 clients & 10s:

URL	 		 			|pages/min
------------------------|---------
waifu.cc/h/pixmicat.php	|5000
waifu.cc/page/1			|77000
waifu.cc/page/9			|74000
make test(1000 replies)	|5000