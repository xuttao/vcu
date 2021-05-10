#pragma once

#include "serialPortDataManager.h"
#include "turbojpeg.h"
#include <QSemaphore>
#include <opencv2/opencv.hpp>
#include <pthread.h>
#include <semaphore.h>

// #include <arm_neon.h>

// void *memcpy_128(void *dest, void *src, size_t count)
// {
//         int i;
//         unsigned long *s = (unsigned long *)src;
//         unsigned long *d = (unsigned long *)dest;
//         for (i = 0; i < count / 64; i++)
//         {
//                 vst1q_u64(&d[0], vld1q_u64(&s[0]));
//                 vst1q_u64(&d[2], vld1q_u64(&s[2]));
//                 vst1q_u64(&d[4], vld1q_u64(&s[4]));
//                 vst1q_u64(&d[6], vld1q_u64(&s[6]));
//                 d += 8;
//                 s += 8;
//         }
//         return dest;
// }

// detect_h = detect_w = 416;
// int len = 0;
// char *pYUV = readBinaryFile("y420.bin", len);
// DecodeParam para;
// para.height = 765;
// para.width = 1360;
// for (int i = 0; i < 10000; i++)
// {
//         vcu_callback(pYUV, len, para);
// }

// return 0;

// yuv_to_rgb(const uint8_t *pYUV, int len, uint16_t src_height, uint16_t src_width, RgbData2 &rgbRes);

// cv::Mat mat = cv::imread("/media/xtt/hdd/code/fpgaSim_v4tiny/image/car.jpg", cv::IMREAD_COLOR);
// cv::Mat dst;
// cv::cvtColor(mat, dst, cv::COLOR_RGB2BGR);

// int width = dst.cols;
// int height = dst.rows;

// uint8 *i420_image = NULL;
// unsigned long l = width * height * 1.5;
// i420_image = (uint8 *)malloc(width * height * 1.5);
// RGB2YUV(dst.data, dst.cols, dst.rows, i420_image, &l);

// std::ofstream of3("y420x.bin", std::ofstream::binary);
// of3.write((char *)i420_image, width * height * 1.5);
// of3.close();

// int width = dst.cols;
// int height = dst.rows;

// uint8 *i420_image = NULL;
// i420_image = (uint8 *)malloc(width * height * 1.5);
// uint8 *i420_image_y_ptr = i420_image;
// uint8 *i420_image_u_ptr = i420_image_y_ptr + (width * height);
// uint8 *i420_image_v_ptr = i420_image_u_ptr + (int)(width * height * 0.25);

// libyuv::RGB24ToI420(
//     dst.data, (width * 3),
//     i420_image_y_ptr, width,
//     i420_image + height * width, width / 2,
//     i420_image + height * width + (height * width) / 4, width / 2,
//     width, height);

// std::ofstream of2("y420.bin", std::ofstream::binary);
// of2.write((char *)i420_image, width * height * 1.5);
// of2.close();

// detect_h = detect_w = 416;

// cv::Mat mat = cv::imread("/media/xtt/hdd/code/fpgaSim_v4tiny/image/car.jpg", cv::IMREAD_COLOR);
// cv::Mat dst;
// cv::cvtColor(mat, dst, cv::COLOR_RGB2BGR);
// uint8_t *pp = resizeRgbBin(dst.data, dst.cols, dst.rows, 416, 416);

// std::ofstream of("rgb416_darkresize.bin", std::ofstream::binary);
// of.write((char *)pp, 416 * 416 * 3);
// of.close();

// RgbData2 RGB_data(dst.rows, dst.cols);
// RGB_data.scale_h = 234,
// RGB_data.scale_w = 416;
// RGB_data.pData = pp;

// auto *pPad2 = pad_data(RGB_data);

// std::ofstream of2("rgb416_2.bin", std::ofstream::binary);
// of2.write((char *)pPad2, 416 * 416 * 4);
// of2.close();

// int width = 1360;
// int height = 765;
// int len = 0;
// uint8_t *pYUV = (uint8_t *)readBinaryFile("/media/xtt/hdd/code/fservo-mutilplatform-vcu/y420.bin", len);
// RgbData2 RGB_data(height, width);
// yuv_to_rgb(pYUV, len, height, width, RGB_data);

