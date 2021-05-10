/*
 * @Author: xtt
 * @Date: 2021-04-28 15:17:26
 * @Description: ...
 * @LastEditTime: 2021-05-08 14:39:26
 */
#pragma once

#include <QtSerialPort/QSerialPort>
#include <QtSerialPort/QSerialPortInfo>
#include <functional>

namespace std
{
        class thread;
}

class SerialPortDataParser
{
public:
        virtual void *parser(const char *_data, int _len) = 0;
};

unsigned short crc16_code(const unsigned char *buf, const int len);

class SerialPort : public QObject
{
        Q_OBJECT
private:
        const static int BUFFER_SIZE = 1024;

public:
        struct PortParam
        {
                const char *name;
                int baudrate;
                int databits;
                int stopbits;
                PortParam() = default;
                PortParam(const char *_n, int b, int d, int s) : name(_n), baudrate(b), databits(d), stopbits(s) {}
        };

public:
        SerialPort(QObject *parent = NULL) {}
        explicit SerialPort(const PortParam &param);
        virtual ~SerialPort()
        {
                if (port.isOpen()) port.close();
        }
        SerialPort(const SerialPort &) = delete;
        SerialPort &operator=(const SerialPort &) = delete;

private:
        std::function<void(uint8_t *, int, void *)> pf_callback;
        PortParam portParam;
        QSerialPort port;
        uint8_t buffer[BUFFER_SIZE] = {0};
        void *call = nullptr;
private slots:
        void readData();
        void sendData(QByteArray);
        void portError(QSerialPort::SerialPortError error);
signals:
        void sigSend(QByteArray);

public:
        void setPortParam(const PortParam &param) { portParam = param; }
        void setCallBack(std::function<void(uint8_t *, int, void *)> _pf, void *arg)
        {
                pf_callback = _pf;
                call = arg;
        }
        bool open();
        void close();
        void send(const char *_p, int _len);
};