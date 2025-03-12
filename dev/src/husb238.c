/**
 * @file husb238.c
 * @author Tomasz Watorowski (tomasz.watorowski@gmail.com)
 * @date 2025-03-01
 *
 * @copyright Copyright (c) 2025
 */

#include "err.h"
#include "dev/husb238.h"
#include "dev/swi2c_dev.h"

#define DEBUG DLVL_INFO
#include "debug.h"

/* i2c address */
#define I2C_ADDR                                    0x08

/** register addresses  */
#define REGADDR_PD_STATUS0                          0x00
#define REGADDR_PD_STATUS1                          0x01
#define REGADDR_SRC_PDO_5V                          0x02
#define REGADDR_SRC_PDO_9V                          0x03
#define REGADDR_SRC_PDO_12V                         0x04
#define REGADDR_SRC_PDO_15V                         0x05
#define REGADDR_SRC_PDO_18V                         0x06
#define REGADDR_SRC_PDO_20V                         0x07
#define REGADDR_SRC_PDO                             0x08
#define REGADDR_GO_COMMAND                          0x09

/** bits of pd_status0 register */
/* voltage values */
#define REG_PD_STATUS0_PD_SRC_VOLTAGE               0xf0
#define REG_PD_STATUS0_PD_SRC_VOLTAGE_UNATTACHED    0x00
#define REG_PD_STATUS0_PD_SRC_VOLTAGE_5V            0x10
#define REG_PD_STATUS0_PD_SRC_VOLTAGE_9V            0x20
#define REG_PD_STATUS0_PD_SRC_VOLTAGE_12V           0x30
#define REG_PD_STATUS0_PD_SRC_VOLTAGE_15V           0x40
#define REG_PD_STATUS0_PD_SRC_VOLTAGE_18V           0x50
#define REG_PD_STATUS0_PD_SRC_VOLTAGE_20V           0x60
/* current values */
#define REG_PD_STATUS0_PD_SRC_CURRENT               0x0f
#define REG_PD_STATUS0_PD_SRC_CURRENT_0A5           0x00
#define REG_PD_STATUS0_PD_SRC_CURRENT_0A7           0x01
#define REG_PD_STATUS0_PD_SRC_CURRENT_1A            0x02
#define REG_PD_STATUS0_PD_SRC_CURRENT_1A25          0x03
#define REG_PD_STATUS0_PD_SRC_CURRENT_1A5           0x04
#define REG_PD_STATUS0_PD_SRC_CURRENT_1A75          0x05
#define REG_PD_STATUS0_PD_SRC_CURRENT_2A            0x06
#define REG_PD_STATUS0_PD_SRC_CURRENT_2A25          0x07
#define REG_PD_STATUS0_PD_SRC_CURRENT_2A5           0x08
#define REG_PD_STATUS0_PD_SRC_CURRENT_2A75          0x09
#define REG_PD_STATUS0_PD_SRC_CURRENT_3A            0x0a
#define REG_PD_STATUS0_PD_SRC_CURRENT_3A25          0x0b
#define REG_PD_STATUS0_PD_SRC_CURRENT_3A5           0x0c
#define REG_PD_STATUS0_PD_SRC_CURRENT_4A            0x0d
#define REG_PD_STATUS0_PD_SRC_CURRENT_4A5           0x0e
#define REG_PD_STATUS0_PD_SRC_CURRENT_5A            0x0f


/** bits of pd_status1 register */
/* voltage values */
#define REG_PD_STATUS1_CC_DIR                       0x80
#define REG_PD_STATUS1_CC_DIR_C1                    0x00
#define REG_PD_STATUS1_CC_DIR_C2                    0x80
/* are we attached bit */
#define REG_PD_STATUS1_ATTACH                       0x40
/* power delivery protocol responses */
#define REG_PD_STATUS1_PD_RESPONSE                  0x38
#define REG_PD_STATUS1_PD_RESPONSE_NO_RESP          0x00
#define REG_PD_STATUS1_PD_RESPONSE_SUCCESS          0x08
#define REG_PD_STATUS1_PD_RESPONSE_INVALID_CMD      0x10
#define REG_PD_STATUS1_PD_RESPONSE_CMD_NOT_SUP      0x18
#define REG_PD_STATUS1_PD_RESPONSE_TRANS_FAIL       0x20
/* are we using legacy 5V? */
#define REG_PD_STATUS1_5V_VOLTAGE                   0x04
#define REG_PD_STATUS1_5V_VOLTAGE_OTHER             0x00
#define REG_PD_STATUS1_5V_VOLTAGE_5V                0x04
/* what is the current capability? */
#define REG_PD_STATUS1_5V_CURRENT                   0x03
#define REG_PD_STATUS1_5V_CURRENT_DEFAULT           0x00
#define REG_PD_STATUS1_5V_CURRENT_1A5               0x01
#define REG_PD_STATUS1_5V_CURRENT_2A4               0x02
#define REG_PD_STATUS1_5V_CURRENT_3A                0x03