// uint8_t *pdet = resizeRgbBin(RGB_data.pData, width, height, 416, 416);

// std::ofstream of("rgb416_last.bin", std::ofstream::binary);
// of.write((char *)pdet, 416 * 416 * 4);
// of.close();

// detect_h = detect_w = 416;
// RgbData2 RGB_data(height, width);
// yuv_to_rgb_scale((uint8_t *)pyuv, width * height * 1.5, height, width, RGB_data);

// std::ofstream of("rgb416_new.bin", std::ofstream::binary);
// of.write((char *)RGB_data.pData, 416 * 416 * 3);
// of.close();

// auto *pPad = pad_data(RGB_data);

// std::ofstream of("rgb416.bin", std::ofstream::binary);
// of.write((char *)pPad, 416 * 416 * 4);
// of.close();

// libyuv::I420Scale(ySrc, src_width, uSrc, src_width >> 1, vSrc, src_width >> 1,
//                   src_width, src_height,
//                   yDst, real_width, uDst, real_width >> 1, vDst, real_width >> 1,
//                   real_width, real_height,
//                   libyuv::FilterMode::kFilterBilinear);

namespace
{
        struct SemControl
        {
                // std::condition_variable semCond;
                // std::mutex  semMutex;

                QSemaphore sem_input;
                sem_t psem2;
        };

        SemControl pSem[8];
} // namespace

#include "../socketbase.h"
void sockettest()
{
        SocketUdp udp;
        udp.createUdpClient("192.168.3.132", 4000, 7780);
        char buffer[1024];
        for (;;)
        {
                int ret = udp.read(buffer, 2);
                int a = 0;
                a++;
        }

        SocketTcp tcpSocket;
        bool res = tcpSocket.connetToServer("192.168.3.132", 4000);
        tcpSocket.setSendTimeout(1, 0);
        // tcpSocket.setSendBuffer(20 * 1024);
        // tcpSocket.setRecvTimeout(1)
        // char buffer[1024];
        char *p = "hello";
        for (;;)
        {
                // tcpSocket.send(p, strlen(p) + 1);    //非阻塞模式返回-1
                int ret = tcpSocket.read(buffer, 1); //非阻塞模式返回-1
                if (ret < 0)
                {
                        if (errno == EWOULDBLOCK || errno == EINTR) //非阻塞模式，ret是-1,这个状态表示连接正常
                        {
                                sleep(1);
                                continue;
                        }
                        else //非阻塞模式下，这种情况表示连接断开
                        {
                                int a = 0;
                                a++;
                        }
                }
                else if (ret == 0)
                {
                        int a = 0;
                        a++;
                }

                fprintf(stderr, "----len:%d，%x\n", ret, buffer[0]);

                sleep(1);
        }
}

void sockettest2()
{
        SocketTcp tcpSocket;
        tcpSocket.createTcpServer("192.168.3.110", 5000);
        tcpSocket.waitBeConnected();
        tcpSocket.setRecvTimeout(0, 50);
        char buffer[1024];
        for (;;)
        {

                int ret = tcpSocket.read(buffer, 5); //非阻塞模式下，并不是要收到指定长度才会返回，而是看接收缓冲区里有没有数据，如果正在接收，
                //则一直接收，接收完毕，就会返回
                if (ret > 0) //阻塞模式下，断开返回-1
                {
                        int a = 0;
                        a++;
                }
                else
                {
                        // int a = 0; //非阻塞模式下，没有数据返回-1
                        // a++;
                        // if (errno == EWOULDBLOCK || errno == EINTR) //非阻塞模式，ret是-1,这个状态表示连接正常
                        // {
                        //         sleep(1);
                        //         continue;
                        // }
                        if (tcpSocket.getConnectStatus() == SocketTcp::CONNECTED)
                        {
                                sleep(1);
                                int a = 0;
                                a++;
                        }
                        else
                        {
                                int a = 0; //非阻塞模式下，连接断开
                                a++;
                        }
                }

                sleep(1);
        }
}

