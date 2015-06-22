cchan 匿名版
============

cchan是基于mongoose、unqlite的匿名版服务器。

cchan目前仍是一个非常早期的预览版本，代码基本不会对运行结果进行检查，这将导致潜在Unhandled Exceptions的出现。即便短时间内没有问题，这也是非常要命的。

编译cchan
---------
cchan在`Windows 8.1`和`Visual Studio 2013`下编译通过，若要在Linux平台下编译，请使用GCC 4.8以上的版本。

请在64位系统中进行编译。

cchan的推荐运行平台为`Windows Sever 2012`和`Windows Sever 2008`，通过修改VS中的`Platform Toolset`为`Visual Studio 2013 - Windows XP (v120_xp)`可以让cchan在`Windows Sever 2003`下运行，但*实测后BUG太多不推荐*。


启动cchan
---------
首次启动cchan时（即`sql.exe`），请务必遵循以下步骤：

1. 请首先在运行目录中新建`images`目录。
2. 若需要将cchan部署到服务器上，请使用命令`-NOGPFAULTERRORBOX`。
3. 第一次运行请使用命令`-newprofile`初始化数据库，否则无法正常运行。
4. 使用命令`-salt XXX`设置MD5盐，设置之后每次启动cchan请使用同一个值。XXX不超过8个ASCII字符，默认为`coyove`。
5. 使用命令`-spell XXX`设置管理员密码，默认为随机生成字符串。
6. 使用命令`-port XXX`设置监听端口。
7. 其他命令请参考源码。

`watch.bat`和`start-server.bat`为启动服务器的脚本，**推荐使用`watch.bat`启动服务器而不是直接运行`sql.exe`**。其中：

* `watch.bat`为监视脚本，其每隔10s检测`sql.exe`是否异常退出，并通过启动`start-server.bat`来打开服务器。
* `start-server.bat`为启动脚本，其首先打开`ssl_wrapper.exe`将443端口的数据重定向至8080端口，然后启动`sql.exe`。
* `cert.pem`为测试用的ssl证书。