#!/bin/bash

## 配置工程路径 ##
QTP_HOME="/home/Binance/BiQTP/"
QTP_NAME="BiQTP"

BIN_DIR="${QTP_HOME}bin/"
BIN_PATH="${QTP_HOME}bin/${QTP_NAME}"
CFG_FILE="${QTP_HOME}etc/QTP.ini"
##LIB_PATH="${QTP_HOME}lib"

OUT_FILE="${QTP_HOME}BiQTP.out"
PID_FILE="${QTP_HOME}BiQTP.pid"


## 指定动态库搜索路径 ##
##export LD_LIBRARY_PATH=/usr/local/lib:${LIB_PATH}


## 返回进程运行状态 ##
function is_running()
{
    # if 0为真 运行then逻辑
    if [ -r "$PID_FILE" ]
    then
        if [ -n "$(ps -o pid= -p "$(cat "$PID_FILE")")" ]
        then
            return 0 # true
        fi
    fi
    return 1 # false
}

## 后台运行进程 ##
function run()
{
    #"$BIN_PATH" "$CFG_FILE" &> "$OUT_FILE" &
    ./"$(basename "$BIN_PATH")" "$CFG_FILE" &> "$OUT_FILE" &
    echo $! > "$PID_FILE"
    wait &> /dev/null
    rm "$PID_FILE"
}

## 启动进程 ##
function start()
{
    if is_running
    then
        echo "$QTP_NAME: already running"
        exit 0
    fi

    cd "$(dirname "$BIN_PATH")" &&
    run &
    disown -h %1
    echo "$QTP_NAME: started"
}

## 杀死进程 (SIGKILL强杀) ##
function stop()
{
    if ! is_running
    then
        echo "$QTP_NAME: not running"
        exit 0
    fi

    pid="$(cat "$PID_FILE")"
    echo "$QTP_NAME: stopping $pid"
    kill "$pid"
}

## 杀死进程 (SIGINT中止所有母单) ##
function expire()
{
    if ! is_running
    then
        echo "$QTP_NAME: not running"
        exit 0
    fi

    pid="$(cat "$PID_FILE")"
    echo "$QTP_NAME: expiring $pid"
    kill -SIGINT "$pid"
}

## 重启进程 (保留母单状态) ##
function restart()
{
    stop
    sleep 2
    start
}

## 获取QTS状态 ##
function status()
{
    if is_running
    then
        pid="$(cat "$PID_FILE")"
        echo "$QTP_NAME: $pid is running"
    else
        echo "$QTP_NAME: stopped"
    fi
}

## 测试函数 ##
function test()
{
    echo "$QTP_HOME"
    echo "$BIN_DIR"
    echo "$BIN_PATH"
}

#----------------------------------------#
#   Shell Main Entry
#----------------------------------------#
case "$1" in
    start)
        start
    ;;
    stop)
        stop
    ;;
    expire)
        expire
    ;;
    status)
        status
    ;;
    restart)
        restart
    ;;
    test)
        test
    ;;
    *)
        echo "Usage: $0 {start|stop|expire|restart|status|test}"
    ;;
esac