bool RGB2YUV(uint8_t *RgbBuf, int nWidth, int nHeight, uint8_t *yuvBuf, unsigned long *len)
{
        if (RgbBuf == NULL)
        {
                return FALSE;
        }

        int i, j;
        unsigned char *bufY, *bufU, *bufV, *bufRGB;
        memset(yuvBuf, 0, (unsigned int)*len);
        bufY = yuvBuf;
        bufV = yuvBuf + nWidth * nHeight;
        bufU = bufV + (nWidth * nHeight * 1 / 4);
        *len = 0;
        unsigned char y, u, v, r, g, b;
        unsigned int ylen = nWidth * nHeight;
        unsigned int ulen = (nWidth * nHeight) / 4;
        unsigned int vlen = (nWidth * nHeight) / 4;
        for (j = 0; j < nHeight; j++)
        {
                bufRGB = RgbBuf + nWidth * (nHeight - 1 - j) * 3;
                for (i = 0; i < nWidth; i++)
                {
                        int pos = nWidth * i + j;
                        r = *(bufRGB++);
                        g = *(bufRGB++);
                        b = *(bufRGB++);
                        y = (unsigned char)((66 * r + 129 * g + 25 * b + 128) >> 8) + 16;
                        u = (unsigned char)((-38 * r - 74 * g + 112 * b + 128) >> 8) + 128;
                        v = (unsigned char)((112 * r - 94 * g - 18 * b + 128) >> 8) + 128;
                        *(bufY++) = std::max<uint8_t>(0, std::min<int>(y, 255));
                        if (j % 2 == 0 && i % 2 == 0)
                        {
                                if (u > 255)
                                {
                                        u = 255;
                                }
                                if (u < 0)
                                {
                                        u = 0;
                                }
                                *(bufU++) = u;
                                //存u分量
                        }
                        else
                        {
                                //存v分量
                                if (i % 2 == 0)
                                {
                                        if (v > 255)
                                        {
                                                v = 255;
                                        }
                                        if (v < 0)
                                        {
                                                v = 0;
                                        }
                                        *(bufV++) = v;
                                }
                        }
                }
        }
        *len = nWidth * nHeight + (nWidth * nHeight) / 2;

        return TRUE;
}

void serialport_test()
{
        // SerialPort::PortParam param("/dev/ttyPS1", 9600, 8, 1);
        // SerialPortDataManager::getInstance()->init(param, serialport_callback, NULL);
        // SerialPortDataManager::getInstance()->start();
        // for (int i = 0; i < 60 * 10; i++)
        // {
        //         sleep(1);
        //         if (i % 10 == 0)
        //         {
        //                 char p[6] = {0x01, 0x02, 0x03, 0xff, 0xef, 0xa0};
        //                 SerialPortDataManager::getInstance()->send(p, 6);
        //         }
        // }
}

inline void opencv_load()
{
        FPGAInput *pInput = new FPGAInput;
        IplImage *m = cvLoadImage("/media/xtt/hdd/temp/chuan/trainimg0001.jpg", CV_LOAD_IMAGE_COLOR);

        pInput->in_data = (unsigned char *)malloc(640 * 512 * 3);

        pInput->in_data_len = 640 * 512 * 3;

        //pInput->picName=file_path;

        uint64_t source_loc = 0;
        uint64_t w_st = 0;
        uint8_t *cv_ptr = (uint8_t *)m->imageData;

        uchar *pdes = pInput->in_data;
        for (int h = 0; h < m->height; ++h)
        {
                uchar *p_pixel = cv_ptr + h * m->widthStep;
                pdes = pInput->in_data + h * m->widthStep;
                for (int w = 0; w < m->width; w++)
                {
                        // unsigned char r=(((p_pixel[2])/(float)255.0-0.485)/0.229*127)/3.9031754+128.0+0.5;
                        // unsigned char g=(((p_pixel[1])/(float)255.0-0.456)/0.224*127)/3.9031754+128.0+0.5;
                        // unsigned char b=(((p_pixel[0])/(float)255.0-0.406)/0.225*127)/3.9031754+128.0+0.5;

                        unsigned char r = p_pixel[2];
                        unsigned char g = p_pixel[1];
                        unsigned char b = p_pixel[0];

                        b = b > 255 ? 255 : b;
                        g = g > 255 ? 255 : g;
                        r = r > 255 ? 255 : r;

                        pdes[0] = r;
                        pdes[1] = g;
                        pdes[2] = b;

                        p_pixel += 3;
                        pdes += 3;
                }
        }

        cvReleaseImage(&m);
        FILE *p = fopen("./trainimg0001_512__640_3.bin", "wb");
        fwrite(pInput->in_data, 1, 640 * 512 * 3, p);
        fclose(p);

        free(pInput->in_data);
}

