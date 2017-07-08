// Copyright 2017 Citra Emulator Project
// Licensed under GPLv2 or any later version
// Refer to the license.txt file included.

#ifdef _WIN32
#include <winsock2.h>
#else
#include <arpa/inet.h>
#endif
#include <cstring>
#include <string>
#include "network/packet.h"

namespace Network {

Packet::Packet() : read_pos(0), is_valid(true) {}

Packet::~Packet() {}

void Packet::Append(const void* in_data, std::size_t size_in_bytes) {
    if (in_data && (size_in_bytes > 0)) {
        std::size_t start = data.size();
        data.resize(start + size_in_bytes);
        std::memcpy(&data[start], in_data, size_in_bytes);
    }
}

void Packet::Read(void* out_data, std::size_t size_in_bytes) {
    if (out_data && CheckSize(size_in_bytes)) {
        std::memcpy(out_data, &data[read_pos], size_in_bytes);
        read_pos += size_in_bytes;
    }
}

void Packet::Clear() {
    data.clear();
    read_pos = 0;
    is_valid = true;
}

const void* Packet::GetData() const {
    return !data.empty() ? &data[0] : NULL;
}

void Packet::IgnoreBytes(u32 length) {
    read_pos += length;
}

std::size_t Packet::GetDataSize() const {
    return data.size();
}

bool Packet::EndOfPacket() const {
    return read_pos >= data.size();
}

Packet::operator BoolType() const {
    return is_valid ? &Packet::CheckSize : NULL;
}

Packet& Packet::operator>>(bool& out_data) {
    u8 value;
    if (*this >> value) {
        out_data = (value != 0);
    }
    return *this;
}

Packet& Packet::operator>>(s8& out_data) {
    Read(&out_data, sizeof(out_data));
    return *this;
}

Packet& Packet::operator>>(u8& out_data) {
    Read(&out_data, sizeof(out_data));
    return *this;
}

Packet& Packet::operator>>(s16& out_data) {
    s16 value;
    Read(&value, sizeof(value));
    out_data = ntohs(value);
    return *this;
}

Packet& Packet::operator>>(u16& out_data) {
    u16 value;
    Read(&value, sizeof(value));
    out_data = ntohs(value);
    return *this;
}

Packet& Packet::operator>>(s32& out_data) {
    s32 value;
    Read(&value, sizeof(value));
    out_data = ntohl(value);
    return *this;
}

Packet& Packet::operator>>(u32& out_data) {
    u32 value;
    Read(&value, sizeof(value));
    out_data = ntohl(value);
    return *this;
}

Packet& Packet::operator>>(float& out_data) {
    Read(&out_data, sizeof(out_data));
    return *this;
}

Packet& Packet::operator>>(double& out_data) {
    Read(&out_data, sizeof(out_data));
    return *this;
}

Packet& Packet::operator>>(char* out_data) {
    // First extract string length
    u32 length = 0;
    *this >> length;

    if ((length > 0) && CheckSize(length)) {
        // Then extract characters
        std::memcpy(out_data, &data[read_pos], length);
        out_data[length] = '\0';

        // Update reading position
        read_pos += length;
    }

    return *this;
}

Packet& Packet::operator>>(std::string& out_data) {
    // First extract string length
    u32 length = 0;
    *this >> length;

    out_data.clear();
    if ((length > 0) && CheckSize(length)) {
        // Then extract characters
        out_data.assign(&data[read_pos], length);

        // Update reading position
        read_pos += length;
    }

    return *this;
}

Packet& Packet::operator<<(bool in_data) {
    *this << static_cast<u8>(in_data);
    return *this;
}

Packet& Packet::operator<<(s8 in_data) {
    Append(&in_data, sizeof(in_data));
    return *this;
}

Packet& Packet::operator<<(u8 in_data) {
    Append(&in_data, sizeof(in_data));
    return *this;
}

Packet& Packet::operator<<(s16 in_data) {
    s16 toWrite = htons(in_data);
    Append(&toWrite, sizeof(toWrite));
    return *this;
}

Packet& Packet::operator<<(u16 in_data) {
    u16 toWrite = htons(in_data);
    Append(&toWrite, sizeof(toWrite));
    return *this;
}

Packet& Packet::operator<<(s32 in_data) {
    s32 toWrite = htonl(in_data);
    Append(&toWrite, sizeof(toWrite));
    return *this;
}

Packet& Packet::operator<<(u32 in_data) {
    u32 toWrite = htonl(in_data);
    Append(&toWrite, sizeof(toWrite));
    return *this;
}

Packet& Packet::operator<<(float in_data) {
    Append(&in_data, sizeof(in_data));
    return *this;
}

Packet& Packet::operator<<(double in_data) {
    Append(&in_data, sizeof(in_data));
    return *this;
}

Packet& Packet::operator<<(const char* in_data) {
    // First insert string length
    u32 length = std::strlen(in_data);
    *this << length;

    // Then insert characters
    Append(in_data, length * sizeof(char));

    return *this;
}

Packet& Packet::operator<<(const std::string& in_data) {
    // First insert string length
    u32 length = static_cast<u32>(in_data.size());
    *this << length;

    // Then insert characters
    if (length > 0)
        Append(in_data.c_str(), length * sizeof(std::string::value_type));

    return *this;
}

bool Packet::CheckSize(std::size_t size) {
    is_valid = is_valid && (read_pos + size <= data.size());

    return is_valid;
}

} // namespace Network
