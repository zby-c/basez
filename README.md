# basez - 一个 RFC4648 (BaseX) 命令行编解码工具
## 数据编码
支持 Base16, Base32(可自定义字母表), Base64(可自定义字母表) 数据编码.  

[RFC4648标准](https://www.rfc-editor.org/rfc/rfc4648)

## 命令行参数
Tip:  使用自修改的 [cmdline](https://github.com/tanakh/cmdline) 库实现命令行参数解析.

- --encoding, -e    \[base16, base32, base64\]  (必需, 使用的编码)
- --type, -t    \[0, 1\]  (0为编码, 1为解码, 默认为0)
- --alphabet, -E    \[自定义字母表\]  (Base32, Base64可指定字母表, 默认使用标准字母表)
- --file, -f  (若使用, 则最后一个命令行参数视为文件名, 否则为数据)
- \[最后一个命令行参数\]  (数据内容/文件名)

## 行为
根据指定的参数, 对文件内容或指定数据进行编码/解码, 结果输出到标准输出流(stdout)中, 
其他错误信息输出到标准错误流中, 用户在shell中可用`>`等将结果重定向到文件(不支持进程间通信管道`|`).

WARNING: 解码结果的显示和编码结果均受到shell代码页(Code Page)的影响, 中文用户编解码中文前请注意.

## 脚本
位于`script`文件夹中有适用于Windows的`batch`脚本, 用于启动工作文件夹位于basez.exe位置的`cmd.exe`, 便于部分用户使用.  

1. `launch_cmd.bat`使用默认的编码页(简中下为GBK), 中文使用GBK编码, 解码时输出为GBK(文件编码GBK/ANSI时使用)
2. `launch_cmd_utf8.bat` 使用UTF-8编码页, 中文使用UTF-8编码, 解码时输出UTF-8(文件编码为UTF-8时使用

## 开源协议
- 主程序的源码/程序基于 [MIT License](https://opensource.org/license/MIT) 分发
- `cmdline` 库参见`cmdline.h`或其github仓库
