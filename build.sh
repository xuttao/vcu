###
 # @Author: xtt
 # @Date: 2020-09-21 11:38:46
 # @Description: ...
 # @LastEditTime: 2021-05-09 16:43:41
### 
#!/bin/sh                                                                    
#strings libm.so.6 | grep GLIBC_

#cmake -DCMAKE_C_COMPILER=/usr/bin/clang -DCMAKE_CXX_COMPILER=/usr/bin/clang
#cmake -DOPT_BUILD_ARM64_ENABLE=ON \

# cmake  -DCMAKE_C_COMPILER=/media/xtt/hdd/3rdlib/build/gcc-linaro-arm-linux-gnueabihf-4.9-2014.07_linux/bin/arm-linux-gnueabihf-gcc \
#        -DCMAKE_CXX_COMPILER=/media/xtt/hdd/3rdlib/build/gcc-linaro-arm-linux-gnueabihf-4.9-2014.07_linux/bin/arm-linux-gnueabihf-g++ \
#        -DCMAKE_BUILD_TYPE=Release \
#        -DOPT_BUILD_ARM64_ENABLE=ON \
#        ..

cd ./build

# cmake  -DCMAKE_C_COMPILER=/usr/bin/gcc \
#        -DCMAKE_CXX_COMPILER=/usr/bin/g++ \
#        -DCMAKE_BUILD_TYPE=Debug \
#        -DOPT_BUILD_ARM64_ENABLE=OFF \
#        -DOPT_NEON_ENABLE=OFF \
#        ..


cmake  -DCMAKE_C_COMPILER=/usr/bin/aarch64-linux-gnu-gcc \
       -DCMAKE_CXX_COMPILER=/usr/bin/aarch64-linux-gnu-g++ \
       -DCMAKE_BUILD_TYPE=Release \
       -DOPT_BUILD_ARM64_ENABLE=ON \
       -DOPT_NEON_ENABLE=OFF \
       ..

make -j4