/** register bit definition for  REG_SRC_PDO_xV registers */
#define REG_SRC_DETECT                              0x80
/* current values */
#define REG_SRC_CURRENT                             0x0f
#define REG_SRC_CURRENT_0A5                         0x00
#define REG_SRC_CURRENT_0A7                         0x01
#define REG_SRC_CURRENT_1A                          0x02
#define REG_SRC_CURRENT_1A25                        0x03
#define REG_SRC_CURRENT_1A5                         0x04
#define REG_SRC_CURRENT_1A75                        0x05
#define REG_SRC_CURRENT_2A                          0x06
#define REG_SRC_CURRENT_2A25                        0x07
#define REG_SRC_CURRENT_2A5                         0x08
#define REG_SRC_CURRENT_2A75                        0x09
#define REG_SRC_CURRENT_3A                          0x0a
#define REG_SRC_CURRENT_3A25                        0x0b
#define REG_SRC_CURRENT_3A5                         0x0c
#define REG_SRC_CURRENT_4A                          0x0d
#define REG_SRC_CURRENT_4A5                         0x0e
#define REG_SRC_CURRENT_5A0                         0x0f


/** bit definitions for SRC_PDO register  */
#define REG_SRC_PDO_PDO_SELECT                      0xf0
#define REG_SRC_PDO_PDO_SELECT_NOT_SEL              0x00
#define REG_SRC_PDO_PDO_SELECT_5V                   0x10
#define REG_SRC_PDO_PDO_SELECT_9V                   0x20
#define REG_SRC_PDO_PDO_SELECT_12V                  0x30
#define REG_SRC_PDO_PDO_SELECT_15V                  0x80
#define REG_SRC_PDO_PDO_SELECT_18V                  0x90
#define REG_SRC_PDO_PDO_SELECT_20V                  0xa0


/** bit definitions for GO_COMMAND register  */
#define REG_GO_COMMAND_COMMAND_FUNC                 0x1f
#define REG_GO_COMMAND_COMMAND_FUNC_REQ_PDO         0x01
#define REG_GO_COMMAND_COMMAND_FUNC_GET_SRC_CAP     0x04
#define REG_GO_COMMAND_COMMAND_FUNC_HARD_RESET      0x10


/* read the chips internal regiuster */
static err_t HUSB238_ReadReg(husb238_dev_t *dev, int reg_addr, int *val)
{
    /* transaction error code */
    err_t ec = EOK; uint8_t value_u8, reg_addr_u8 = reg_addr;

    /* perform the read operation  */
    with_sem (&dev->swi2c->sem) {
        /* setup the register address */
        if (ec >= EOK) ec |= SwI2C_Transfer(dev->swi2c, SWI2C_OPER_ADDR |
            SWI2C_OPER_WR | SWI2C_OPER_STOP, I2C_ADDR, &reg_addr_u8,
            sizeof(reg_addr_u8));
        /* read the register contents */
        if (ec >= EOK) ec |= SwI2C_Transfer(dev->swi2c, SWI2C_OPER_ADDR |
            SWI2C_OPER_RD | SWI2C_OPER_STOP, I2C_ADDR, &value_u8,
            sizeof(value_u8));
    }

    /* store the value */
    if (val) *val = value_u8;
    /* return the error code */
    return ec;
}


/* write the chips internal regiuster */
static err_t HUSB238_WriteReg(husb238_dev_t *dev, int reg_addr, int val)
{
    /* operation error code */
    err_t ec = EOK;
    /* payload that we are about to send */
    struct { uint8_t reg_addr, value; } PACKED pld = {
        .reg_addr = reg_addr, .value = val };

    /* perform the read operation  */
    with_sem (&dev->swi2c->sem)
        ec = SwI2C_Transfer(dev->swi2c, SWI2C_OPER_ADDR |
            SWI2C_OPER_WR | SWI2C_OPER_STOP, I2C_ADDR, &pld, sizeof(pld));

    /* return the error code */
    return ec;
}

/* initialize common parts of the driver */
err_t HUSB238_Init(void)
{
    /* report status */
    return EOK;
}

/* initialize particular device */
err_t HUSB238_DevInit(husb238_dev_t *dev)
{
    /* perform a hard reset on the chip */
    return HUSB238_HardReset(dev);
}

/* query the source about it's capabilities */
err_t HUSB238_QuerySource(husb238_dev_t *dev)
{
    /* write the command code into the register */
    return HUSB238_WriteReg(dev, REGADDR_GO_COMMAND,
        REG_GO_COMMAND_COMMAND_FUNC_GET_SRC_CAP);
}

/* perform a hard reset command */
err_t HUSB238_HardReset(husb238_dev_t *dev)
{
    /* write the command code into the register */
    return HUSB238_WriteReg(dev, REGADDR_GO_COMMAND,
        REG_GO_COMMAND_COMMAND_FUNC_HARD_RESET);
}

