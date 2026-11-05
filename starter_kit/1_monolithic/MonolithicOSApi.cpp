#include "MonolithicOSApi.hpp"

namespace os_sim {
namespace monolithic {

MonolithicOSApi::MonolithicOSApi(MonolithicKernel& kernel) : kernel_(kernel) {}

int MonolithicOSApi::open(const std::string& path, OpenMode mode) {
    SyscallArgs args;
    args.path = path;
    args.mode = mode;
    return kernel_.syscall(SYS_OPEN, args);
}

int MonolithicOSApi::read(int handle, std::vector<uint8_t>& out_buffer, size_t max_bytes) {
    out_buffer.resize(max_bytes);

    SyscallArgs args;
    args.handle = handle;
    args.buffer = out_buffer.data();
    args.count = max_bytes;

    const int result = kernel_.syscall(SYS_READ, args);
    if (result <= 0) {
        out_buffer.clear();
    } else {
        out_buffer.resize(static_cast<size_t>(result));
    }
    return result;
}

int MonolithicOSApi::write(int handle, const std::vector<uint8_t>& data) {
    SyscallArgs args;
    args.handle = handle;
    args.write_buffer = data.data();
    args.count = data.size();
    return kernel_.syscall(SYS_WRITE, args);
}

int MonolithicOSApi::close(int handle) {
    SyscallArgs args;
    args.handle = handle;
    return kernel_.syscall(SYS_CLOSE, args);
}

void MonolithicOSApi::console_write(const std::string& text) {
    SyscallArgs args;
    args.text = text;
    (void)kernel_.syscall(SYS_CONSOLE_WRITE, args);
}

std::string MonolithicOSApi::get_architecture_name() const {
    return "Linux-inspired Monolithic";
}

} // namespace monolithic
} // namespace os_sim
