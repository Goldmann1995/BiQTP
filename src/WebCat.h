#include <websocketpp/config/asio_client.hpp>
#include <websocketpp/client.hpp>
#include <websocketpp/common/thread.hpp>
#include <websocketpp/config/asio_client.hpp>

// 使用TLS的客户端类型定义
typedef websocketpp::client<websocketpp::config::asio_tls_client> client;
typedef websocketpp::lib::shared_ptr<boost::asio::ssl::context> context_ptr;

// 定义消息处理回调函数
void on_message(websocketpp::connection_hdl, client::message_ptr msg)
{
    sptrAsyncLogger->info("Received message: {}", msg->get_payload());
}

// TLS初始化回调函数
context_ptr on_tls_init()
{
    // 创建TLS上下文
    context_ptr ctx = std::make_shared<boost::asio::ssl::context>(boost::asio::ssl::context::sslv23);

    try
    {
        ctx->set_options(boost::asio::ssl::context::default_workarounds |
                         boost::asio::ssl::context::no_sslv2 |
                         boost::asio::ssl::context::no_sslv3 |
                         boost::asio::ssl::context::single_dh_use);
    }
    catch (std::exception& e)
    {
        sptrAsyncLogger->error("Error in context pointer:{}", e.what());
    }
    return ctx;
}
