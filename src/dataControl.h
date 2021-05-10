/*
 * @Author: xtt
 * @Date: 2020-12-02 14:56:17
 * @Description: ...
 * @LastEditTime: 2020-12-06 21:55:16
 */
#include <iostream>
class DataControl
{
private:
        DataControl() = delete;
        ~DataControl() {}

public:
        static void setRgbData(void *pdata, int size, uint32_t id);
        static void *getRgbData();
        static void clearRgbData();
};