    // 实例化客户端
    client c;

    try
    {
        // 设置日志输出等级
        c.set_access_channels(websocketpp::log::alevel::all);
        c.clear_access_channels(websocketpp::log::alevel::frame_payload);

        // 初始化ASIO
        c.init_asio();

        // 设置TLS初始化回调
        c.set_tls_init_handler(bind(&on_tls_init));

        // 注册消息处理回调
        c.set_message_handler(&on_message);

        // 获取websocket连接，并添加相关异常处理
        websocketpp::lib::error_code ec;
        std::string uri = "wss://stream.binance.com:443/ws/bomeusdt@trade";
        client::connection_ptr con = c.get_connection(uri, ec);

        if (ec)
        {
            std::cout << "Could not create connection because: " << ec.message() << std::endl;
            return -1;
        }

        // 连接到服务器
        c.connect(con);

        // 开始ASIO io_service事件循环
        c.run();
    }
    catch (websocketpp::exception const & e)
    {
        std::cout << e.what() << std::endl;
    }
    catch (const std::exception& e)
    {
        std::cout << e.what() << std::endl;
    }