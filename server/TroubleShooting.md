# Server 常见问题排查

## 无法发送图片？

发送图片需要使用到 msfs 后台程序，但是 meili/TeamTalk 最后的官方部署说明中没有提到 msfs。 可以通过以下方法开启 msfs 服务器：
 
 - cp ~/TeamTalk/server/im-server-1/msfs/msfs.conf.example  ~/TeamTalk/server/im-server-1/msfs/msfs.conf 

 - 修改 ~/TeamTalk/server/im-server-1/msfs/msfs.conf 文件里的 ListenIP 为部署机器的IP地址

 - cd ~/TeamTalk/server/im-server-1/msfs && ../daeml msfs
 
这时候 ps -ef | grep msfs 就可以看到 msfs 服务器已经再运行了。

但是需要提及一点安全问题： msfs 使用 http (NOT https), 所有远端的图片都是 http 的公开 URL，每次查看历史消息的对方图片都是 HTTP 连接，不但会有安全隐患，而且读取非本地缓存图片效率也低。


## 截屏发送的图片格式是 BMP？

是的，目前亲测发现截屏是 BMP 格式，没有压缩过，会导致某些情况下传输和显示都比较慢…… 

