#include "picProcess.h"
#include <memory>
#include <opencv2/opencv.hpp>

struct image
{
        int w;
        int h;
        int c;
        float *data = nullptr;
};

static image make_empty_image(int w, int h, int c)
{
        image out;
        out.data = 0;
        out.h = h;
        out.w = w;
        out.c = c;
        return out;
}

static image make_image(int w, int h, int c)
{
        image out = make_empty_image(w, h, c);
        out.data = new float[h * w * c];
        return out;
}

static image mat_to_image(int w, int h, int c, uint8_t *pdata)
{
        image im = make_image(w, h, c);
        unsigned char *data = (unsigned char *)pdata;
        // int step = mat.step;
        int step = w * 3;
        for (int y = 0; y < h; ++y)
        {
                for (int k = 0; k < c; ++k)
                {
                        for (int x = 0; x < w; ++x)
                        {
                                im.data[k * w * h + y * w + x] = data[y * step + x * c + k] / 255.0f;
                        }
                }
        }
        return im;
}

static image copy_image(image p)
{
        image copy = p;
        // copy.data = (float *)malloc(p.h * p.w * p.c * sizeof(float));
        copy.data = new float[p.h * p.w * p.c];
        memcpy(copy.data, p.data, p.h * p.w * p.c * sizeof(float));
        return copy;
}

static float get_pixel(image m, int x, int y, int c)
{
        assert(x < m.w && y < m.h && c < m.c);
        return m.data[c * m.h * m.w + y * m.w + x];
}

static void set_pixel(image m, int x, int y, int c, float val)
{
        if (x < 0 || y < 0 || c < 0 || x >= m.w || y >= m.h || c >= m.c) return;
        assert(x < m.w && y < m.h && c < m.c);
        m.data[c * m.h * m.w + y * m.w + x] = val;
}

static void add_pixel(image m, int x, int y, int c, float val)
{
        assert(x < m.w && y < m.h && c < m.c);
        m.data[c * m.h * m.w + y * m.w + x] += val;
}

static void free_image(image m)
{
        if (m.data)
        {
                delete[] m.data;
        }
}

static image resize_image(image im, int w, int h)
{
        if (im.w == w && im.h == h)
        {
                std::cout << "----------------------err" << std::endl;
                return copy_image(im);
        }

        image resized = make_image(w, h, im.c);
        image part = make_image(w, im.h, im.c);
        int r, c, k;
        float w_scale = (float)(im.w - 1) / (w - 1);
        float h_scale = (float)(im.h - 1) / (h - 1);
        for (k = 0; k < im.c; ++k)
        {
                for (r = 0; r < im.h; ++r)
                {
                        for (c = 0; c < w; ++c)
                        {
                                float val = 0;
                                if (c == w - 1 || im.w == 1)
                                {
                                        val = get_pixel(im, im.w - 1, r, k);
                                }
                                else
                                {
                                        float sx = c * w_scale;
                                        int ix = (int)sx;
                                        float dx = sx - ix;
                                        val = (1 - dx) * get_pixel(im, ix, r, k) + dx * get_pixel(im, ix + 1, r, k);
                                }
                                set_pixel(part, c, r, k, val);
                        }
                }
        }
        for (k = 0; k < im.c; ++k)
        {
                for (r = 0; r < h; ++r)
                {
                        float sy = r * h_scale;
                        int iy = (int)sy;
                        float dy = sy - iy;
                        for (c = 0; c < w; ++c)
                        {
                                float val = (1 - dy) * get_pixel(part, c, iy, k);
                                set_pixel(resized, c, r, k, val);
                        }
                        if (r == h - 1 || im.h == 1) continue;
                        for (c = 0; c < w; ++c)
                        {
                                float val = dy * get_pixel(part, c, iy + 1, k);
                                add_pixel(resized, c, r, k, val);
                        }
                }
        }

        free_image(part);

        return resized;
}

uint8_t *resizeImage(const char *file, int dst_width, int dst_height)
{
#if 0
        cv::Mat mat = cv::imread(file, cv::IMREAD_COLOR);
        cv::Mat dst;
        cv::cvtColor(mat, dst, cv::COLOR_RGB2BGR);

        image im = mat_to_image(mat);

        float scale = std::min<float>(static_cast<float>(dst_width) / im.w,
                                      static_cast<float>(dst_height) / im.h);

        int resize_width = static_cast<int>(scale * im.w);
        int resize_height = static_cast<int>(scale * im.h);

        image resized = resize_image(im, resize_width, resize_height);

        float *pData = resized.data;
        uint8_t *pRes = new uint8_t[resized.h * resized.w * resized.c];

        for (int c = 0; c < resized.c; c++)
        {
                for (int h = 0; h < resized.h; h++)
                {
                        for (int w = 0; w < resized.w; w++)
                        {
                                pRes[c * resized.h * resized.w + h * resized.w + w] = pData[c * resized.h * resized.w + h * resized.w + w];
                        }
                }
        }
#endif
        // return pRes;
}

uint8_t *resizeRgbBin(uint8_t *data, int width, int height, int dst_width, int dst_height)
{
        image im = mat_to_image(width, height, 3, data);

        int resize_width = static_cast<int>(dst_width);
        int resize_height = static_cast<int>(dst_height);

        image resized = resize_image(im, resize_width, resize_height);

        float *pData = resized.data;
        uint8_t *pRes = new uint8_t[resized.h * resized.w * 4];
        memset(pRes, 0, resized.h * resized.w * 4);

        uint64_t num = 0;
        for (int h = 0; h < resized.h; h++)
        {
                for (int w = 0; w < resized.w; w++)
                {
                        for (int c = 0, n = num; c < resized.c; c++, n++)
                        {
                                pRes[n] = pData[c * resized.h * resized.w + h * resized.w + w] * 255.0f + 0.5;
                        }
                        num += 4;
                }
        }

        free_image(resized);
        free_image(im);

        return pRes;
}