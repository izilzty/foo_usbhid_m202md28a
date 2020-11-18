# foo_usbhid_m202md28a.dll

## 目录
* [使用说明](#使用说明)
     * [插件安装](#1-插件安装)
     * [首次使用](#2-首次使用)
     * [自定义显示内容](#3-自定义显示内容)
     * [自定义电源控制](#4-自定义电源控制)
     * [自定义滚动和音量显示延时](#5-自定义滚动和音量显示延时)
     * [HID设备VID&nbsp;PID设置](#6-HID设备VIDPID设置)
     * [恢复默认设置](#7-恢复默认设置)
     * [其他问题](#8-其他问题)
 * [编译说明](#编译说明)
      * [发布版本编译](#1-发布版本编译)
      * [调试](#2-调试)
***
## 使用说明
### 1. 插件安装
请在右侧Releases内下载foo_usbhid_m202md28a.dll文件，然后将此文件放入foobar2000安装目录的components文件夹内然即可。

如需自定义设置可以在 设置页面 -> Display -> HID VFD Display 内进行修改。
### 2. 首次使用
因为此型号屏幕的自定义字体需要先写入内置的闪存后才可以使用，为了不浪费闪存的写入寿命，在屏幕首次使用时需要手动写入一次自定义字体，之后只要不更换屏幕，都不需要再次此进行此操作了。如果要进行此操作，请执行以下步骤：
* 按Ctrl+P打开foobar2000的设置页面
* 转到 Display -> HID VFD Display
* 点击页面右下角FROM setting内的***WRITE***按钮
* 待屏幕显示“FROM WRITE SUCCESS”后，重新启动foobar2000

如果需要清除自定义字体，则可以按照相同的步骤，使用FROM setting内的***RESET***按钮擦除自定义字体并将屏幕恢复到出厂设置。
### 3. 自定义显示内容
可自定义显示内容的状态有四种，分别为播放时（On play）、加载时（On load）、停止时（On stop）、音量更改时（On volume change），其中播放时（On play）的显示内容支持foobar2000内置的格式化功能，可以自定义显示标题等信息，音量更改时（On volume change）的字符串为音量数字之前的文字，其余两项为普通字符串，留空则不更改上次显示内容。

关于foobar2000格式化功能的语法，详见 设置页面 -> Display -> Default User Interface 下方的Syntax help或[此链接](http://htmlpreview.github.io/?https://github.com/izilzty/foo_usbhid_m202md28a/blob/main/%E8%AE%BE%E8%AE%A1%E8%B5%84%E6%BA%90/%E6%A0%BC%E5%BC%8F%E5%8C%96%E5%B8%AE%E5%8A%A9%E6%96%87%E4%BB%B6/titleformat_help.html)。
### 4. 自定义电源控制
可自定义电源的状态有四种，分别为：播放时（On play）、暂停时（On pause）、停止时（On stop）、退出时（On exit），每种状态的电源控制可通过下拉列表选择。
```
Dim 25    [ 亮度 25% ]
Dim 50    [ 亮度 50% ]
Dim 75    [ 亮度 75% ]
Dim 100   [ 亮度 100% ]
Dim OFF   [ 显示关闭，但电源不关闭，下次启动速度快 ]
Power OFF [ 电源关闭，下次启动速度慢，会有灯丝加热过程 ]
```
### 5. 自定义滚动和音量显示延时
可自定义的延时有四种，分别为：音量显示时间（Display hold）、第一行文字从右向左渐入的速度（Fadein speed）、第一行文字渐入完成后等待时间（Scroll wait）、第一行文字循环滚动速度（Scroll speed）。
```
  Display hold [ 音量显示时间，设置为0则在音量更改时不显示音量 ]
  Fadein speed [ 文字从右向左渐入的速度，设置为0则直接显示在左侧 ]
  Scroll wait  [ 文字渐入完成后到开始滚动之间的延时，设置为0则直接开始滚动 ]
  Scroll speed [ 文字循环滚动速度，设置为0则在使用默认的300ms滚动速度完成一次滚动后停止 ]
```
### 6. HID设备VID&nbsp;PID设置
保持默认即可，无特殊情况无需更改，如果存在多个相同VID PID的屏幕则会使用找到的第一个屏幕，无法手动指定。

### 7. 恢复默认设置
点击页面下方的Reset page按钮，再点击Apply按钮，即可恢复设置到默认值，屏幕上的闪存不会被复位，所以无需重新写入FROM。
### 8. 其他问题
由于屏幕设计的问题，在USB接收数据时可能会停止扫描，这就会造成某些像素在非常短的时间内闪烁一下，亮度越低越明显，但在100%亮度下基本看不出来。所以解决方法有两种：一是降低文字滚动的速度，二是提高屏幕亮度。

因为屏幕内的双字节字符代码页是分开的，所以为了显示中日韩混合字符，可能会在一次显示数据内多次切换代码页，这就造成了可能在某些无关的字符位置会闪烁过一些乱码（只是偶尔会出现，貌似和外界干扰也有关系，闪烁一下会自动消失，也不会影响现有的字符，原因不明。
猜测可能和上面所说的停止扫描一样是USB设计的问题？因为这个屏幕设计是类似收银机上使用的，所以不会有频繁的切换代码页也请求。），不过速度非常快，不仔细看看不太出来，也无法100%复现。

由于屏幕设计的问题，在特定的亮度下升压电路的噪音可能比较明显，可以通过更改亮度来缓解。
***
## 编译说明
如果Visual Studio版本为2019，除了基础的“使用C++的桌面开发”外，还需要安装VS 2017 (v141)支持，或手动更改工程目标版本，但是会有报错，需要解决。
```
安装VS 2017 (v141)支持步骤如下：

运行Visual Studio安装程序，在单个组件里搜索141，然后分别选中

C++ ATL v141 生成工具 (x86 & x64)
C++ MFC v141 生成工具 (x86 & x64)
MSVC v141 - VS 2017 C++ x64/x86 生成工具(v14.16)
对 VS 2017 (v141)工具的 C++ Windows XP 支持
在右侧应该还会自动选中一个 Windows 通用 CRT SDK

最后点击修改即可
```
### 1. 发布版本编译
* 使用git clone或直接下载压缩包，将文件下载到本地
* 使用Visual Studio打开foo_usbhid_m202md28a.sln
* 将解决方案配置切换为Releases
* 生成 -> 生成解决方案（Ctrl+Shift+B）即可，生成的dll文件在foo_usbhid_m202md28a.sln目录下的Releases文件夹内
### 2. 调试
* 使用git clone或直接下载压缩包，将文件下载到本地
* 使用Visual Studio打开foo_usbhid_m202md28a.sln
* 将解决方案配置切换为Debug
* 调试 -> 开始调试（F5）即可。开始调试后，会自动启动工程文件夹内的foobar2000用于调试，不影响电脑上安装的现有版本
