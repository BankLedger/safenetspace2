# safenetspace2
safenetspace2 (darknetspace &amp; darknetcoin )

# Winows平台版本

dnsd/simplewallet与darknetspacewallet编译：

1.安装依赖库

    a) 安装cmake2.8.6或者之后的版本，下载地址：https://cmake.org/download/
  
    b) 安装boost1.55或者之后的版本，下载地址：
  
      https://sourceforge.net/projects/boost/files/boost/1.56.0/
      
2.下载源码https://github.com/BankLedger/safenetspace2到本地目录

3.打开命令行工具（cmd），cd进入源码根目录

    a)cryptonote目录是dnsd与simplewallet项目

    b)src目录是darknetspacewallet项目
  
4.分别在cryptonote、src目录下创建build文件夹

5.cd build分别进入到build目录下

6.输入命令：cmake –G “Visval Studio 12 Win64” ..

    a)Visval Studio 12指定编译vs2013版本
  
    b)Win64指定编译64位程序
  
    c).. 指定Makefile的目录 ..是上级目录
  
7.命令执行成功后会在build目录下生成各个工程文件及其它编译文件

8.cryptonote\build目录下包含工程dnsd与simplewallet等

9.build目录下包含工程darknetspacewallet等









# Linux平台版本

dnsd与simplewallet编译

1.安装依赖库

    sudo add-apt-repository ppa:boost-latest/ppa
  
    sudo apt-get update
  
    sudo apt-get -y install gcc-4.8 g++-4.8 libboost1.55-all-dev git cmake
  
2.下载源码https://github.com/BankLedger/safenetspace2到本地目录

3.cd进入cryptonote目录，执行make命令，等待编译完成

4.cd进入build/release/src，在这里你可以看到编译好的dnsd，simplewallet可执行文件
