/*
 * @Author: xtt
 * @Date: 2021-04-29 15:39:46
 * @Description: ...
 * @LastEditTime: 2021-05-09 16:39:10
 */
#pragma once
#include "serialPort.h"

namespace std
{
        class thread;
}

class SerialPortDataManager
{
public:
        enum PortDataType
        {
                MavLink,
                UnKnow = -1
        };

public:
        SerialPortDataManager() = default;
        ~SerialPortDataManager() = default;
        SerialPortDataManager(const SerialPortDataManager &) = delete;
        SerialPortDataManager &operator=(const SerialPortDataManager &) = delete;

private:
        std::thread *pThread = nullptr;
        SerialPort::PortParam param;
        std::function<void(uint8_t *, int, void *)> pf;
        void *call = nullptr;
        SerialPort *port = nullptr;
        SerialPortDataParser *parser = nullptr;

private:
        static void receive_thread(void *, int argc, char *argv[]);

public:
        // static SerialPortDataManager *getInstance() = delete;
        void init(const SerialPort::PortParam &param, /*std::function<void(uint8_t *, int, void *)> _pf,*/ void *arg);
        void initParser(PortDataType);
        inline SerialPortDataParser *getParser() { return parser; }
        void start();
        void stop();
        void send(const char *_p, int _len);
};