#pragma once
#include "commands.h"
#include "otp.h"
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef enum MochaUtilsStatus {
    MOCHA_RESULT_SUCCESS                 = 0,
    MOCHA_RESULT_INVALID_ARGUMENT        = -0x01,
    MOCHA_RESULT_MAX_CLIENT              = -0x02,
    MOCHA_RESULT_OUT_OF_MEMORY           = -0x03,
    MOCHA_RESULT_ALREADY_EXISTS          = -0x04,
    MOCHA_RESULT_ADD_DEVOPTAB_FAILED     = -0x05,
    MOCHA_RESULT_NOT_FOUND               = -0x06,
    MOCHA_RESULT_UNSUPPORTED_API_VERSION = -0x10,
    MOCHA_RESULT_UNSUPPORTED_COMMAND     = -0x11,
    MOCHA_RESULT_UNSUPPORTED_CFW         = -0x12,
    MOCHA_RESULT_LIB_UNINITIALIZED       = -0x20,
    MOCHA_RESULT_UNKNOWN_ERROR           = -0x100,
} MochaUtilsStatus;

const char *Mocha_GetStatusStr(MochaUtilsStatus status);

/**
 * Initializes the mocha lib. Needs to be called before any other functions can be used
 * @return MOCHA_RESULT_SUCCESS:                Library has been successfully initialized <br>
 *         MOCHA_RESULT_UNSUPPORTED_COMMAND:    Failed to initialize the library caused by an outdated mocha version.
 */
MochaUtilsStatus Mocha_InitLibrary();

/**
 * Deinitializes the mocha lib
 * @return
 */
MochaUtilsStatus Mocha_DeInitLibrary();

/**
 * Retrieves the API Version of the running mocha.
 *
 * @param outVersion pointer to the variable where the version will be stored.
 *
 * @return MOCHA_RESULT_SUCCESS:                    The API version has been store in the version ptr<br>
 *         MOCHA_RESULT_INVALID_ARGUMENT:           Invalid version pointer<br>
 *         MOCHA_RESULT_UNSUPPORTED_API_VERSION:    Failed to get the API version caused by an outdated mocha version.<br>
 *         MOCHA_RESULT_UNSUPPORTED_CFW:            Failed to get the API version caused by not using a (compatible) CFW.
 */
MochaUtilsStatus Mocha_CheckAPIVersion(uint32_t *outVersion);

/**
 * Copies data within IOSU with MCP permission.
 * @param dst - Destination address
 * @param src - Source address
 * @param size - Bytes to copy.
 * @return MOCHA_RESULT_SUCCESS:            The data has been copied successfully<br>
 *         MOCHA_RESULT_INVALID_ARGUMENT:   Invalid dst or src pointer<br>
 *         MOCHA_RESULT_LIB_UNINITIALIZED:  Library was not initialized. Call Mocha_InitLibrary() before using this function.<br>
 *         MOCHA_RESULT_UNKNOWN_ERROR:      Unknown error
 */
MochaUtilsStatus Mocha_IOSUMemoryMemcpy(uint32_t dst, uint32_t src, uint32_t size);

/**
 * Writes data to a given address with MCP permission.
 * @param address - Address where the data will be written to.
 * @param buffer - Pointer to the data which should be written.
 * @param size - Bytes to write.
 * @return MOCHA_RESULT_SUCCESS:            The data has been written successfully<br>
 *         MOCHA_RESULT_INVALID_ARGUMENT:   Invalid address or buffer pointer<br>
 *         MOCHA_RESULT_LIB_UNINITIALIZED:  Library was not initialized. Call Mocha_InitLibrary() before using this function.<br>
 *         MOCHA_RESULT_OUT_OF_MEMORY:      Not enough memory <br>
 *         MOCHA_RESULT_UNKNOWN_ERROR:      Unknown error
 */
MochaUtilsStatus Mocha_IOSUMemoryWrite(uint32_t address, const uint8_t *buffer, uint32_t size);

/**
 * Reads data from a given address with MCP permission.
 * @param address - Address where the data will be read from.
 * @param buffer - Pointer to the buffer where the read will be stored
 * @param size - Bytes to read.
 * @return MOCHA_RESULT_SUCCESS: The data has been read successfully<br>
 *         MOCHA_RESULT_INVALID_ARGUMENT:   Invalid source address<br>
 *         MOCHA_RESULT_LIB_UNINITIALIZED:  Library was not initialized. Call Mocha_InitLibrary() before using this function.<br>
 *         MOCHA_RESULT_UNKNOWN_ERROR:      Unknown error
 */
MochaUtilsStatus Mocha_IOSUMemoryRead(uint32_t address, uint8_t *out_buffer, uint32_t size);

/**
 * Writes 4 bytes with IOSU kernel permission
 * @param address Address where the value will be written.
 * @param value Value that will be written to address.
 * @return MOCHA_RESULT_SUCCESS: The data has been written successfully<br>
 *         MOCHA_RESULT_INVALID_ARGUMENT:   Invalid target address<br>
 *         MOCHA_RESULT_LIB_UNINITIALIZED:  Library was not initialized. Call Mocha_InitLibrary() before using this function.<br>
 *         MOCHA_RESULT_OUT_OF_MEMORY:      Not enough memory <br>
 *         MOCHA_RESULT_UNKNOWN_ERROR:      Unknown error
 */
MochaUtilsStatus Mocha_IOSUKernelWrite32(uint32_t address, uint32_t value);

/**
 * Reads 4 bytes with IOSU kernel permission
 * @param address Address from which the data will be read.
 * @param out_buffer Pointer where the result will be stored
 * @return MOCHA_RESULT_SUCCESS: The data has been read successfully<br>
 *         MOCHA_RESULT_INVALID_ARGUMENT:   Invalid target address<br>
 *         MOCHA_RESULT_LIB_UNINITIALIZED:  Library was not initialized. Call Mocha_InitLibrary() before using this function.<br>
 *         MOCHA_RESULT_UNKNOWN_ERROR:      Unknown error
 */
MochaUtilsStatus Mocha_IOSUKernelRead32(uint32_t address, uint32_t *out_buffer);

#ifdef __cplusplus
} // extern "C"
#endif