**1 创建目录![](Aspose.Words.e69b8471-f1c8-4218-bd23-34e74846423f.001.png)**

在home目录下创建

ffmpeg\_sources：用于下载源文件

ffmpeg\_build： 存储编译后的库文件 bin：存储二进制文件（ffmpeg，ffplay，ffprobe，X264，X265等） cd ~ 

mkdir ffmpeg\_sources  ffmpeg\_build bin

**2 安装依赖![](Aspose.Words.e69b8471-f1c8-4218-bd23-34e74846423f.002.png)**

更新

sudo apt-get update 安装需要的组件

sudo apt-get -y install \   autoconf \ 

`  `automake \ 

`  `build-essential \

`  `cmake \ 

`  `git-core \ 

`  `libass-dev \ 

`  `libfreetype6-dev \

`  `libsdl2-dev \ 

`  `libtool \ 

`  `libva-dev \ 

`  `libvdpau-dev \ 

`  `libvorbis-dev \ 

`  `libxcb1-dev \ 

`  `libxcb-shm0-dev \ 

`  `libxcb-xfixes0-dev \   pkg-config \ 

`  `texinfo \ 

`  `wget \ 

`  `zlib1g-dev

**3 编译与安装![](Aspose.Words.e69b8471-f1c8-4218-bd23-34e74846423f.003.png)**

