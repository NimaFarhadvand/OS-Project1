#include "MonolithicKernel.hpp"

namespace os_sim {
namespace monolithic {

MonolithicKernel::MonolithicKernel(VirtualDisk& disk, IVirtualConsoleDevice& console, Tracer& tracer)
    : disk_(disk), console_(console), tracer_(tracer),
      file_engine_(disk.get_total_blocks(),
                   [&disk](size_t id, std::array<uint8_t, DISK_BLOCK_SIZE>& b) { return disk.read_block(id, b); },
                   [&disk](size_t id, const std::array<uint8_t, DISK_BLOCK_SIZE>& b) { return disk.write_block(id, b); }) {
    setup_syscall_table();
}

int MonolithicKernel::syscall(int syscall_num, const SyscallArgs& args) {
    tracer_.record_domain_switch("UserSpace", "MonolithicKernel", "syscall_entry");
    tracer_.record_call_depth(3);

    int result = ErrorCode::InvalidArgument;
    const auto it = syscall_table_.find(syscall_num);
    if (it != syscall_table_.end()) {
        result = it->second(*this, args);
    }

    // Every syscall, including an invalid syscall number, returns to user space.
    tracer_.record_domain_switch("MonolithicKernel", "UserSpace", "syscall_return");
    return result;
}

void MonolithicKernel::setup_syscall_table() {
    syscall_table_[SYS_OPEN] = &MonolithicKernel::handle_open;
    syscall_table_[SYS_READ] = &MonolithicKernel::handle_read;
    syscall_table_[SYS_WRITE] = &MonolithicKernel::handle_write;
    syscall_table_[SYS_CLOSE] = &MonolithicKernel::handle_close;
    syscall_table_[SYS_CONSOLE_WRITE] = &MonolithicKernel::handle_console_write;
}

int MonolithicKernel::handle_open(MonolithicKernel& kernel, const SyscallArgs& args) {
    return kernel.sys_open(args.path, args.mode);
}

int MonolithicKernel::handle_read(MonolithicKernel& kernel, const SyscallArgs& args) {
    return kernel.sys_read(args.handle, args.buffer, args.count);
}

int MonolithicKernel::handle_write(MonolithicKernel& kernel, const SyscallArgs& args) {
    return kernel.sys_write(args.handle, args.write_buffer, args.count);
}

int MonolithicKernel::handle_close(MonolithicKernel& kernel, const SyscallArgs& args) {
    return kernel.sys_close(args.handle);
}

int MonolithicKernel::handle_console_write(MonolithicKernel& kernel, const SyscallArgs& args) {
    return kernel.sys_console_write(args.text);
}

int MonolithicKernel::sys_open(const std::string& path, OpenMode mode) {
    if (path.empty()) {
        return ErrorCode::InvalidArgument;
    }

    if (mode != OpenMode::Read && mode != OpenMode::Write) {
        return ErrorCode::InvalidArgument;
    }

    if (mode == OpenMode::Read) {
        if (!file_engine_.file_exists(path)) {
            return ErrorCode::NotFound;
        }
    } else {
        if (file_engine_.file_exists(path)) {
            file_engine_.truncate_file(path);
        } else if (!file_engine_.create_file(path)) {
            return ErrorCode::NoSpace;
        }
    }

    const int handle = allocate_fd();
    KernelFileDesc& entry = fd_table_[static_cast<size_t>(handle)];
    entry.path = path;
    entry.mode = mode;
    entry.offset = 0;
    entry.is_open = true;
    return handle;
}

int MonolithicKernel::sys_read(int handle, uint8_t* out_buf, size_t max_bytes) {
    if (!is_valid_fd(handle)) {
        return ErrorCode::InvalidHandle;
    }

    KernelFileDesc& entry = fd_table_[static_cast<size_t>(handle)];
    if (entry.mode != OpenMode::Read) {
        return ErrorCode::PermissionDenied;
    }

    // A zero-byte operation is valid and must not inspect or advance the buffer/offset.
    if (max_bytes == 0) {
        return 0;
    }
    if (out_buf == nullptr) {
        return ErrorCode::InvalidArgument;
    }

    const int bytes_read = file_engine_.read_bytes(entry.path, entry.offset, out_buf, max_bytes);
    if (bytes_read > 0) {
        entry.offset += static_cast<size_t>(bytes_read);
        tracer_.record_data_copy(static_cast<size_t>(bytes_read), "KernelBuffer", "UserBuffer");
    }
    return bytes_read;
}

int MonolithicKernel::sys_write(int handle, const uint8_t* in_data, size_t count) {
    if (!is_valid_fd(handle)) {
        return ErrorCode::InvalidHandle;
    }

    KernelFileDesc& entry = fd_table_[static_cast<size_t>(handle)];
    if (entry.mode != OpenMode::Write) {
        return ErrorCode::PermissionDenied;
    }

    // A zero-byte operation is valid and must leave the file offset unchanged.
    if (count == 0) {
        return 0;
    }
    if (in_data == nullptr) {
        return ErrorCode::InvalidArgument;
    }

    const int bytes_written = file_engine_.write_bytes(entry.path, entry.offset, in_data, count);
    if (bytes_written > 0) {
        entry.offset += static_cast<size_t>(bytes_written);
        tracer_.record_data_copy(static_cast<size_t>(bytes_written), "UserBuffer", "KernelBuffer");
    }
    return bytes_written;
}

int MonolithicKernel::sys_close(int handle) {
    if (!is_valid_fd(handle)) {
        return ErrorCode::InvalidHandle;
    }

    fd_table_[static_cast<size_t>(handle)].is_open = false;
    return ErrorCode::Success;
}

int MonolithicKernel::sys_console_write(const std::string& text) {
    console_.write(text);
    return ErrorCode::Success;
}

int MonolithicKernel::allocate_fd() {
    for (size_t i = 0; i < fd_table_.size(); ++i) {
        if (!fd_table_[i].is_open) {
            return static_cast<int>(i);
        }
    }
    fd_table_.push_back(KernelFileDesc{});
    return static_cast<int>(fd_table_.size() - 1);
}

bool MonolithicKernel::is_valid_fd(int handle) const {
    if (handle < 0 || static_cast<size_t>(handle) >= fd_table_.size()) {
        return false;
    }
    return fd_table_[handle].is_open;
}

} // namespace monolithic
} // namespace os_sim
