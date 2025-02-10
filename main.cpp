/**
 * @brief basez - 一个 RFC4648 (BaseX) 命令行编解码工具
 * @author Zou Boyu
 * @copyright 2025 Zou Boyu (under MIT License)
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated
 * documentation files (the “Software”), to deal in the Software without restriction, including without limitation the
 * rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to
 * permit persons to whom the Software is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all copies or substantial portions of the
 * Software.
 *
 * THE SOFTWARE IS PROVIDED “AS IS”, WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
 * WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR
 * COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR
 * OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 *
 * @note 对C++一知半解, 随手写的玩具罢了
 */

#include <chrono>
#include <fstream>
#include <print>
#include <stdexcept>
#include <string>
#include <thread>
#ifdef WIN32 // 防呆设计
#include <Windows.h>
#include <tlhelp32.h>

DWORD GetParentProcessId(const DWORD dwProcessId) {
    DWORD dwParentProcessId = 0;
    HANDLE hSnapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
    if (hSnapshot != INVALID_HANDLE_VALUE) {
        PROCESSENTRY32 pe32;
        pe32.dwSize = sizeof(PROCESSENTRY32);
        if (Process32First(hSnapshot, &pe32)) {
            do {
                if (pe32.th32ProcessID == dwProcessId) {
                    dwParentProcessId = pe32.th32ParentProcessID;
                    break;
                }
            } while (Process32Next(hSnapshot, &pe32));
        }
        CloseHandle(hSnapshot);
    }
    return dwParentProcessId;
}

// 获取进程名称
std::string GetProcessName(const DWORD dwProcessId) {
    std::string processName;
    HANDLE hSnapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
    if (hSnapshot != INVALID_HANDLE_VALUE) {
        PROCESSENTRY32 pe32;
        pe32.dwSize = sizeof(PROCESSENTRY32);
        if (Process32First(hSnapshot, &pe32)) {
            do {
                if (pe32.th32ProcessID == dwProcessId) {
                    processName = pe32.szExeFile;
                    break;
                }
            } while (Process32Next(hSnapshot, &pe32));
        }
        CloseHandle(hSnapshot);
    }
    return processName;
}

#endif

#define CMDLINE_USAGE_HEADER                                                                                           \
    "basez - BaseX 命令行编解码工具 - v1.1.1\n"                                                                        \
    "对 RFC4648 的简单实现 (https://www.rfc-editor.org/rfc/rfc4648)\n"                                                 \
    "gh源码地址: zby-c/basez\n"                                                                                        \
    "Copyright (c) 2025 Zou Boyu [sharkzby@outlook.com]\n"                                                             \
    "基于 MIT License 分发 (https://opensource.org/license/MIT)\n"                                                     \
    "使用 cmdline 库处理命令行参数\n"                                                                                  \
    "------------------------------------------------------------\n"

#include "cmdline.h"

/**
 * @class RFC4648
 * @brief 一个静态函数类, RFC4648 的部分简单实现 (Base 16, Base 32, Base 64)
 * @note 详见 https://www.rfc-editor.org/rfc/rfc4648
 */
class RFC4648 {
public:
    /**
     * @param input 数据字符串
     * @return Base 16 编码结果
     */
    static std::string Base16Encode(const std::string &input) {
        std::string output;
        for (const unsigned char c: input) {
            constexpr auto chars = "0123456789ABCDEF";
            output.push_back(chars[c >> 4]);
            output.push_back(chars[c & 0x0F]);
        }
        return output;
    }

    /**
     * @param input Base 16 编码字符串
     * @return 解码结果
     */
    static std::string Base16Decode(const std::string &input) {
        if (input.length() % 2 != 0) {
            throw std::invalid_argument("ERROR: 无效数据");
        }
        std::string output;
        for (size_t i = 0; i < input.length(); i += 2) {
            const unsigned char high = std::stoi(input.substr(i, 1), nullptr, 16);
            const unsigned char low = std::stoi(input.substr(i + 1, 1), nullptr, 16);
            output.push_back((high << 4) | low);
        }
        return output;
    }

    /**
     * @param input 数据字符串
     * @param chars 自定义字母表
     * @return Base 32 编码结果
     */
    static std::string Base32Encode(const std::string &input, const char *chars = "ABCDEFGHIJKLMNOPQRSTUVWXYZ234567") {
        std::string output;
        int buffer = 0, bitsLeft = 0;
        for (const unsigned char c: input) {
            buffer <<= 8;
            buffer |= c & 0xFF;
            bitsLeft += 8;
            while (bitsLeft >= 5) {
                output.push_back(chars[(buffer >> (bitsLeft - 5)) & 0x1F]);
                bitsLeft -= 5;
            }
        }
        if (bitsLeft > 0) {
            buffer <<= (5 - bitsLeft);
            output.push_back(chars[buffer & 0x1F]);
        }
        return output;
    }

    /**
     * @param input Base 32 编码字符串
     * @param chars 自定义字母表
     * @return 解码结果
     */
    static std::string Base32Decode(const std::string &input, const char *chars = "ABCDEFGHIJKLMNOPQRSTUVWXYZ234567") {
        std::string output;
        int buffer = 0, bitsLeft = 0;
        for (const char c: input) {
            const int val = std::strchr(chars, c) - chars;
            if (val < 0) {
                throw std::invalid_argument("ERROR: 无效数据");
            }
            buffer <<= 5;
            buffer |= val & 0x1F;
            bitsLeft += 5;
            if (bitsLeft >= 8) {
                output.push_back((buffer >> (bitsLeft - 8)) & 0xFF);
                bitsLeft -= 8;
            }
        }
        return output;
    }