inline void freeimage_load()
{
        // auto startTime=get_current_Mtime();
        // FreeImage_Initialise(TRUE);
        // //加载图片
        // FIBITMAP * JPEG = FreeImage_Load(FIF_JPEG, "/home/detection/img/trainimg0102.jpg", JPEG_DEFAULT);
        // //获取影像的宽高，都以像素为单位
        // int Width = FreeImage_GetWidth(JPEG);
        // int Height = FreeImage_GetHeight(JPEG);
        // std::cout << "Width: " << Width << "\nHeight: " << Height << std::endl;

        // uchar *bits = FreeImage_GetBits(JPEG);

        // unsigned char* in_data=(unsigned char*)malloc(640*512*3);

        // //FreeImage_FlipVertical(JPEG);
        // uchar *bits = FreeImage_GetBits(JPEG);
        // uchar *pdes=in_data;
        // int num=Width*Height;
        // uint num_1=0;
        // uint num_2=0;

        // for(int h=Height-1;h>=0;h--){

        //     uint c1=h*Width;
        //     uint c2=(Height-h-1)*Width;

        //     for(int w=0;w<Width;w++){
        //         num_1=(c1+w)*3;
        //         num_2=(c2+w)*3;

        //         pdes[num_2]= bits[num_1+2];
        //         pdes[num_2+1]= bits[num_1+1];
        //         pdes[num_2+2]= bits[num_1];

        //     }
        // }

        // FILE* p=fopen("./my.bin","wb");
        // fwrite(in_data,1,640*512*3,p);
        // fclose(p);

        // free(in_data);

        // if (JPEG)
        // {
        //     FreeImage_Save(FIF_JPEG, JPEG, "./11.jpg", JPEG_DEFAULT);
        // }

        // FreeImage_Unload(JPEG);
        // FreeImage_DeInitialise();

        // auto endTime=get_current_Mtime();
        // std::cout<<"----------------"<<"Time:"<<endTime-startTime<<std::endl;
}

inline void turbo_load()
{
        // struct jpeg_decompress_struct cinfo;
        // struct jpeg_error_mgr jerr;

        // FILE * infile;/* source file */
        // JSAMPARRAY buffer;/* Output row buffer */
        // int row_stride;/* physical row width in output buffer */

        // std::string strImageName;

        // if ((infile = fopen(strImageName.c_str(), "rb")) == NULL) {
        //       fprintf(stderr, "can't open %s\n", strImageName);
        //      return -1;
        // }

        // cinfo.err = jpeg_std_error(&jerr);

        // jpeg_create_decompress(&cinfo);

        // jpeg_stdio_src(&cinfo, infile);

        // jpeg_read_header(&cinfo, TRUE);

        // printf("image_width = %d\n", cinfo.image_width);
        // printf("image_height = %d\n", cinfo.image_height);
        // printf("num_components = %d\n", cinfo.num_components);

        // printf("enter scale M/N:\n");

        // cinfo.scale_num = 1;
        // cinfo.scale_denom = 1;
        // printf("scale to : %d/%d\n", cinfo.scale_num, cinfo.scale_denom);

        // jpeg_start_decompress(&cinfo);

        // printf("output_width = %d\n", cinfo.output_width);
        // printf("output_height = %d\n", cinfo.output_height);
        // printf("output_components = %d\n", cinfo.output_components);

        // row_stride = cinfo.output_width * cinfo.output_components;

        // buffer = (*cinfo.mem->alloc_sarray)((j_common_ptr)&cinfo, JPOOL_IMAGE, row_stride, 1);

        // while (cinfo.output_scanline < cinfo.output_height) {

        //     jpeg_read_scanlines(&cinfo, buffer, 1);

        // }

        // jpeg_finish_decompress(&cinfo);

        // jpeg_destroy_decompress(&cinfo);

        // fclose(infile);
        // return 0;
}

