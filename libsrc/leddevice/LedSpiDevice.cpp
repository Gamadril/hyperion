#include <fcntl.h>
#include <unistd.h>
#include <sys/ioctl.h>

#include "LedSpiDevice.h"

LedSpiDevice::LedSpiDevice(const std::string& outputDevice, const unsigned baudrate, const int latchTime_ns) :
    _deviceName(outputDevice),
    _baudRate(baudrate),
    _latchTime_ns(latchTime_ns),
    _fid(-1)
{
    memset(&_spi, 0, sizeof(_spi));
}

LedSpiDevice::~LedSpiDevice()
{
	close(_fid);
}

int LedSpiDevice::open()
{
    const int bitsPerWord = 8;

    _fid = ::open(_deviceName.c_str(), O_RDWR);

    if (_fid < 0)
    {
        std::cerr << "Failed to open device('" << _deviceName << "') " << std::endl;
        return -1;
    }

    int mode = SPI_MODE_0;
    if (ioctl(_fid, SPI_IOC_WR_MODE, &mode) == -1 || ioctl(_fid, SPI_IOC_RD_MODE, &mode) == -1)
    {
        return -2;
    }

    if (ioctl(_fid, SPI_IOC_WR_BITS_PER_WORD, &bitsPerWord) == -1 || ioctl(_fid, SPI_IOC_RD_BITS_PER_WORD, &bitsPerWord) == -1)
    {
        return -4;
    }

    if (ioctl(_fid, SPI_IOC_WR_MAX_SPEED_HZ, &_baudRate) == -1 || ioctl(_fid, SPI_IOC_RD_MAX_SPEED_HZ, &_baudRate) == -1)
    {
        return -6;
    }

    return 0;
}

int LedSpiDevice::writeBytes(const unsigned size, const uint8_t * data)
{
    if (_fid < 0)
    {
        return -1;
    }

    _spi.tx_buf = __u64(data);
    _spi.len    = __u32(size);

    int retVal = ioctl(_fid, SPI_IOC_MESSAGE(1), &_spi);

    if (retVal == 0 && _latchTime_ns > 0)
    {
        // The 'latch' time for latching the shifted-value into the leds
        timespec latchTime;
        latchTime.tv_sec  = 0;
        latchTime.tv_nsec = _latchTime_ns;

        // Sleep to latch the leds (only if write succesfull)
        nanosleep(&latchTime, NULL);
    }

    return retVal;
}