    /**
     * @param input 数据字符串
     * @param chars 自定义字母表
     * @return Base 64 编码结果
     */
    static std::string
    Base64Encode(const std::string &input,
                 const char *chars = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/") {
        std::string output;
        int buffer = 0, bitsLeft = 0;
        for (const unsigned char c: input) {
            buffer <<= 8;
            buffer |= c & 0xFF;
            bitsLeft += 8;
            while (bitsLeft >= 6) {
                output.push_back(chars[(buffer >> (bitsLeft - 6)) & 0x3F]);
                bitsLeft -= 6;
            }
        }
        if (bitsLeft > 0) {
            buffer <<= (6 - bitsLeft);
            output.push_back(chars[buffer & 0x3F]);
        }
        while (output.length() % 4 != 0) {
            output.push_back('=');
        }
        return output;
    }

    /**
     * @param input Base 64 编码字符串
     * @param chars 自定义字母表
     * @return 解码结果
     */
    static std::string
    Base64Decode(const std::string &input,
                 const char *chars = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/") {
        std::string output;
        int buffer = 0, bitsLeft = 0;
        for (const char c: input) {
            if (c == '=')
                break;
            const int val = std::strchr(chars, c) - chars;
            if (val < 0) {
                throw std::invalid_argument("ERROR: 无效数据");
            }
            buffer <<= 6;
            buffer |= val & 0x3F;
            bitsLeft += 6;
            if (bitsLeft >= 8) {
                output.push_back((buffer >> (bitsLeft - 8)) & 0xFF);
                bitsLeft -= 8;
            }
        }
        return output;
    }
};

[[noreturn]] int main(const int argc, char *argv[]) {
#ifdef WIN32 // 防呆设计
    if (argc == 1) {
        DWORD dwCurrentProcessId = GetCurrentProcessId();
        DWORD dwParentProcessId = GetParentProcessId(dwCurrentProcessId);
        // 狠狠吐槽 不会真的有人直接双击打开吧
        if (std::string parentProcessName = GetProcessName(dwParentProcessId); parentProcessName == "explorer.exe") {
            std::println(std::cerr, "ERROR: 不要资源管理器里直接打开, 请使用启动脚本或在shell中启动");
            std::this_thread::sleep_for(std::chrono::seconds(5));
            return -1;
        }
    }
#endif


    cmdline::parser parser;

    parser.set_program_name("basez");
    parser.footer("数据/文件名");
    parser.add<std::string>("encoding", 'e', "使用的编码", true, "",
                            cmdline::oneof(std::string("base16"), std::string("base32"), std::string("base64")));
    parser.add<int>("type", 't', "编码(0)/解码(1)", false, 0, cmdline::range(0, 1));
    parser.add<std::string>("alphabet", 'E', "自定义字母表(当编码为base32, base64时可用)", false, "");
    parser.add("file", 'f', "文件标志(则最后一个参数将视为文件名, 否则为数据)");

    parser.parse_check(argc, argv);

    auto encoding = parser.get<std::string>("encoding");
    auto type = parser.get<int>("type");
    auto alphabet = parser.get<std::string>("alphabet");
    std::string data = argv[argc - 1];

    if (parser.exist("file")) { // 获取文件内容为 data
        std::ifstream file(data.c_str());
        std::istreambuf_iterator<char> beg(file), end;
        data = std::string(beg, end);
    }

    if (encoding == "base16" && !alphabet.empty()) {
        std::println(std::cerr, "ERROR: 自定义字母表仅编码为base32, base64时可用)");
        return -1;
    } else if (!alphabet.empty() &&
               (encoding == "base32" && alphabet.length() != 32 || encoding == "base64" && alphabet.length() != 64)) {
        std::println(std::cerr, "ERROR: 自定义字母表长度错误");
        return -2;
    }

    try {
        if (encoding == "base16") {
            if (type == 0) {
                // 编码
                std::print("{}", RFC4648::Base16Encode(data));
            } else {
                // 解码
                std::print("{}", RFC4648::Base16Decode(data));
            }
        } else if (encoding == "base32") {
            if (type == 0) {
                if (!alphabet.empty()) { // 自定义编码表
                    std::print("{}", RFC4648::Base32Encode(data, alphabet.c_str()));
                } else {
                    std::print("{}", RFC4648::Base32Encode(data));
                }
            } else {
                if (!alphabet.empty()) {
                    std::print("{}", RFC4648::Base32Decode(data, alphabet.c_str()));
                } else {
                    std::print("{}", RFC4648::Base32Decode(data));
                }
            }
        } else if (encoding == "base64") {
            if (type == 0) {
                if (!alphabet.empty()) {
                    std::print("{}", RFC4648::Base64Encode(data, alphabet.c_str()));
                } else {
                    std::print("{}", RFC4648::Base64Encode(data));
                }
            } else {
                if (!alphabet.empty()) {
                    std::print("{}", RFC4648::Base64Decode(data, alphabet.c_str()));
                } else {
                    std::print("{}", RFC4648::Base64Decode(data));
                }
            }
        }


    } catch (const std::invalid_argument &e) {
        std::println(std::cerr, "{}", e.what());
    }
    return 0;
}
