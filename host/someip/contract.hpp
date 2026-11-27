#ifndef ZARC_SOMEIP_CONTRACT_HPP
#define ZARC_SOMEIP_CONTRACT_HPP
#include <cstdint>
namespace zarc::someip {
constexpr std::uint16_t service=0x1234;
constexpr std::uint16_t instance=1;
constexpr std::uint16_t snapshot_method=1;
constexpr std::uint16_t health_method=3;
constexpr std::uint8_t major=1;
constexpr std::uint32_t minor=0;
// Lab identifiers only, not allocated OEM identifiers. Snapshot input u32 BE.
// Response status then, on completion, value, quality, boot:u64, generation:u64.
// Health input u32 BE; result status, ID:u32, failures:u32, ready:u8,
// ownership:u8, sample tick:u64, source sequence:u64, boot:u64, session:u64.
}
#endif
