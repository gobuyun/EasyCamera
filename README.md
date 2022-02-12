# EasyCamera
ffmpeg+Camera=EasyCamera
使用ffmpeg采集摄像头

运行结果：
![image](https://user-images.githubusercontent.com/29557326/153695397-5db8c192-9e74-4c34-81be-4d895fbe7351.png)


如何跑起来：
- 0、链接ffmpeg相关头文件、SDL头文件。
以及链接以下库文件：
avformat.lib
avcodec.lib
avutil.lib
swscale.lib
avdevice.lib
avfilter.lib
swresample.lib
SDL2.lib
SDL2main.lib
SDL2test.lib
- 1、打开sln
- 2、修改main.cpp的摄像头名称为自己的摄像头名字
- 3、编译
- 4、运行提示缺少dll，把ffmpegSDK/bin的所有dll拷贝到exe运行目录下，再跑一次即可

功能：
- 固定每秒30帧采集摄像头视频，麦克风采集没写，自动读取摄像头名字功能没写

我遇到的坑：
- 我的ffmpeg为mingw64编译的，mingw32编出来的ffmpeg我发现调用avformat_open_input接口打开dshow时会崩溃。官网所说是dshow仅在mingw64支持。
原话如下，这是网址http://trac.ffmpeg.org/wiki/CompilationGuide/MinGW：
DirectShow
DirectShow SDK is required for DirectShow capture, supported through the dshow input device. DirectShow support is enabled only through mingw-64 compilation.
For more detailed information related to DirectShow check the dshow section in the FFmpeg manual.
