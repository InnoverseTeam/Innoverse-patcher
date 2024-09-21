#include "common.h"
#include "program.h"
#include "iosu_url_patches.h"
#include "mocha/mocha.h"

int iosuhaxHandle        = -1;
int mochaInitDone        = 0;
uint32_t mochaApiVersion = 0;

#define ALIGN(align)                 __attribute__((aligned(align)))
#define ALIGN_0x40                   ALIGN(0x40)
#define ROUNDUP(x, align)            (((x) + ((align) -1)) & ~((align) -1))

#define __FSAShimSetupRequestMount   ((FSError(*)(FSAShimBuffer *, uint32_t, const char *, const char *, uint32_t, void *, uint32_t))(0x101C400 + 0x042f88))
#define __FSAShimSetupRequestUnmount ((FSError(*)(FSAShimBuffer *, uint32_t, const char *, uint32_t))(0x101C400 + 0x43130))
#define __FSAShimSend                ((FSError(*)(FSAShimBuffer *, uint32_t))(0x101C400 + 0x042d90))

#define IOCTL_MEM_WRITE      0x00
#define IOCTL_MEM_READ       0x01
#define IOCTL_SVC            0x02
#define IOCTL_MEMCPY         0x04
#define IOCTL_REPEATED_WRITE 0x05
#define IOCTL_KERN_READ32    0x06
#define IOCTL_KERN_WRITE32   0x07
#define IOCTL_READ_OTP       0x08

MochaUtilsStatus Mocha_IOSUKernelWrite32(uint32_t address, uint32_t value) {
    if (address == 0) {
        return MOCHA_RESULT_INVALID_ARGUMENT;
    }
    if (!mochaInitDone || iosuhaxHandle < 0) {
        return MOCHA_RESULT_LIB_UNINITIALIZED;
    }

    ALIGN_0x40 uint32_t io_buf[0x40 >> 2];
    io_buf[0] = address;
    io_buf[1] = value;

    auto res = IOS_Ioctl(iosuhaxHandle, IOCTL_KERN_WRITE32, io_buf, 2 * sizeof(uint32_t), 0, 0);
    return res >= 0 ? MOCHA_RESULT_SUCCESS : MOCHA_RESULT_UNKNOWN_ERROR;
}

MochaUtilsStatus Mocha_IOSUKernelRead32(uint32_t address, uint32_t *out_buffer) {
    if (address == 0 || out_buffer == nullptr) {
        return MOCHA_RESULT_INVALID_ARGUMENT;
    }
    if (!mochaInitDone || iosuhaxHandle < 0) {
        return MOCHA_RESULT_LIB_UNINITIALIZED;
    }

    ALIGN_0x40 uint32_t io_buf[0x40 >> 2];
    io_buf[0] = address;

    void *tmp_buf = NULL;
    int32_t count = 1;

    if (((uintptr_t) out_buffer & 0x3F) || ((count * 4) & 0x3F)) {
        tmp_buf = (uint32_t *) memalign(0x40, ROUNDUP((count * 4), 0x40));
        if (!tmp_buf) {
            return MOCHA_RESULT_OUT_OF_MEMORY;
        }
    }

    int res = IOS_Ioctl(iosuhaxHandle, IOCTL_KERN_READ32, io_buf, sizeof(address), tmp_buf ? tmp_buf : out_buffer, count * 4);

    if (res >= 0 && tmp_buf) {
        memcpy(out_buffer, tmp_buf, count * 4);
    }

    free(tmp_buf);
    return res >= 0 ? MOCHA_RESULT_SUCCESS : MOCHA_RESULT_UNKNOWN_ERROR;
}

static void write_string(uint32_t addr, const char *str) {
    int len = strlen(str) + 1;
    int remaining = len % 4;
    int num = len - remaining;

    for (int i = 0; i < (num / 4); i++) {
        Mocha_IOSUKernelWrite32(addr + i * 4, *(uint32_t * )(str + i * 4));
    }

    if (remaining > 0) {
        uint8_t buf[4];
        Mocha_IOSUKernelRead32(addr + num, (uint32_t * ) & buf);

        for (int i = 0; i < remaining; i++) {
            buf[i] = *(str + num + i);
        }

        Mocha_IOSUKernelWrite32(addr + num, *(uint32_t * ) & buf);
    }
}

int MainThread(int argc, void* argv) {
    uint64_t title_id = OSGetTitleID();
    if (title_id != 0x000500101004A100 &&  // USA
        title_id != 0x000500101004A000 &&  // EUR
        title_id != 0x0005001010040000)    // JPN
    {
        log_printf("error for starting program.");
        return 0;    
    } 

    for (const auto &patch: url_patches) {
        write_string(patch.address, patch.url);
    }

    return 0;
}