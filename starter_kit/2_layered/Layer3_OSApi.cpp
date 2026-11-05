
#include "Layer3_OSApi.hpp"

namespace os_sim {
namespace layered {

Layer3_OSApi::Layer3_OSApi(Layer2_FileSystem& layer2, Tracer& tracer)
    : layer2_(layer2), tracer_(tracer) {}

int Layer3_OSApi::open(const std::string& path, OpenMode mode) {
    ParameterBlock pb{};
    pb.op = LayerOp::Open;
    pb.path = path;
    pb.mode = mode;

    tracer_.record_layer_transition(3, 2, "OPEN");
    layer2_.process_request(pb);

    return pb.status;
}

int Layer3_OSApi::read(
    int handle,
    std::vector<uint8_t>& out_buffer,
    size_t max_bytes) {

    ParameterBlock pb{};
    pb.op = LayerOp::Read;
    pb.handle = handle;
    pb.length = max_bytes;

    out_buffer.resize(max_bytes);
    pb.buffer = out_buffer.data();

    tracer_.record_layer_transition(3, 2, "READ");
    layer2_.process_request(pb);

    if (pb.status <= 0) {
        out_buffer.clear();
    } else {
        out_buffer.resize(static_cast<size_t>(pb.status));
    }

    return pb.status;
}

int Layer3_OSApi::write(
    int handle,
    const std::vector<uint8_t>& data) {

    ParameterBlock pb{};
    pb.op = LayerOp::Write;
    pb.handle = handle;
    pb.write_buffer = data.data();
    pb.length = data.size();

    tracer_.record_layer_transition(3, 2, "WRITE");
    layer2_.process_request(pb);

    return pb.status;
}

int Layer3_OSApi::close(int handle) {
    ParameterBlock pb{};
    pb.op = LayerOp::Close;
    pb.handle = handle;

    tracer_.record_layer_transition(3, 2, "CLOSE");
    layer2_.process_request(pb);

    return pb.status;
}

void Layer3_OSApi::console_write(const std::string& text) {
    ParameterBlock pb{};
    pb.op = LayerOp::ConsoleWrite;
    pb.text = text;

    tracer_.record_layer_transition(3, 2, "CONSOLE_WRITE");
    layer2_.process_request(pb);
}

std::string Layer3_OSApi::get_architecture_name() const {
    return "Strict Layered Approach";
}

} // namespace layered
} // namespace os_sim
