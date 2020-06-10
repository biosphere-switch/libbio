
#pragma once
#include <bio/gpu/gpu_Types.hpp>

namespace bio::gpu {

    enum class ConnectionApi : i32 {
        EGL = 1,
        Cpu = 2,
        Media = 3,
        Camera = 4,
    };

    enum class DisconnectMode : u32 {
        Api = 0,
        AllLocal = 1,
    };

    struct QueueBufferOutput {
        u32 width;
        u32 height;
        u32 transform_hint;
        u32 pending_buffer_count;
    };

    struct Plane {
        u32 width;
        u32 height;
        ColorFormat color_format;
        Layout layout;
        u32 pitch;
        u32 map_handle_unused;
        u32 offset;
        Kind kind;
        u32 block_height_log2;
        DisplayScanFormat display_scan_format;
        u32 second_field_offset;
        u64 flags;
        u64 size;
        u32 unk[6];
    };
    static_assert(sizeof(Plane) == 0x58);

    struct GraphicBufferHeader {
        u32 magic;
        u32 width;
        u32 height;
        u32 stride;
        PixelFormat pixel_format;
        GraphicsAllocatorUsage usage;
        u32 pid;
        u32 refcount;
        u32 fd_count;
        u32 buffer_size;

        static constexpr u32 Magic = 0x47424652; // GBFR (Graphic Buffer)

    };

    // TODO: is the packed attribute really needed here?

    struct __attribute__((packed)) GraphicBuffer {
        GraphicBufferHeader header;
        u32 null;
        u32 map_id;
        u32 zero;
        u32 buffer_magic;
        u32 pid;
        u32 type;
        GraphicsAllocatorUsage usage;
        PixelFormat pixel_format;
        PixelFormat external_pixel_format;
        u32 stride;
        u32 full_size;
        u32 plane_count;
        u32 zero_2;
        Plane planes[3];
        u64 unused;

        static constexpr u32 Magic = 0xDAFFCAFF;

    };
    static_assert(sizeof(GraphicBuffer) == 0x16C);

    struct Fence {
        u32 id;
        u32 value;
    };

    struct MultiFence {
        u32 fence_count;
        Fence fences[4];
    };

    struct Rect {
        i32 left;
        i32 top;
        i32 right;
        i32 bottom;
    };

    enum class Transform : u32 {
        FlipH = 1,
        FlipV = 2,
        Rotate90 = 4,
        Rotate180 = 3,
        Rotate270 = 7,
    };

    struct QueueBufferInput {
        i64 timestamp;
        i32 is_auto_timestamp;
        Rect crop;
        i32 scaling_mode;
        Transform transform;
        u32 sticky_transform;
        u32 unk;
        u32 swap_interval;
        MultiFence fence;
    };

}