本指南假定您要安装一些最常见的第三方库。每个小节为您提供安装该库所需的命令。 如果不需要某些功能，则可以跳过相关小节（如果不需要），然后在FFmpeg中删除相应的./configure 选项。例如，如果不需要libvpx，请跳过该小节，然后从“ 安装FFmpeg”[部分中删除--enable-libvpx](https://trac.ffmpeg.org/wiki/CompilationGuide/Ubuntu#FFmpeg)。 **提示：**如果要在多核系统中加快编译速度，可以在每个make命令（例如make -j4）中使用-j选项。 **建议都使用源码进行安装。**

**NASM![](Aspose.Words.e69b8471-f1c8-4218-bd23-34e74846423f.004.png)**

部分库使用到汇编程序。

**使用源码进行安装**

cd ~/ffmpeg\_sources && \

wget [https://www.nasm.us/pub/nasm/releasebuilds/2.14.02/nasm-2.14.02.tar.bz2 && \ ](https://www.nasm.us/pub/nasm/releasebuilds/2.14.02/nasm-2.14.02.tar.bz2)

tar xjvf nasm-2.14.02.tar.bz2 && \

cd nasm-2.14.02 && \

./autogen.sh && \

PATH="$HOME/bin:$PATH" ./configure --prefix="$HOME/ffmpeg\_build" --bindir="$HOME/bin" && \ 

make && \ 

make install

**Yasm![](Aspose.Words.e69b8471-f1c8-4218-bd23-34e74846423f.005.png)**

部分库使用到该汇编库

使用源码进行安装: 

cd ~/ffmpeg\_sources && \

wget -O yasm-1.3.0.tar.gz [https://www.tortall.net/projects/yasm/releases/yasm-1.3.0.tar.gz && \ ](https://www.tortall.net/projects/yasm/releases/yasm-1.3.0.tar.gz)tar xzvf yasm-1.3.0.tar.gz && \

cd yasm-1.3.0 && \ 

./configure --prefix="$HOME/ffmpeg\_build" --bindir="$HOME/bin" && \

make && \ 

make install

**libx264![](Aspose.Words.e69b8471-f1c8-4218-bd23-34e74846423f.006.png)**

H.264视频编码器。更多信息和使用范例参考H.264 [Encoding Guide ](https://trac.ffmpeg.org/wiki/Encode/H.264)要求编译ffmpeg时配置：--enable-gpl--enable-libx264.

使用源码进行编译：

cd ~/ffmpeg\_sources && \

git -C x264 pull 2> /dev/null || git clone --depth 1 [https://gitee.com/mirrors_addons/x264.git && \ ](https://gitee.com/mirrors_addons/x264.git)cd x264 && \ 

PATH="$HOME/bin:$PATH" PKG\_CONFIG\_PATH="$HOME/ffmpeg\_build/lib/pkgconfig" ./configure --prefix="$HOME/ffmpeg\_build" --bindir="$HOME/bin" --enable-static --enable-pic && \ PATH="$HOME/bin:$PATH" make && \ 

make install

**libx265![](Aspose.Words.e69b8471-f1c8-4218-bd23-34e74846423f.007.png)**

H.265/HEVC 视频编码器， 更多信息和使用范例参考H.265[ Encoding Guide。 ](https://trac.ffmpeg.org/wiki/Encode/H.265)要求编译ffmpeg时配置：--enable-gpl--enable-libx265.

使用源码进行编译：

sudo apt-get install mercurial libnuma-dev && \

cd ~/ffmpeg\_sources && \

[if cd x265 2> /dev/null; then git pull && cd ..; else git clone https://gitee.com/mirrors_videolan/x26 5.git; fi && \ ](https://gitee.com/mirrors_videolan/x265.git)

cd x265/build/linux && \

PATH="$HOME/bin:$PATH" cmake -G "Unix Makefiles" - DCMAKE\_INSTALL\_PREFIX="$HOME/ffmpeg\_build" -DENABLE\_SHARED=off ../../source && \ PATH="$HOME/bin:$PATH" make && \ 

make install

**libvpx![](Aspose.Words.e69b8471-f1c8-4218-bd23-34e74846423f.004.png)**

VP8/VP9视频编解码器。 更多信息和使用范例参考VP9 Video[ Encoding Guide 。 ](https://trac.ffmpeg.org/wiki/Encode/VP9)要求编译ffmpeg时配置：--enable-libvpx. 

使用源码进行编译：

cd ~/ffmpeg\_sources && \

git -C libvpx pull 2> /dev/null || git clone --depth 1 [https://github.com/webmproject/libvpx.git && \ ](https://github.com/webmproject/libvpx.git)cd libvpx && \ 

PATH="$HOME/bin:$PATH" ./configure --prefix="$HOME/ffmpeg\_build" --disable-examples -- disable-unit-tests --enable-vp9-highbitdepth --as=yasm --enable-pic && \ PATH="$HOME/bin:$PATH" make && \ 

make install

**libfdk-aac![](Aspose.Words.e69b8471-f1c8-4218-bd23-34e74846423f.005.png)**

AAC音频编码器. 更多信息和使用范例参考AAC [Audio Encoding Guide。 ](https://trac.ffmpeg.org/wiki/Encode/AAC)要求编译ffmpeg时配置：--enable-libfdk-aac( 如果你已经配置了 --enable-gpl则需要加上--enable- nonfree). 

使用源码进行编译：

cd ~/ffmpeg\_sources && \

git -C fdk-aac pull 2> /dev/null || git clone --depth 1 [https://github.com/mstorsjo/fdk-aac && \ ](https://github.com/mstorsjo/fdk-aac)cd fdk-aac && \ 

autoreconf -fiv && \ 

./configure CFLAGS=-fPIC --prefix="$HOME/ffmpeg\_build"   && \

make && \ 

make install

**libmp3lame![](Aspose.Words.e69b8471-f1c8-4218-bd23-34e74846423f.006.png)**

MP3音频编码器. 要求编译ffmpeg时配置：--enable-libmp3lame.

使用源码进行编译：

cd ~/ffmpeg\_sources && \

git clone  --depth 1 [https://gitee.com/hqiu/lame.git && \ ](https://gitee.com/hqiu/lame.git)

cd lame && \ 

PATH="$HOME/bin:$PATH" ./configure --prefix="$HOME/ffmpeg\_build" --bindir="$HOME/bin"  -- enable-nasm --with-pic && \

PATH="$HOME/bin:$PATH" make && \ 

make install

**libopus![](Aspose.Words.e69b8471-f1c8-4218-bd23-34e74846423f.001.png)**

Opus音频编解码器. 

要求编译ffmpeg时配置：--enable-libopus.

使用源码进行编译：

cd ~/ffmpeg\_sources && \

git -C opus pull 2> /dev/null || git clone --depth 1 [https://github.com/xiph/opus.git && \ ](https://github.com/xiph/opus.git)cd opus && \ 

./autogen.sh && \

./configure --prefix="$HOME/ffmpeg\_build"  -with-pic&& \

make && \ 

make install

**FFmpeg![](Aspose.Words.e69b8471-f1c8-4218-bd23-34e74846423f.008.png)**

注意注意：如果要安装debug版本，请参考第6章节 《6 支持FFmpeg 代码 Debug》配置ffmpeg。

cd ~/ffmpeg\_sources && \

wget -O ffmpeg-4.2.1.tar.bz2 [https://ffmpeg.org/releases/ffmpeg-4.2.1.tar.bz2 && \ ](https://ffmpeg.org/releases/ffmpeg-4.2.1.tar.bz2)

tar xjvf ffmpeg-4.2.1.tar.bz2 && \

cd ffmpeg-4.2.1 && \ 

PATH="$HOME/bin:$PATH" PKG\_CONFIG\_PATH="$HOME/ffmpeg\_build/lib/pkgconfig" CFLAGS="- O3 -fPIC" ./configure \ 

`  `--prefix="$HOME/ffmpeg\_build" \

`  `--pkg-config-flags="--static" \

`  `--extra-cflags="-I$HOME/ffmpeg\_build/include" \

`  `--extra-ldflags="-L$HOME/ffmpeg\_build/lib" \

`  `--extra-libs="-lpthread -lm" \

`  `--bindir="$HOME/bin" \ 

`  `--enable-gpl \ 

`  `--enable-libass \ 

`  `--enable-libfdk-aac \

`  `--enable-libfreetype \

`  `--enable-libmp3lame \

`  `--enable-libopus \

`  `--enable-libvorbis \

`  `--enable-libvpx \ 

`  `--enable-libx264 \ 

`  `--enable-libx265 \ 

`  `--enable-pic \ 

`  `--enable-shared   \

`  `--enable-nonfree && \

PATH="$HOME/bin:$PATH" make && \ 

make install && \ 

hash -r 

然后重新登录系统或者在当前shell会话执行如下命令以识别新安装ffmpeg的位置： 

source ~/.profile

现在已经完成编译和安装ffmpeg(alsoffplay,ffprobe,lame,x264, &x265) 。该文档剩余章节主要讲如 何更新和删除ffmepg。 

编译完成后，ffmpeg\_build ffmpeg\_sourcesbin目录的大体情况

lqf@ubuntu:~/ffmpeg\_build$ ls

bin  include  lib  share

lqf@ubuntu:~/ffmpeg\_sources$ ls

fdk-aac               lame-3.100.tar.gz     opus        yasm-1.3.0.tar.gz ffmpeg-4.2.1          libvpx                x264

ffmpeg-4.2.1.tar.bz2  nasm-2.14.02          x265

lame                  nasm-2.14.02.tar.bz2  yasm-1.3.0

lqf@ubuntu:~/bin$ ls

ffmpeg  ffplay  ffprobe  lame  nasm  ndisasm  vsyasm  x264  yasm  ytasm

**4 使用![](Aspose.Words.e69b8471-f1c8-4218-bd23-34e74846423f.009.png)**

现在，您可以打开一个终端，输入ffmpeg命令，它应该执行新的ffmpeg。 如果你需要多个用户能同时使用你新编译的ffmpeg，则可以移动或者拷贝ffmpeg二进制文件从~/bin 到/usr/local/bin。 

测试ffplay是否可以使用（需要在图形方式进行测试） ffplayrtmp://media3.scctv.net/live/scctv\_800

如果能够正常播放则说明 ffplay能够编译成功使用。播放的时候要等等画面。

**5 文档![](Aspose.Words.e69b8471-f1c8-4218-bd23-34e74846423f.010.png)**

你可以使用 man ffmpeg以本地的方式访问文档: 

echo"MANPATH\_MAP $HOME/bin $HOME/ffmpeg\_build/share/man" >> ~/.manpath 你可能必须注销系统然后重新登录man ffmpeg才生效。 

HTML 格式的文档位于~/ffmpeg\_build/share/doc/ffmpeg.

你也可以参考在线文档 [online FFmpeg documentation, ](https://ffmpeg.org/documentation.html)

**6 支持FFmpeg 代码 Debug![](Aspose.Words.e69b8471-f1c8-4218-bd23-34e74846423f.011.png)**

刚才的工程可以运行，但不能debug。解决此问题，首先认定一点，生成的可执行程序中，ffmpeg 不包 含调试信息，调试信息在 ffmpeg\_g 中,debug 要选择 ffmpeg\_g。 另外，./config选项也是确定包含调试信息的核心，需要在config中添加：

- -–enable-debug=3：开启debug调试
  - -–disable-asm：禁用 asm 优化
    - --disable-optimizations：禁用优化，以便调试时按函数顺序执行。
      - –-disable-stripping：禁用剥离可执行程序和共享库，即调试时可以进入到某个函数进行单独调 试。

采用以下命令重新config: 先clean

make clean ![](Aspose.Words.e69b8471-f1c8-4218-bd23-34e74846423f.012.png)再重新config

./configure \![](Aspose.Words.e69b8471-f1c8-4218-bd23-34e74846423f.013.png)

`  `--prefix="$HOME/ffmpeg\_build" \

`  `--pkg-config-flags="--static" \

`  `--extra-cflags="-I$HOME/ffmpeg\_build/include" \   --extra-ldflags="-L$HOME/ffmpeg\_build/lib" \

`  `--extra-libs="-lpthread -lm" \

`  `--bindir="$HOME/bin" \

`  `--enable-gpl \![](Aspose.Words.e69b8471-f1c8-4218-bd23-34e74846423f.014.png)

`  `--enable-libass \

`  `--enable-libfdk-aac \

`  `--enable-libfreetype \

`  `--enable-libmp3lame \

`  `--enable-libopus \

`  `--enable-libvorbis \

`  `--enable-libvpx \

`  `--enable-libx264 \

`  `--enable-libx265 \

`  `--enable-pic \

`  `--enable-shared  \

`  `--enable-nonfree \

`  `--enable-debug=3 \

`  `--disable-optimizations \

`  `--disable-asm \

`  `--disable-stripping && \ PATH="$HOME/bin:$PATH" make && \ make install && \

hash -r

一些注意事项：

1\. 在使用 ffplay 播放生成 h264 格式的视频时，播放速度会加快，解决方式：不要使用 FFmpeg 转码 生成纯 h264 格式的视频，要使用一种容器包含 h264 视频，即生成一种音视频流格式，也就是不 要生成纯粹的 h264 码流，而是生成诸如 mkv 等格式的文件。
