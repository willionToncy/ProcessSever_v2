#pragma once

#include <cstdint>
#include <vector>
#include <string>
#include <cstring>
#include <memory>
#include <stdexcept>
#include <type_traits>

/**
 * Message类 - 表示一条逻辑消息
 * 
 * 设计原则：
 * - 业务层永远只接触Message对象
 * - 支持POD类型和string的序列化
 * - 自动管理payload容量
 * - 使用shared_ptr进行生命周期管理
 */
class Message
{
public:
    uint32_t command;
    std::vector<uint8_t> payload;

    /**
     * 构造函数
     * @param cmd 命令码
     */
    explicit Message(uint32_t cmd) : command(cmd) {}

    /**
     * 写入POD类型数据
     * @param v 要写入的数据
     */
    template<typename T>
    void write(const T& v) {
        static_assert(std::is_trivially_copyable_v<T>, "T must be trivially copyable");
        
        size_t old_size = payload.size();
        payload.resize(old_size + sizeof(T));
        std::memcpy(payload.data() + old_size, &v, sizeof(T));
    }

    /**
     * 写入字符串数据
     * @param str 要写入的字符串
     */
    void write(const std::string& str) {
        uint32_t len = static_cast<uint32_t>(str.length());
        write(len);  // 先写入长度
        if (len > 0) {
            size_t old_size = payload.size();
            payload.resize(old_size + len);
            std::memcpy(payload.data() + old_size, str.data(), len);
        }
    }

    /**
     * 读取POD类型数据
     * @param offset 读取偏移量
     * @return 读取的数据
     */
    template<typename T>
    T read(size_t offset) const {
        static_assert(std::is_trivially_copyable_v<T>, "T must be trivially copyable");
        
        if (offset + sizeof(T) > payload.size()) {
            throw std::out_of_range("Read offset out of range");
        }
        
        T result;
        std::memcpy(&result, payload.data() + offset, sizeof(T));
        return result;
    }

    /**
     * 读取字符串数据
     * @param offset 读取偏移量
     * @return 读取的字符串
     */
    std::string readString(size_t offset) const {
        if (offset + sizeof(uint32_t) > payload.size()) {
            throw std::out_of_range("String length offset out of range");
        }
        
        uint32_t len = read<uint32_t>(offset);
        offset += sizeof(uint32_t);
        
        if (offset + len > payload.size()) {
            throw std::out_of_range("String data out of range");
        }
        
        return std::string(reinterpret_cast<const char*>(payload.data() + offset), len);
    }

    /**
     * 获取payload大小
     */
    size_t getPayloadSize() const {
        return payload.size();
    }

    /**
     * 清空payload
     */
    void clearPayload() {
        payload.clear();
    }
};

// 类型别名，方便使用
using MessagePtr = std::shared_ptr<Message>;