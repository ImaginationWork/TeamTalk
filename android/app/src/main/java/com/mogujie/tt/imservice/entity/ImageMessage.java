package com.mogujie.tt.imservice.entity;

import com.mogujie.tt.DB.entity.MessageEntity;
import com.mogujie.tt.DB.entity.PeerEntity;
import com.mogujie.tt.DB.entity.UserEntity;
import com.mogujie.tt.config.DBConstant;
import com.mogujie.tt.config.MessageConstant;
import com.mogujie.tt.config.SysConstant;
import com.mogujie.tt.imservice.support.SequenceNumberMaker;
import com.mogujie.tt.protobuf.IMBaseDefine;
import com.mogujie.tt.ui.adapter.album.ImageItem;
import com.mogujie.tt.utils.CommonUtil;
import com.mogujie.tt.utils.FileUtil;

import org.json.JSONException;
import org.json.JSONObject;

import java.io.File;
import java.io.FileOutputStream;
import java.io.Serializable;
import java.io.UnsupportedEncodingException;
import java.util.ArrayList;
import java.util.Arrays;
import java.util.Collections;
import java.util.Comparator;

/**
 * @author : yingmu on 14-12-31.
 * @email : yingmu@mogujie.com.
 */
public class ImageMessage extends MessageEntity implements Serializable {

    /**本地保存的path*/
    private String path = "";
    /**图片的网络地址*/
    private String url = "";
    private int loadStatus;
    private static int extHeaderLength = 10; // TODO: USE PROTOBUF INSTEAD

    //存储图片消息
    private static java.util.HashMap<Long,ImageMessage> imageMessageMap = new java.util.HashMap<Long,ImageMessage>();
    private static ArrayList<ImageMessage> imageList=null;
    /**
     * 添加一条图片消息
     * @param msg
     */
    public static synchronized void addToImageMessageList(ImageMessage msg){
        try {
            if(msg!=null && msg.getId()!=null)
            {
                imageMessageMap.put(msg.getId(),msg);
            }
        }catch (Exception e){
        }
    }

    /**
     * 获取图片列表
     * @return
     */
    public static ArrayList<ImageMessage> getImageMessageList(){
        imageList = new ArrayList<>();
        java.util.Iterator it = imageMessageMap.keySet().iterator();
        while (it.hasNext()) {
            imageList.add(imageMessageMap.get(it.next()));
        }
        Collections.sort(imageList, new Comparator<ImageMessage>(){
            public int compare(ImageMessage image1, ImageMessage image2) {
                Integer a =  image1.getUpdated();
                Integer b = image2.getUpdated();
                if(a.equals(b))
                {
                    return image2.getId().compareTo(image1.getId());
                }
                // 升序
                //return a.compareTo(b);
                // 降序
                return b.compareTo(a);
            }
        });
        return imageList;
    }

    /**
     * 清除图片列表
     */
    public static synchronized void clearImageMessageList(){
        imageMessageMap.clear();
        imageMessageMap.clear();
    }



    public ImageMessage(){
        msgId = SequenceNumberMaker.getInstance().makelocalUniqueMsgId();
    }

    /**消息拆分的时候需要*/
    private ImageMessage(MessageEntity entity){
        /**父类的id*/
         id =  entity.getId();
         msgId  = entity.getMsgId();
         fromId = entity.getFromId();
         toId   = entity.getToId();
        sessionKey = entity.getSessionKey();
         content=entity.getContent();
         msgType=entity.getMsgType();
         displayType=entity.getDisplayType();
         status = entity.getStatus();
         created = entity.getCreated();
         updated = entity.getUpdated();
    }

