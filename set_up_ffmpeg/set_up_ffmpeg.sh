#!/bin/bash
FFMPEG_BIN=ffmpeg_bin

# 1. 創建目錄
mkdir ~/ffmpeg_sources

mkdir ~/$FFMPEG_BIN

mkdir ~/ffmpeg_build


# 2. 更新套件
sudo apt-get update


# 3. 安裝所需套件
sudo apt-get -y install \
 autoconf \
 automake \
 build-essential \
 cmake \
 git-core \
 libass-dev \
 libfreetype6-dev \
 libsdl2-dev \
 libtool \
 libva-dev \
 libvdpau-dev \
 libvorbis-dev \
 libxcb1-dev \
 libxcb-shm0-dev \
 libxcb-xfixes0-dev \
 pkg-config \
 texinfo \
 wget \
 zlib1g-dev \
 libnuma-dev
 

# NASM
cd ~/ffmpeg_sources && \
 wget https://www.nasm.us/pub/nasm/releasebuilds/2.14.02/nasm-2.14.02.tar.bz2 && \
 tar xjvf nasm-2.14.02.tar.bz2 && \
 cd nasm-2.14.02 && \
 ./autogen.sh && \
 PATH="$HOME/$FFMPEG_BIN:$PATH" ./configure --prefix="$HOME/ffmpeg_build" --bindir="$HOME/$FFMPEG_BIN" &&
 \
 make && \
 make install

#Yasm
cd ~/ffmpeg_sources && \
 wget -O yasm-1.3.0.tar.gz https://www.tortall.net/projects/yasm/releases/yasm-1.3.0.tar.gz && \
 tar xzvf yasm-1.3.0.tar.gz && \
 cd yasm-1.3.0 && \
 ./configure --prefix="$HOME/ffmpeg_build" --bindir="$HOME/$FFMPEG_BIN" && \
 make && \
 make install

#Libx264
cd ~/ffmpeg_sources && \
 git -C x264 pull 2> /dev/null || git clone --depth 1 https://github.com/mirror/x264.git && \
 cd x264 && \
 PATH="$HOME/$FFMPEG_BIN:$PATH" PKG_CONFIG_PATH="$HOME/ffmpeg_build/lib/pkgconfig" ./configure --prefix="$HOME/ffmpeg_build" --bindir="$HOME/$FFMPEG_BIN" --enable-static --enable-pic && \
 PATH="$HOME/$FFMPEG_BIN:$PATH" make && \
 make install
 
#libx265
cd ~/ffmpeg_sources && git clone https://gitee.com/mirrors_videolan/x265.git && \
  cd x265/build/linux && \
  PATH="$HOME/$FFMPEG_BIN:$PATH" cmake -G "Unix Makefiles" -DCMAKE_INSTALL_PREFIX="$HOME/ffmpeg_build" -DENABLE_SHARED=off ../../source && \
  PATH="$HOME/$FFMPEG_BIN:$PATH" make && \
  make install
  
#libvpx
cd ~/ffmpeg_sources && \
  git -C libvpx pull 2> /dev/null || git clone --depth 1 https://github.com/webmproject/libvpx.git && \
  cd libvpx && \
  PATH="$HOME/$FFMPEG_BIN:$PATH" ./configure --prefix="$HOME/ffmpeg_build" --disable-examples --disable-unit-tests --enable-vp9-highbitdepth --as=yasm --enable-pic && \
  PATH="$HOME/$FFMPEG_BIN:$PATH" make && \
  make install
  
#libfdk-aac
cd ~/ffmpeg_sources && \
 git -C fdk-aac pull 2> /dev/null || git clone --depth 1 https://github.com/mstorsjo/fdk-aac && \
 cd fdk-aac && \
 autoreconf -fiv && \
 ./configure CFLAGS=-fPIC --prefix="$HOME/ffmpeg_build" && \
 make && \
 make install
 
#libmp3lame
cd ~/ffmpeg_sources && \
  git clone --depth 1 https://gitee.com/hqiu/lame.git && \
  cd lame && \
  PATH="$HOME/$FFMPEG_BIN:$PATH" ./configure --prefix="$HOME/ffmpeg_build" --bindir="$HOME/$FFMPEG_BIN" --enable-nasm --with-pic && \
  PATH="$HOME/$FFMPEG_BIN:$PATH" make && \
  make install
  
#libopus
cd ~/ffmpeg_sources && \
  git -C opus pull 2> /dev/null || git clone --depth 1 https://github.com/xiph/opus.git && \
  cd opus && \
  ./autogen.sh && \
  ./configure --prefix="$HOME/ffmpeg_build" -with-pic&& \
  make && \
  make install
  
#ffmpeg
echo "Building ffmpeg, FFMPEG_BIN=$FFMPEG_BIN"
cd ~/ffmpeg_sources && \
 wget -O ffmpeg-4.2.1.tar.bz2 https://ffmpeg.org/releases/ffmpeg-4.2.1.tar.bz2 && \
 tar xjvf ffmpeg-4.2.1.tar.bz2 && \
 cd ffmpeg-4.2.1 && \
 PATH="$HOME/$FFMPEG_BIN:$PATH" PKG_CONFIG_PATH="$HOME/ffmpeg_build/lib/pkgconfig" CFLAGS="-O3 -fPIC" ./configure \
 --prefix="$HOME/ffmpeg_build" \
 --pkg-config-flags="--static" \
 --extra-cflags="-I$HOME/ffmpeg_build/include" \
 --extra-ldflags="-L$HOME/ffmpeg_build/lib" \
 --extra-libs="-lpthread -lm" \
 --bindir="$HOME/$FFMPEG_BIN" \
 --enable-gpl \
 --enable-libass \
 --enable-libfdk-aac \
 --enable-libfreetype \
 --enable-libmp3lame \
 --enable-libvorbis \
 --enable-libvpx \
 --enable-libx264 \
 --enable-libx265 \
 --enable-pic \
 --enable-shared \
 --enable-nonfree && \
 PATH="$HOME/$FFMPEG_BIN:$PATH" make && \
 make install && \
 hash -r
 
sudo sh -c "echo '/home/wangefan/ffmpeg_build/lib/' >> /etc/ld.so.conf"
sudo ldconfig

echo "腳本執行完成"
