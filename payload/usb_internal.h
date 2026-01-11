#ifndef CL64_USB_INTERNAL_H
#define CL64_USB_INTERNAL_H

#define CL64_MAGIC_N64_TO_PC 0x36343E50
#define CL64_MAGIC_PC_TO_N64 0x503E3634

unsigned cl64_usb_io_read(unsigned pi_address);

void cl64_usb_io_write(unsigned pi_address, unsigned value);

#endif