    public static String saveImageToFile(byte[] content, String mainName) {
        try {
            byte header[] = new byte[extHeaderLength];
            System.arraycopy(content, 0, header, 0, extHeaderLength);

            int extLength = 0;
            for (int i = 0; i < extHeaderLength; i++){
                if (header[i] == 0){
                    extLength = i;
                    break;
                }
            }

            byte extBytes[] = new byte[extLength];
            System.arraycopy(header, 0, extBytes, 0, extLength);
            String ext = new String(extBytes);

            String savePath = CommonUtil.getImageSavePath(mainName + "."+ext);
            File file = new File(savePath);
            FileOutputStream fops = new FileOutputStream(file);
            fops.write(content,extHeaderLength,content.length-extHeaderLength);
            fops.flush();
            fops.close();
            return savePath;
        } catch (Exception e) {
            e.printStackTrace();
            return null;
        }
    }

    public static MessageEntity parseFromNet(IMBaseDefine.MsgInfo msgInfo) {
        ImageMessage messageEntity = new ImageMessage();

        messageEntity.setCreated(msgInfo.getCreateTime());
        messageEntity.setUpdated(msgInfo.getCreateTime());
        messageEntity.setFromId(msgInfo.getFromSessionId());
        messageEntity.setMsgId(msgInfo.getMsgId());
        messageEntity.setMsgType(msgInfo.getMsgType().getNumber());
        messageEntity.setStatus(MessageConstant.MSG_SUCCESS);
        messageEntity.setDisplayType(DBConstant.SHOW_IMAGE_TYPE);
//        messageEntity.setContent(msgInfo.getMsgData().toStringUtf8());
        byte desMessage[] = com.mogujie.tt.Security.getInstance().DecryptMsg2(msgInfo.getMsgData().toByteArray());
        String finalPath = saveImageToFile(desMessage, msgInfo.getMsgId()+"_"+msgInfo.getFromSessionId());
        try {
            JSONObject extraContent = new JSONObject();
            extraContent.put("path",finalPath);
            extraContent.put("loadStatus",MessageConstant.IMAGE_LOADED_SUCCESS);
            messageEntity.setContent(extraContent.toString());

            messageEntity.setPath(finalPath);
            messageEntity.setLoadStatus(MessageConstant.IMAGE_LOADED_SUCCESS);

        } catch (JSONException e) {
            e.printStackTrace();
        }
//        messageEntity = ImageMessage.parseFromNet(messageEntity);

        return messageEntity;
    }


    /**接受到网络包，解析成本地的数据*/
    public static ImageMessage parseFromNet(MessageEntity entity) throws JSONException {
        String strContent = entity.getContent();
        ImageMessage textMessage = new ImageMessage(entity);
        textMessage.setStatus(MessageConstant.MSG_SUCCESS);
        textMessage.setDisplayType(DBConstant.SHOW_IMAGE_TYPE);
        return textMessage;
    }


    public static ImageMessage parseFromDB(MessageEntity entity)  {
        if(entity.getDisplayType() != DBConstant.SHOW_IMAGE_TYPE){
            throw new RuntimeException("#ImageMessage# parseFromDB,not SHOW_IMAGE_TYPE");
        }
        ImageMessage imageMessage = new ImageMessage(entity);
        String originContent = entity.getContent();
        JSONObject extraContent;
        try {
            extraContent = new JSONObject(originContent);
            imageMessage.setPath(extraContent.getString("path"));
            int loadStatus = extraContent.getInt("loadStatus");

            //todo temp solution
            if(loadStatus == MessageConstant.IMAGE_LOADING){
                loadStatus = MessageConstant.IMAGE_UNLOAD;
            }
            imageMessage.setLoadStatus(loadStatus);
        } catch (JSONException e) {
            e.printStackTrace();
        }

        return imageMessage;
    }

