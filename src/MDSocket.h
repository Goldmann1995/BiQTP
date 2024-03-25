/*
 * File:        MDSocket.h
 * Author:      summer@SummerLab
 * CreateDate:  2024-03-21
 * LastEdit:    2024-03-25
 * Description: Receive MarketData by WebSocket from Binance
 */

#pragma once

#include <ThreadBase.h>

// 使用TLS的客户端类型定义
typedef websocketpp::client<websocketpp::config::asio_tls_client> WSSClient;
typedef websocketpp::lib::shared_ptr<boost::asio::ssl::context> ContextSPtr;


//############################################################//
//   MDSocket Class
//############################################################//
class MDSocket: public ThreadBase
{
public:
    MDSocket(const std::string& url);
    ~MDSocket();

    // 线程运行实体
    void Run();

    //@websocketp// 回调函数需要static
    // 初始化WSSUrl
    static void InitWSSUrl();
    // TLS初始化回调函数
    static ContextSPtr OnTlsInit();
    // WebSocket消息回调函数
    static void OnMessage(websocketpp::connection_hdl, WSSClient::message_ptr msg);
    // WebSocket连接回调函数
    static void OnOpen(websocketpp::connection_hdl hdl);
    // WebSocket失败回调函数
    static void OnFail(websocketpp::connection_hdl hdl);
    // WebSocket关闭回调函数
    static void OnClose(websocketpp::connection_hdl hdl);
    // WSSClient重新初始化
    static void ReInit();

private:
    static int mMsgCnt;
    static std::string mMdUrl;
    static WSSClient mWSSClient;
    static websocketpp::connection_hdl mConnHdl;
};

//############################################################//