inline void jpeg_turbo_load(std::vector<cv::String> vecFiles)
{
        //#if 0
        auto startTime = get_current_Mtime();

        using namespace std;

        for (int i = 0; i < vecFiles.size(); i++)
        {
                const char *pFilePath = vecFiles[i].c_str();

                ifstream inStream(pFilePath, ifstream::binary);
                inStream.seekg(0, inStream.end);
                int length = inStream.tellg();
                inStream.seekg(0, inStream.beg);

                uchar *img_buffer = new uchar[length];
                inStream.read((char *)img_buffer, length);

                inStream.close();

                std::cout << vecFiles[i] << std::endl;

                int width = 0;
                int height = 0;
                int JPEG_QUALITY = 75;
                int COLOR_COMPONENTS = 3;

                tjscalingfactor scalingFactor = {1, 2};
                tjscalingfactor *scalingFactors = NULL;

                int numScalingFactors = 0;
                if ((scalingFactors = tjGetScalingFactors(&numScalingFactors)) == NULL)
                {
                        return;
                }

                // for(int i=0;i<numScalingFactors;i++){
                //     auto d1=scalingFactors[i].num;
                //     auto d2=scalingFactors[i].denom;
                //     auto d3=scalingFactors[i];
                //     std::cout<<d1<<"---"<<d2<<std::endl;
                // }

                tjhandle jpeg_handle = tjInitDecompress();

                tjDecompressHeader2(jpeg_handle, img_buffer, length, &width, &height, &JPEG_QUALITY);

                width = TJSCALED(width, scalingFactor);
                height = TJSCALED(height, scalingFactor);

                uchar *rgb_buffer = new uchar[width * height * 3];
                memset(rgb_buffer, 0, width * height * 3);
                tjDecompress2(jpeg_handle, img_buffer, length, rgb_buffer, width, 0, height, TJPF_RGB, TJFLAG_FASTUPSAMPLE);
                // auto cc=tjGetErrorStr();
                // if(rgb_buffer[0]==0||rgb_buffer[10]==0){
                //     auto cc=tjGetErrorStr();
                //     std::cout<<"ssss"<<cc<<std::endl;

                // }
                // std::stringstream ostr;
                // ostr<<"./out/resize"<<i<<".bmp";
                // auto pixelFormat = TJPF_BGRX;
                // tjSaveImage(ostr.rdbuf()->str().c_str(), rgb_buffer, width, 0, height, 3, 0);

                tjDestroy(jpeg_handle);

                //std::cout << "Width: " << width << "\nHeight: " << height << std::endl;

                // uchar* pdes=new uchar[640*512*3];
                // for(int i=0;i<640*512*3;){

                //     unsigned char r=(((rgb_buffer[i])/(float)255.0-0.485)/0.229*127)/3.7077868+128.0+0.5;
                //     unsigned char g=(((rgb_buffer[i+1])/(float)255.0-0.456)/0.224*127)/3.7077868+128.0+0.5;
                //     unsigned char b=(((rgb_buffer[i+2])/(float)255.0-0.406)/0.225*127)/3.7077868+128.0+0.5;

                //     pdes[i]     =r;
                //     pdes[i+1]   =g;
                //     pdes[i+2]   =b;

                //     i+=3;
                // }

                // FILE* p=fopen("./hwc001.bin","wb");
                // fwrite(rgb_buffer,1,width*height*3,p);
                // fclose(p);

                //std::string pFilePath();
                //tjSaveImage("", imgBuf, width, 0, height, pixelFormat, 0)
                delete[] img_buffer;
                delete[] rgb_buffer;

                //tjDestroy(jpeg_handle);
        }

        auto endTime = get_current_Mtime();
        std::cout << "----------------"
                  << "Time:" << endTime - startTime << std::endl;
        //#endif

#if 0
    tjscalingfactor scalingFactor = { 1, 2 };
	int outSubsamp = -1, outQual = -1;
	tjtransform xform;
    tjscalingfactor *scalingFactors = NULL;

	int flags = 0;
	int width, height;
	char *inFormat, *outFormat = "bmp";
	FILE *jpegFile = NULL;
	unsigned char *imgBuf = NULL, *jpegBuf = NULL;
	int retval = 0, i, pixelFormat = TJPF_UNKNOWN;
	tjhandle tjInstance = NULL;
    int numScalingFactors=0;
	if ((scalingFactors = tjGetScalingFactors(&numScalingFactors)) == NULL)
		LOG_ERR("err");
	memset(&xform, 0, sizeof(tjtransform));

	clock_t start, finish;
	double  duration;

	start = clock();

	
	flags |= TJFLAG_FASTDCT;   // �Ƿ���fastDCT
	flags |= TJFLAG_FASTUPSAMPLE;

	for (int iImg = 1; iImg < 301; iImg++)
	{
		/* Input image is a JPEG image.  Decompress and/or transform it. */
		long size;
		int inSubsamp, inColorspace;
		int doTransform = (xform.op != TJXOP_NONE || xform.options != 0 ||
			xform.customFilter != NULL);
		unsigned long jpegSize;

		/* Read the JPEG file into memory. */
		char bufNameIn[256];
		sprintf(bufNameIn, "/media/xtt/hdd/temp/chuan/trainimg%04d.jpg", iImg);
        LOG_INFO("%s",bufNameIn);

		if ((jpegFile = fopen(bufNameIn, "rb")) == NULL)
			LOG_ERR("err");
		if (fseek(jpegFile, 0, SEEK_END) < 0 || ((size = ftell(jpegFile)) < 0) ||
			fseek(jpegFile, 0, SEEK_SET) < 0)
			LOG_ERR("err");
		if (size == 0)
			LOG_ERR("err");
		jpegSize = (unsigned long)size;
		if ((jpegBuf = (unsigned char *)tjAlloc(jpegSize)) == NULL)
			LOG_ERR("err");
		if (fread(jpegBuf, jpegSize, 1, jpegFile) < 1)
			LOG_ERR("err");
		fclose(jpegFile);  jpegFile = NULL;

		if (doTransform) 
		{
			/* Transform it. */
			unsigned char *dstBuf = NULL;  /* Dynamically allocate the JPEG buffer */
			unsigned long dstSize = 0;

			if ((tjInstance = tjInitTransform()) == NULL)
				LOG_ERR("err");
			xform.options |= TJXOPT_TRIM;
			if (tjTransform(tjInstance, jpegBuf, jpegSize, 1, &dstBuf, &dstSize, &xform, flags) < 0)
				LOG_ERR("err");
			tjFree(jpegBuf);
			jpegBuf = dstBuf;
			jpegSize = dstSize;
		}
		else 
		{
			if ((tjInstance = tjInitDecompress()) == NULL)
				LOG_ERR("err");
		}

		if (tjDecompressHeader3(tjInstance, jpegBuf, jpegSize, &width, &height,
			&inSubsamp, &inColorspace) < 0)
			LOG_ERR("err");

		// printf("%s Image:  %d x %d pixels, %s subsampling, %s colorspace\n",
		// 	(doTransform ? "Transformed" : "Input"), width, height,
		// 	subsampName[inSubsamp], colorspaceName[inColorspace]);

		/* Scaling and/or a non-JPEG output image format and/or compression options
		have been selected, so we need to decompress the input/transformed
		image. */
		width = TJSCALED(width, scalingFactor);
		height = TJSCALED(height, scalingFactor);
// 		width = 640/3;
// 		height = 512/3;
		if (outSubsamp < 0)
			outSubsamp = inSubsamp;

		pixelFormat = TJPF_BGRX;
		if ((imgBuf = (unsigned char *)tjAlloc(width * height *
			tjPixelSize[pixelFormat])) == NULL)
			LOG_ERR("err");

		if (tjDecompress2(tjInstance, jpegBuf, jpegSize, imgBuf, width, 0, height,
			pixelFormat, flags) < 0)
			LOG_ERR("err");
		tjFree(jpegBuf);  jpegBuf = NULL;
		tjDestroy(tjInstance);  tjInstance = NULL;


		//printf("Output Image (%s):  %d x %d pixels\n", outFormat, width, height);


		/* Output image format is not JPEG.  Save the uncompressed image
		directly to disk. */

		// char bufNameOut[256];
		// sprintf(bufNameOut, "K:/image/trainimg%04d.bmp", iImg);
		// printf("\n");
		// if (tjSaveImage(bufNameOut, imgBuf, width, 0, height, pixelFormat, 0) < 0)
        // LOG_ERR("err");

			//THROW_TJ("saving output image");

	}

	finish = clock();
	duration = (double)(finish - start) / CLOCKS_PER_SEC;
	printf("%f seconds\n", duration);



bailout:
	tjFree(imgBuf);
	if (tjInstance) tjDestroy(tjInstance);
	tjFree(jpegBuf);
	if (jpegFile) fclose(jpegFile);
	return ;
#endif
}

