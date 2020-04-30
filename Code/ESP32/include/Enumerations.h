#pragma once
namespace ModbusAdapter
{

enum FunctionCode {
    FC_READ_COILS       = 0x01, // Read Coils (Output) Status
    FC_READ_INPUT_STAT  = 0x02, // Read Input Status (Discrete Inputs)
    FC_READ_REGS        = 0x03, // Read Holding Registers
    FC_READ_INPUT_REGS  = 0x04, // Read Input Registers
    FC_WRITE_COIL       = 0x05, // Write Single Coil (Output)
    FC_WRITE_REG        = 0x06, // Preset Single Register
    FC_DIAGNOSTICS      = 0x08, // Not implemented. Diagnostics (Serial Line only)
    FC_WRITE_COILS      = 0x0F, // Write Multiple Coils (Outputs)
    FC_WRITE_REGS       = 0x10, // Write block of contiguous registers
    FC_READ_FILE_REC    = 0x14, // Not implemented. Read File Record
    FC_WRITE_FILE_REC   = 0x15, // Not implemented. Write File Record
    FC_MASKWRITE_REG    = 0x16, // Not implemented. Mask Write Register
    FC_READWRITE_REGS   = 0x17  // Not implemented. Read/Write Multiple registers
};

enum ResultCode {
    EX_SUCCESS              = 0x00, // Custom. No error
    EX_ILLEGAL_FUNCTION     = 0x01, // Function Code not Supported
    EX_ILLEGAL_ADDRESS      = 0x02, // Output Address not exists
    EX_ILLEGAL_VALUE        = 0x03, // Output Value not in Range
    EX_SLAVE_FAILURE        = 0x04, // Slave or Master Device Fails to process request
    EX_ACKNOWLEDGE          = 0x05, // Not used
    EX_SLAVE_DEVICE_BUSY    = 0x06, // Not used
    EX_MEMORY_PARITY_ERROR  = 0x08, // Not used
    EX_PATH_UNAVAILABLE     = 0x0A, // Not used
    EX_DEVICE_FAILED_TO_RESPOND = 0x0B, // Not used
    EX_GENERAL_FAILURE      = 0xE1, // Custom. Unexpected master error
    EX_DATA_MISMACH         = 0xE2, // Custom. Inpud data size mismach
    EX_UNEXPECTED_RESPONSE  = 0xE3, // Custom. Returned result doesn't mach transaction
    EX_TIMEOUT              = 0xE4, // Custom. Operation not finished within reasonable time
    EX_CONNECTION_LOST      = 0xE5, // Custom. Connection with device lost
    EX_CANCEL               = 0xE6  // Custom. Transaction/request canceled
};

enum ReplyCode {
    REPLY_OFF            = 0x01,
    REPLY_ECHO           = 0x02,
    REPLY_NORMAL         = 0x03,
    REPLY_ERROR          = 0x04,
    REPLY_UNEXPECTED     = 0x05
};

}