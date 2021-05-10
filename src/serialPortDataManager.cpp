/*
 * @Author: xtt
 * @Date: 2021-04-29 15:40:07
 * @Description: ...
 * @LastEditTime: 2021-05-09 16:39:26
 */
#include "serialPortDataManager.h"
#include "aircraftDataParser.h"
#include "log.h"

#include <QtCore/QCoreApplication>
#include <thread>

static void serialport_callback(uint8_t *data, int len, void *arg)
{
        SerialPortDataParser *pParser = static_cast<SerialPortDataParser *>(arg);
#if 0
        fprintf(stderr, "port receive:");
        for (int i = 0; i < len; i++)
        {
                fprintf(stderr, "%x", data[i]);
        }
        fprintf(stderr, "\n");
#endif
        pParser->parser((char *)data, len);
}

// SerialPortDataManager *SerialPortDataManager::getInstance()
// {
//         static SerialPortDataManager ins;
//         return &ins;
// }

void SerialPortDataManager::init(const SerialPort::PortParam &_param, /*std::function<void(uint8_t *, int, void *)> _pf, */ void *arg)
{
        param = _param;
        // pf = _pf;
        call = arg;
}

void SerialPortDataManager::initParser(PortDataType _type)
{
        if (MavLink == _type)
        {
                parser = new AircraftDataParser;
        }
        else
        {
                LOG_ERR("unknow parser type");
                assert(false);
        }
}

void SerialPortDataManager::start()
{
        int argc = 0;
        char *a = NULL;
        pThread = new std::thread(receive_thread, this, argc, &a);
}

void SerialPortDataManager::stop()
{
        pThread->join();
        delete pThread;
}

void SerialPortDataManager::send(const char *_p, int _len)
{
        port->send(_p, _len);
}

void SerialPortDataManager::receive_thread(void *arg, int argc, char *argv[])
{
        LOG_INFO("serial port thread start id:%lu", std::this_thread::get_id());

        SerialPortDataManager *pThis = static_cast<SerialPortDataManager *>(arg);

        QCoreApplication app(argc, argv);

        pThis->port = new SerialPort;
        pThis->port->setPortParam(pThis->param);
        pThis->port->setCallBack(serialport_callback, pThis->parser);
        bool res = pThis->port->open();
        if (!res)
        {
                LOG_ERR("open %s failed", pThis->param.name);
                return;
        }

        app.exec();
        pThis->port->close();

        LOG_INFO("serial port thread exit");
}