/* get the current contract */
err_t HUSB238_GetCurrentContract(husb238_dev_t *dev, husb328_volts_t *volts,
    husb328_amps_t *amps)
{
    /* operation error code */
    err_t ec = EOK;
    /* placeholder for the status register */
    int status0, status1;
    /* volts and amps */
    husb328_volts_t u = HUSB_VOLTS_UNKNOWN;
    husb328_amps_t i = HUSB_AMPS_UNKNOWN;

    /* read the status registers */
    if (ec >= EOK) ec = HUSB238_ReadReg(dev, REGADDR_PD_STATUS0, &status0);
    if (ec >= EOK) ec = HUSB238_ReadReg(dev, REGADDR_PD_STATUS1, &status1);
    /* error during communication */
    if (ec < EOK)
        return ec;

    /* according to the chip we are not attached to the usb port */
    if (!(status1 & REG_PD_STATUS1_ATTACH) ||
        (status1 & REG_PD_STATUS1_PD_RESPONSE) != REG_PD_STATUS1_PD_RESPONSE_SUCCESS)
        goto end;

    /* are we using legacy 5V */
    int using_5v = (status1 & REG_PD_STATUS1_5V_VOLTAGE) ==
        REG_PD_STATUS1_5V_VOLTAGE_5V;

    /* voltage determination */
    /* now we have the 5V contract */
    if (using_5v) {
        u = HUSB_VOLTS_5V;
    /* we are using power delivery negotiated contract */
    } else {
        /* power delivery voltages */
        switch (status0 & REG_PD_STATUS0_PD_SRC_VOLTAGE) {
        /* standart power delivery voltages */
        case REG_PD_STATUS0_PD_SRC_VOLTAGE_5V: u = HUSB_VOLTS_5V; break;
        case REG_PD_STATUS0_PD_SRC_VOLTAGE_9V: u = HUSB_VOLTS_9V; break;
        case REG_PD_STATUS0_PD_SRC_VOLTAGE_12V: u = HUSB_VOLTS_12V; break;
        case REG_PD_STATUS0_PD_SRC_VOLTAGE_15V: u = HUSB_VOLTS_15V; break;
        case REG_PD_STATUS0_PD_SRC_VOLTAGE_18V: u = HUSB_VOLTS_18V; break;
        case REG_PD_STATUS0_PD_SRC_VOLTAGE_20V: u = HUSB_VOLTS_20V; break;
        /* unknown voltage setting */
        default: u = HUSB_VOLTS_UNKNOWN; break;
        }
    }

    /* current determination in legacy mode */
    if (using_5v) {
        switch (status1 & REG_PD_STATUS1_5V_CURRENT) {
        /* unknown current reported */
        case REG_PD_STATUS1_5V_CURRENT_DEFAULT: i = HUSB_AMPS_UNKNOWN; break;
        /* standart currents */
        case REG_PD_STATUS1_5V_CURRENT_1A5: i = HUSB_AMPS_1A5; break;
        case REG_PD_STATUS1_5V_CURRENT_2A4: i = HUSB_AMPS_2A25; break;
        case REG_PD_STATUS1_5V_CURRENT_3A: i = HUSB_AMPS_3A; break;
        }
    /* power delivery */
    } else {
        switch (status0 & REG_PD_STATUS0_PD_SRC_CURRENT) {
        /* power delivery current values */
        case REG_PD_STATUS0_PD_SRC_CURRENT_0A5: i = HUSB_AMPS_0A5; break;
        case REG_PD_STATUS0_PD_SRC_CURRENT_0A7: i = HUSB_AMPS_0A7; break;
        case REG_PD_STATUS0_PD_SRC_CURRENT_1A: i = HUSB_AMPS_1A; break;
        case REG_PD_STATUS0_PD_SRC_CURRENT_1A25: i = HUSB_AMPS_1A25; break;
        case REG_PD_STATUS0_PD_SRC_CURRENT_1A5: i = HUSB_AMPS_1A5; break;
        case REG_PD_STATUS0_PD_SRC_CURRENT_1A75: i = HUSB_AMPS_1A75; break;
        case REG_PD_STATUS0_PD_SRC_CURRENT_2A: i = HUSB_AMPS_2A; break;
        case REG_PD_STATUS0_PD_SRC_CURRENT_2A25: i = HUSB_AMPS_2A25; break;
        case REG_PD_STATUS0_PD_SRC_CURRENT_2A5: i = HUSB_AMPS_2A5; break;
        case REG_PD_STATUS0_PD_SRC_CURRENT_2A75: i = HUSB_AMPS_2A75; break;
        case REG_PD_STATUS0_PD_SRC_CURRENT_3A: i = HUSB_AMPS_3A; break;
        case REG_PD_STATUS0_PD_SRC_CURRENT_3A25: i = HUSB_AMPS_3A25; break;
        case REG_PD_STATUS0_PD_SRC_CURRENT_3A5: i = HUSB_AMPS_3A5; break;
        case REG_PD_STATUS0_PD_SRC_CURRENT_4A: i = HUSB_AMPS_4A; break;
        case REG_PD_STATUS0_PD_SRC_CURRENT_4A5: i = HUSB_AMPS_4A5; break;
        case REG_PD_STATUS0_PD_SRC_CURRENT_5A: i = HUSB_AMPS_5A; break;
        }
    }

    /* store the voltages and currents */
    end: {
        if (volts) *volts = u;
        if (amps) *amps = i;
    }
    /* return the status */
    return ec;
}