inline void resize_bin()
{
        unsigned char *data = new unsigned char[416 * 416 * 4];
        memset(data, 0, 416 * 416 * 4);
        int file_size = 0;

        // write jpg png to share memory
        int dst_width = 416;
        int dst_heigh = 416;
        //IplImage *m = cvLoadImage("/media/xtt/hdd/old/temp/chuan/trainimg0023.jpg", CV_LOAD_IMAGE_COLOR);
        IplImage *m = cvLoadImage("/media/xtt/hdd/temp/chuan/trainimg0001.jpg", CV_LOAD_IMAGE_COLOR);

        float scale = std::min<float>(static_cast<float>(dst_width) / (m->width * 1.0f), static_cast<float>(dst_heigh) / (m->height * 1.0f));
        CvSize size;
        size.width = static_cast<int>(scale * m->width);
        size.height = static_cast<int>(scale * m->height);
        IplImage *dst = cvCreateImage(size, m->depth, m->nChannels);
        cvResize(m, dst, CV_INTER_LINEAR);

        uint8_t *cv_ptr = (uint8_t *)dst->imageData;
        int begin_w = (dst_width - size.width) / 2;
        int begin_h = (dst_heigh - size.height) / 2;
        int end_h = size.height + begin_h;
        int end_w = size.width + begin_w;

        int data_size = dst_width * dst_heigh;
        //unsigned char* output_data = (unsigned char*)malloc(dst_width*dst_width*RBGA_PIXEL_BYTES);

        uint64_t source_loc = 0;
        uint64_t w_st = 0;

        for (int h = 0; h < dst_heigh; ++h)
        {
                for (int w = 0; w < dst_width; w++)
                {
                        w_st = (h * dst_width + w) * 4;
                        if (h >= begin_h && h < end_h && w >= begin_w && w < end_w)
                        {
                                //r g b
                                source_loc = (h - begin_h) * dst->widthStep + (w - begin_w) * dst->nChannels;

                                // data[w_st]     = g_r_table[cv_ptr[source_loc + 2]];
                                // data[w_st + 1] = g_g_table[cv_ptr[source_loc + 1]];
                                // data[w_st + 2] = g_b_table[cv_ptr[source_loc]];

                                unsigned char r = (((cv_ptr[source_loc + 2]) / (float)255.0 - 0.485) / 0.229 * 127) / 3.9031754 + 128.0 + 0.5;
                                unsigned char g = (((cv_ptr[source_loc + 1]) / (float)255.0 - 0.456) / 0.224 * 127) / 3.9031754 + 128.0 + 0.5;
                                unsigned char b = (((cv_ptr[source_loc]) / (float)255.0 - 0.406) / 0.225 * 127) / 3.9031754 + 128.0 + 0.5;

                                // unsigned char r=(((cv_ptr[source_loc + 2])/(float)255.0-0.485)/0.229*127)/3.7077868+128.0+0.5;
                                // unsigned char g=(((cv_ptr[source_loc + 1])/(float)255.0-0.456)/0.224*127)/3.7077868+128.0+0.5;
                                // unsigned char b=(((cv_ptr[source_loc])/(float)255.0-0.406)/0.225*127)/3.7077868+128.0+0.5;

                                // unsigned char r=(cv_ptr[source_loc + 2]);
                                // unsigned char g=(cv_ptr[source_loc + 1]);
                                // unsigned char b=(cv_ptr[source_loc]);

                                b = b > 255 ? 255 : b;
                                g = g > 255 ? 255 : g;
                                r = r > 255 ? 255 : r;

                                data[w_st] = r;
                                data[w_st + 1] = g;
                                data[w_st + 2] = b;
                        }
                        else
                        {
                                // data[w_st] = (((128.0)/(float)255.0-0.485)/0.229*127)/3.7077868+128.0+0.5;
                                // data[w_st + 1] = (((128.0)/(float)255.0-0.456)/0.224*127)/3.7077868+128.0+0.5;
                                // data[w_st + 2] = (((128.0)/(float)255.0-0.406)/0.225*127)/3.7077868+128.0+0.5;

                                data[w_st] = 128.0;
                                data[w_st + 1] = 128.0;
                                data[w_st + 2] = 128.0;
                        }
                }
        }
        // 导出输入

        FILE *fp = fopen("xtt-hwctrainimg0118-gy.bin", "wb");
        fwrite(data, 1, 416 * 416 * 4, fp);
        fclose(fp);

        //if (output_data) free(output_data);
        if (m) cvReleaseImage(&m);
        if (dst) cvReleaseImage(&dst);
}

