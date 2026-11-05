


#include "Layer1_Drivers.hpp"

namespace os_sim {
namespace layered {

Layer1_Drivers::Layer1_Drivers(Layer0_Hardware& hw, Tracer& tracer)
    : hw_(hw), tracer_(tracer) {}

bool Layer1_Drivers::driver_read_sector(
    size_t sector_id,
    std::array<uint8_t, DISK_BLOCK_SIZE>& buf) {

    tracer_.record_layer_transition(1, 0, "READ_SECTOR");

    const bool success = hw_.raw_read_block(sector_id, buf);

    if (success) {
        tracer_.record_data_copy(
            DISK_BLOCK_SIZE,
            "Layer0_DiskBuffer",
            "Layer1_DriverBuffer"
        );
    }

    return success;
}

bool Layer1_Drivers::driver_write_sector(
    size_t sector_id,
    const std::array<uint8_t, DISK_BLOCK_SIZE>& buf) {

    tracer_.record_layer_transition(1, 0, "WRITE_SECTOR");

    const bool success = hw_.raw_write_block(sector_id, buf);

    if (success) {
        tracer_.record_data_copy(
            DISK_BLOCK_SIZE,
            "Layer1_DriverBuffer",
            "Layer0_DiskBuffer"
        );
    }

    return success;
}

void Layer1_Drivers::driver_write_display(
    const std::string& text) {

    tracer_.record_layer_transition(1, 0, "WRITE_DISPLAY");

    hw_.raw_write_console(text);
}

size_t Layer1_Drivers::get_capacity_sectors() const {
    return hw_.get_total_blocks();
}

} // namespace layered
} // namespace os_sim
