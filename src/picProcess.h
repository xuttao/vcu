/*
 * @Author: xtt
 * @Date: 2021-03-01 20:22:39
 * @Description: ...
 * @LastEditTime: 2021-03-01 21:30:22
 */
#pragma once

#include <stdint.h>

uint8_t *resizeImage(const char *file, int dst_w, int dst_h);

uint8_t *resizeRgbBin(uint8_t *data, int width, int height, int dst_w, int dst_h);