    // 消息页面，发送图片消息
    public static ImageMessage buildForSend(ImageItem item,UserEntity fromUser,PeerEntity peerEntity){
        ImageMessage msg = new ImageMessage();
        if (new File(item.getImagePath()).exists()) {
            msg.setPath(item.getImagePath());
        } else {
            if (new File(item.getThumbnailPath()).exists()) {
                msg.setPath(item.getThumbnailPath());
            } else {
                // 找不到图片路径时使用加载失败的图片展示
                msg.setPath(null);
            }
        }
        // 将图片发送至服务器
        int nowTime = (int) (System.currentTimeMillis() / 1000);

        msg.setFromId(fromUser.getPeerId());
        msg.setToId(peerEntity.getPeerId());
        msg.setCreated(nowTime);
        msg.setUpdated(nowTime);
        msg.setDisplayType(DBConstant.SHOW_IMAGE_TYPE);
        // content 自动生成的
        int peerType = peerEntity.getType();
        int msgType = peerType == DBConstant.SESSION_TYPE_GROUP ? IMBaseDefine.MsgType.MSG_TYPE_GROUP_IMAGE_VALUE:
                IMBaseDefine.MsgType.MSG_TYPE_SINGLE_IMAGE_VALUE ;
        msg.setMsgType(msgType);

        msg.setStatus(MessageConstant.MSG_SENDING);
        msg.setLoadStatus(MessageConstant.IMAGE_UNLOAD);
        msg.buildSessionKey(true);
        return msg;
    }

    public static ImageMessage buildForSend(String takePhotoSavePath,UserEntity fromUser,PeerEntity peerEntity){
        ImageMessage imageMessage = new ImageMessage();
        int nowTime = (int) (System.currentTimeMillis() / 1000);
        imageMessage.setFromId(fromUser.getPeerId());
        imageMessage.setToId(peerEntity.getPeerId());
        imageMessage.setUpdated(nowTime);
        imageMessage.setCreated(nowTime);
        imageMessage.setDisplayType(DBConstant.SHOW_IMAGE_TYPE);
        imageMessage.setPath(takePhotoSavePath);
        int peerType = peerEntity.getType();
        int msgType = peerType == DBConstant.SESSION_TYPE_GROUP ? IMBaseDefine.MsgType.MSG_TYPE_GROUP_TEXT_VALUE
                : IMBaseDefine.MsgType.MSG_TYPE_SINGLE_TEXT_VALUE;
        imageMessage.setMsgType(msgType);

        imageMessage.setStatus(MessageConstant.MSG_SENDING);
        imageMessage.setLoadStatus(MessageConstant.IMAGE_UNLOAD);
        imageMessage.buildSessionKey(true);
        return imageMessage;
    }

    /**
     * Not-null value.
     */
    @Override
    public String getContent() {
        JSONObject extraContent = new JSONObject();
        try {
            extraContent.put("path",path);
            extraContent.put("loadStatus",loadStatus);
            String imageContent = extraContent.toString();
            return imageContent;
        } catch (JSONException e) {
            e.printStackTrace();
        }
        return null;
    }

    @Override
    public byte[] getSendContent() {
        // 发送的时候非常关键
        byte header[] = new byte[extHeaderLength];
        Arrays.fill(header, (byte)0);
        String fileName = this.getPath();

        String suffix = fileName.substring(fileName.lastIndexOf(".") + 1);
        byte ext[] = suffix.getBytes();

        System.arraycopy(ext, 0, header, 0, ext.length);
        byte out[] = FileUtil.File2byteAndAppendHeader(header, this.getPath());

        /**
         * 加密
         */
       String  encrySendContent =new String(com.mogujie.tt.Security.getInstance().EncryptMsg2(out));

        try {
            return encrySendContent.getBytes("utf-8");
        } catch (UnsupportedEncodingException e) {
            e.printStackTrace();
        }
        return null;
    }

    /**-----------------------set/get------------------------*/
    public String getPath() {
        return path;
    }

    public void setPath(String path) {
        this.path = path;
    }

    public int getLoadStatus() {
        return loadStatus;
    }

    public void setLoadStatus(int loadStatus) {
        this.loadStatus = loadStatus;
    }
}
