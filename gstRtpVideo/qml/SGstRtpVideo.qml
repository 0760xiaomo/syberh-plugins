/*
* This file is part of syberh
*
* Copyright (C) 2019 Beijing Yuan Xin Technology Co.,Ltd. All rights reserved.
*
* Authors:
*       Liu Jingpeng <liujingpeng@syberos.com>
*
* This software, including documentation, is protected by copyright controlled
* by Beijing Yuan Xin Technology Co.,Ltd. All rights are reserved.
*/

import QtQuick 2.3
import com.syberos.basewidgets 2.0
import QtMultimedia 5.5
import gstRtpVideo 1.0

Item {
    id: player

    property string videoId
    property int offsetTop
    property int offsetLeft
    property string address
    property int port
    property int format
    property int pixelWidth
    property int pixelHeight
    property var autoPlay

    property var initVideo: false
    // 视频事件
    signal videoEvent(string vid, string eventType, var eventData)
    // 父级cpage弹出信号
    signal cpagePoped(string vid);

    visible: true

    function start() {
        rtpPlayer.start();
    }

    function stop() {
        rtpPlayer.stop();
    }

    function _ratio(px){
        var ratio = gScreenInfo.density ? gScreenInfo.density : 1;
        return px * ratio;
    }

    onInitVideoChanged: {
        if(!initVideo){
            return;
        }
        var parentItem = gAppUtils.pageStackWindow.pageStack.currentPage;
        // 当前父级的cpage弹出时，发送信号
        parentItem.popped.connect(function(){
            if(player){
                cpagePoped(player.videoId);
            }
        })

        //设置位置
        anchors.top = parentItem.top;
        anchors.left = parentItem.left;
        z = parentItem.z + 1;
        // 设置宽高和距离
        width = _ratio(width);
        height = _ratio(height);
        anchors.topMargin = _ratio(offsetTop) + parentItem.getNavigationBarHeight();
        anchors.leftMargin = _ratio(offsetLeft);

        videoSource.setFormat(player.pixelWidth, player.pixelHeight, player.format);
        videoOutput.source = videoSource

        rtpPlayer.initPipeline(player.address, player.port);
        rtpPlayer.newVideoFrame.connect(videoSource.newVideoFrame);

        if(player.autoPlay){
            player.start();
        }
    }

    // Rectangle {
    //     anchors.fill: parent
    //     color: "red"
    //     z: parent.z +1
    // }

    VideoSource {
        id: videoSource
    }

    GstRtpPlayer {
        id: rtpPlayer
    }

    VideoOutput {
        id: videoOutput
        anchors.fill: parent
        z: parent.z +1
    }

    MouseArea {
        anchors.fill: parent

        onPositionChanged: {
            mouse.accepted = true
            videoEvent(videoId, 'move', {x: mouse.x, y: mouse.y})
        }
        onPressed: {
            mouse.accepted = true
            videoEvent(videoId, 'pressed', {x: mouse.x, y: mouse.y})
        }
        onReleased: {
            mouse.accepted = true
            videoEvent(videoId, 'released', {x: mouse.x, y: mouse.y})
        }
        onDoubleClicked: {
            mouse.accepted = true
            videoEvent(videoId, 'doubleClicked', {x: mouse.x, y: mouse.y})
        }
    }
}

