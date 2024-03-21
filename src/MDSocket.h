/*
 * File:        MDSocket.h
 * Author:      summer@SummerLab
 * CreateDate:  2024-03-21
 * LastEdit:    2024-03-21
 * Description: Receive MarketData by WebSocket from Binance
 */

#pragma once

// internal
#include <ThreadBase.h>
// 3rd-lib
#include <websocketpp/config/asio_client.hpp>
#include <websocketpp/client.hpp>
#include <websocketpp/common/thread.hpp>
#include <websocketpp/config/asio_client.hpp>

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
    
    // TLS初始化回调函数
    static ContextSPtr OnTlsInit();
    // WebSocket消息回调函数
    static void OnMessage(websocketpp::connection_hdl, WSSClient::message_ptr msg);
    // WebSocket关闭回调函数
    static void OnClose(websocketpp::connection_hdl hdl);
    // 断线重连
    static void TryReconnect();

private:
    static int mMsgCnt;
    static std::string mMdUrl;
    static WSSClient mWSSClient;
};

//############################################################//