inline void pic_bin()
{
}

void *f1()
{
        int i = 0;
        for (int j = 0; j < 1000; j++)
        {
                pSem[i % 8].sem_input.acquire();
                i++;
                LOG_INFO("----%d", i);
        }
}

void *f2()
{
        int i = 0;
        for (int j = 0; j < 1000; j++)
        {
                pSem[i % 8].sem_input.release();
                i++;
        }
}

void *f3(void *)
{
        int i = 0;
        for (int j = 0; j < 1000; j++)
        {
                sem_wait(&pSem[i & 7].psem2);
                i++;
                LOG_INFO("----%d", i);
        }
}

void *f4(void *)
{
        int i = 0;
        for (int j = 0; j < 1000; j++)
        {
                sem_post(&pSem[i & 7].psem2);
                i++;
        }
}

inline void fun()
{
        auto t1 = get_current_Mtime();
        std::thread th1(f1);
        std::thread th2(f2);
        th2.join();
        th1.join();
        auto t2 = get_current_Mtime();
        std::cout << "----" << t2 - t1 << std::endl;

        // std::stringstream file;
        // std::fstream stream;
        // stream.open("file.txt", std::fstream::in);
        // file << stream.rdbuf();
        // stream.close();

        // auto t1=get_current_Mtime();
        // pthread_t th1;
        // pthread_t th2;
        // pthread_create(&th1,NULL,f3,NULL);
        // pthread_create(&th2,NULL,f4,NULL);
        // pthread_join(th1,NULL);
        // pthread_join(th2,NULL);
        // auto t2=get_current_Mtime();
        // std::cout<<"----"<<t2-t1<<std::endl;

        // int batchNum=640*512*3/4096;
        // int batch=4096;

        // char* p1=new char[640*512*3];
        // char* p2=new char[640*512*3];

        // memset(p1,1,640*512*3);

        // memcpy(p2,p1,640*512*3);
        // FileManager::getInstance()->writeBinaryFile(p2,640*512*3,"./ss.bin");

        // neon_memcpy(p2,p1,640*512*3);

        // FileManager::getInstance()->writeBinaryFile(p2,640*512*3,"./neon.bin");

        // auto t1=get_current_Mtime();
        // for(int j=0;j<300;j++)
        // for(int i=0;i<batchNum;i++){
        //     memcpy(p1+i*batch,p2+i*batch,batch);
        // }
        // auto t2=get_current_Mtime();
        // std::cout<<"----"<<t2-t1<<std::endl;
        // for(int j=0;j<300;j++)
        // memcpy(p1,p2,640*512*3);
        //std::cout<<"----"<<get_current_Mtime()-t2<<std::endl;
}