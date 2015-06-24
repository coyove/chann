cchan 匿名版
============

cchan是基于[mongoose](https://github.com/cesanta/mongoose)、[unqlite](http://unqlite.org)、[ssl_wrapper](https://github.com/cesanta/ssl_wrapper)的匿名版服务器。

cchan目前仍是一个非常早期的预览版本，代码基本不会对运行结果进行检查，这将导致潜在Unhandled Exceptions的出现。即便短时间内没有问题，这也是非常要命的。

[Demo](http://h.waifu.cc/)，托管于阿里云ECS，配置：单核、1G内存、Server 2012 x64 中文标准版、峰值带宽100Mbps。

编译cchan
---------
cchan在`Windows 8.1`和`Visual Studio 2012 Express`下编译通过，若要在Linux平台下编译，请使用GCC 4.8以上的版本。

请在64位系统中进行编译。

cchan的推荐运行平台为`Windows Sever 2012`和`Windows Sever 2008`，通过修改VS中的`Platform Toolset`为`Visual Studio 2012 - Windows XP (v110_xp)`可以让cchan在`Windows Sever 2003`下运行，但*实测后BUG太多不推荐*。

cchan前端页面使用UTF-8编码，但cchan本身并不处理UTF的转换。所以在修改代码时，请*不要输入任何中文*，请*输入中文字符的转义代码*。
[汉字转化unicode编码](http://www.bangnishouji.com/tools/chtounicode.html)

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