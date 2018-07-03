# Server 编译部署说明

## 基本环境
以下过程都是基于 CentOS-7-x86_64-Minimal-1804.iso 实验的。建议使用 root 账户调试，并且请确保网络畅通，能够 git clone 这个工程。

PS：如果要设置动态获取IP的话请修改 /etc/sysconfig/network-scripts/ifcfg-enp0s3 的 ONBOOT 选项为 yes （假设网卡名称是 enp0s3），然后重启网络：“service network restart”


## yum install 项
大部分软件可以通过 yum install 安装：

yum install -y epel-release && yum install -y git wget gcc gcc-c++ unzip mariadb-devel mariadb-server redis


## 编译安装 其他依赖项
先 clone 下来（假设在 ~/TeamTalk 目录下），然后弄一下 protobuf, hiredis, log4cxx ； 然后编译整个 server： 

git clone https://github.com/ImaginationWork/TeamTalk && cd ~/TeamTalk/server/src/ && chmod +x *.sh && ./make_protobuf.sh && ./make_hiredis.sh && ./make_log4cxx.sh && ./build.sh version 1


## 配置数据库、关闭防火墙
cd ~/TeamTalk/auto_setup/mariadb/ && chmod +x *.sh && ./setup.sh install

在这里会提示修改 MariaDB 的 root 密码。这里设置为 12345，确保和 dbproxyserver.conf 和 ~/TeamTalk/auto_setup/mariadb/setup.sh 文件里的保持一致。剩下的 MariaDB 的设置可以都按照默认的来（一路回车）。 然后设置：

systemctl disable firewalld.service # 防火墙默认开机不启动

systemctl stop firewalld.service # 防火墙关闭

systemctl enable redis

systemctl start redis

systemctl enable mariadb

systemctl start mariadb


## 手动添加两个测试账号：

mysql -u root -p12345

show databases;

use teamtalk;

insert into IMUser (id, sex, name, domain, nick, password, salt, phone, email, departId, created, updated, push_shield_status, sign_info) VALUES (1,1,"LiLe","","李雷","e73a9ee09b8d114a8f1cf45786c00374","LiLei","","LiLei@wtf.com",4,1,1,1,"我是李雷");

insert into IMUser (id, sex, name, domain, nick, password, salt, phone, email, departId, created, updated, push_shield_status, sign_info) VALUES (2,2,"HanM","","韩梅梅","8cc8fab7cad923e51b94c3d26dc499eb","HanMeimei","","HanMeimei@wtf.com",6,1,1,1,"我是韩梅梅");


## 配置服务器 IP
修改 ~/TeamTalk/server/src/msg_server/msgserver.conf 的 IpAdr1 为 运行机器的 IP 地址。


## 编译 Server
cd ~/TeamTalk/server/src/ && chmod +x *.sh && ./build.sh version 1


## 运行！
cd ~/TeamTalk/server/ && tar -zxf im-server-1.tar.gz && cd im-server-1 && ./sync_lib_for_zip.sh && ./restart.sh db_proxy_server && ./restart.sh login_server && ./restart.sh msg_server && ./restart.sh route_server && ./restart.sh file_server && ./restart.sh http_msg_server

这时候 

ps -ef | grep _server 

至少会有 6 个进程。

然后各个 server 的 log 都在各自目录的 log/default.log 文件里。

到此服务端就run起来了。


## 常见问题

请步移至 [这篇文章](TroubleShooting.md)

