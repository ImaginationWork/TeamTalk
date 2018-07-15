package com.mogujie.tt.imservice.entity;

import android.text.TextUtils;

import com.mogujie.tt.DB.entity.MessageEntity;
import com.mogujie.tt.config.DBConstant;
import com.mogujie.tt.config.MessageConstant;
import com.mogujie.tt.protobuf.helper.ProtoBuf2JavaBean;
import com.mogujie.tt.protobuf.IMBaseDefine;

import org.json.JSONException;

import java.util.ArrayList;
import java.util.List;

/**
 * @author : yingmu on 15-1-6.
 * @email : yingmu@mogujie.com.
 *
 * historical reasons,没有充分利用msgType字段
 * 多端的富文本的考虑
 */
public class MsgAnalyzeEngine {
    public static String analyzeMessageDisplay(String content){
        String finalRes = content;
        String originContent = content;
        return finalRes;
    }



}
