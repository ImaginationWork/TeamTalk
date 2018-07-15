package com.mogujie.tt.config;

/**
 * @author : yingmu on 15-1-5.
 * @email : yingmu@mogujie.com.
 *
 */
public interface DBConstant {

    /**性别
     * 1. 男性 2.女性
     * */
    public final int SEX_MAILE = 1;
    public final int SEX_FEMALE = 2;

    /**msgDisplayType
     * 保存在DB中，与服务端一致，图文混排也是一条
     * 1. 最基础的文本信息
     * 2. 纯图片信息
     * 3. 语音
     * 4. 图文混排
     * */
    public final int SHOW_ORIGIN_TEXT_TYPE = 1;
    public final int  SHOW_IMAGE_TYPE = 2;
    public final int  SHOW_AUDIO_TYPE = 3;
    public final int  SHOW_MIX_TEXT = 4;
    public final int  SHOW_GIF_TYPE = 5;


    public final String DISPLAY_FOR_IMAGE = "[图片]";
    public final String DISPLAY_FOR_MIX = "[图文消息]";
    public final String DISPLAY_FOR_AUDIO = "[语音]";
    public final String DISPLAY_FOR_ERROR = "[未知消息]";


    /**sessionType*/
    public final int  SESSION_TYPE_SINGLE = 1;
    public final int  SESSION_TYPE_GROUP = 2;
    public final int SESSION_TYPE_ERROR= 3;

    
   /**group type*/
   public final int  GROUP_TYPE_NORMAL = 1;
   public final int  GROUP_TYPE_TEMP = 2;

    /**group status
     * 1: shield  0: not shield
     * */

    public final int  GROUP_STATUS_ONLINE = 0;
    public final int  GROUP_STATUS_SHIELD = 1;

    /**group change Type*/
    public final int  GROUP_MODIFY_TYPE_ADD= 0;
    public final int  GROUP_MODIFY_TYPE_DEL =1;

    /**depart status Type*/
    public final int  DEPT_STATUS_OK= 0;
    public final int  DEPT_STATUS_DELETE =1;

}
