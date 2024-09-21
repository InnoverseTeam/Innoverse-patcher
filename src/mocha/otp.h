#pragma once

#include <assert.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct WiiUConsoleOTP WiiUConsoleOTP;
typedef struct OTPWiiBank OTPWiiBank;
typedef struct OTPWiiUBank OTPWiiUBank;
typedef struct OTPWiiUNGBank OTPWiiUNGBank;
typedef struct OTPWiiUCertBank OTPWiiUCertBank;
typedef struct OTPWiiCertBank OTPWiiCertBank;
typedef struct OTPMiscBank OTPMiscBank;

typedef uint32_t OTPJTAGStatus;
typedef uint32_t OTPSecurityLevel;

typedef enum OTPSecurityLevelFlags {
    SECURITY_FLAG_UNKNOWN              = 0x40000000, // Unknown, causes error in boot0
    SECURITY_FLAG_CONSOLE_PROGRAMMED   = 0x80000000, // Console type has been programmed
    SECURITY_FLAG_USE_DEBUG_KEY_IMAGE  = 0x08000000, // Use first RSA key and debug ancast images in boot0
    SECURITY_FLAG_USE_RETAIL_KEY_IMAGE = 0x10000000  // Use second RSA key and retail ancast images in boot0
} OTPSecurityLevelFlags;

typedef enum OTPIOStrength {
    IO_HW_IOSTRCTRL0   = 0x00008000,
    IO_HW_IOSTRCTRL1_3 = 0x00002000,
    IO_HW_IOSTRCTRL1_2 = 0x00000800,
    IO_HW_IOSTRCTRL1_1 = 0x00000080,
    IO_HW_IOSTRCTRL1_0 = 0x00000008,
    IO_NONE            = 0x00000000
} OTPIOStrength;

typedef enum OTPPulseLength {
    PULSE_BOOT0 = 0x0000002F,
    PULSE_NONE  = 0x00000000
} OTPPulseLength;

typedef enum OTPJTAGMask {
    JTAG_MASK_DISABLED = 0x80
} OTPJTAGMask;

struct OTPWiiBank {
    uint8_t boot1SHA1Hash[0x14];
    uint8_t commonKey[0x10];
    uint32_t ngId;
    uint8_t ngPrivateKey[0x1C];
    uint8_t nandHMAC[0x14];
    uint8_t nandKey[0x10];
    uint8_t rngKey[0x10];
};

struct OTPWiiUBank {
    OTPSecurityLevel securityLevel;
    OTPIOStrength ioStrength;
    OTPPulseLength pulseLength;
    uint32_t signature;
    uint8_t starbuckAncastKey[0x10];
    uint8_t seepromKey[0x10];
    uint8_t vWiiCommonKey[0x10];
    uint8_t wiiUCommonKey[0x10];
    uint8_t sslRSAKey[0x10];
    uint8_t usbStorageSeedsKey[0x10];
    uint8_t xorKey[0x10];
    uint8_t rngKey[0x10];
    uint8_t slcKey[0x10];
    uint8_t mlcKey[0x10];
    uint8_t sshdKey[0x10];
    uint8_t drhWLAN[0x10];
    uint8_t slcHmac[0x14];
};

struct OTPWiiUNGBank {
    uint32_t ngId;
    uint8_t ngPrivateKey[0x20];
    uint8_t privateNSSDeviceCertKey[0x20];
    uint8_t otpRNGSeed[0x10];
};

struct OTPWiiUCertBank {
    uint32_t rootCertMSId;
    uint32_t rootCertCAId;
    uint32_t rootCertNGKeyId;
    uint8_t rootCertNGSignature[0x3C];
};

struct OTPWiiCertBank {
    uint32_t rootCertMSId;
    uint32_t rootCertCAId;
    uint32_t rootCertNGKeyId;
    uint8_t rootCertNGSignature[0x3C];
    uint8_t koreanKey[0x10];
    uint8_t privateNSSDeviceCertKey[0x20];
};

struct OTPMiscBank {
    uint8_t boot1Key_protected[0x10];
    uint32_t otpVersionAndRevision;
    uint64_t otpDateCode;
    char otpVersionName[0x08];
    OTPJTAGStatus jtagStatus;
};

struct WiiUConsoleOTP {
    OTPWiiBank wiiBank;
    OTPWiiUBank wiiUBank;
    OTPWiiUNGBank wiiUNGBank;
    OTPWiiUCertBank wiiUCertBank;
    OTPWiiCertBank wiiCertBank;
    OTPMiscBank miscBank;
};

#ifdef __cplusplus
} // extern "C"
#endif