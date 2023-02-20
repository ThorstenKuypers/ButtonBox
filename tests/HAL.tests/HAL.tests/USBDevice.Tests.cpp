
#include <gtest/gtest.h>
#include <gmock/gmock.h>

using namespace ::testing;

struct UsbBase
{
	virtual ~UsbBase() = default;

	virtual void Init() = 0;
	virtual void InitControlEp(uint8_t const& size) const = 0;
	virtual void Send(uint8_t const* buf, uint8_t const& size) const = 0;
};

struct UsbSetupPacket
{
	uint8_t bmRequestType;
	uint8_t bRequest;
	uint8_t wValueL;
	uint8_t wValueH;
	uint16_t wIndex;
	uint16_t wLength;
};

enum class UsbRequestType
{

};

enum class UsbRequest :uint8_t
{
	DeviceDescriptor = 6
};

const uint8_t devDesc[18] = {
	18,
	1,
	0x00, 0x02,
	0,
	0,
	0,
	64,
	0xED, 0xFE,
	0xEF, 0xBE,
	0x00, 0x01,
	0,
	0,
	0,
	1 };

struct DeviceDescriptor
{
	uint8_t bLength{ 18 };
	uint8_t bDescriptorType{ 1 };
	uint16_t bcdUSB{ 0x200 };
	uint8_t bDeviceClass{};
	uint8_t bDeviceSubClass{};
	uint8_t bDeviceProtocol{};
	uint8_t bMaxPacketSize{ 64 };
	uint16_t idVendor{ 0xFEED };
	uint16_t idProduct{ 0xBEEF };
	uint16_t bcdDevice{ 0x100 };
	uint8_t iManufacturer{};
	uint8_t iProduct{};
	uint8_t iSerialNumber{};
	uint8_t bNumConfigurations{ 1 };
};

enum EnpointType : uint8_t
{
	Control = 0,
	Interrupt
};

constexpr static uint8_t EnpointSize_8 = 8;
constexpr static uint8_t EnpointSize_16 = 16;
constexpr static uint8_t EnpointSize_32 = 32;
constexpr static uint8_t EnpointSize_64 = 64;

template<uint8_t size, EnpointType ep_type>
struct UsbEnpoint
{
	EnpointType _type{ ep_type };
	uint8_t _buf[size]{};
};

class UsbDevice
{
	UsbBase& _usb;
	UsbEnpoint<EnpointSize_64, EnpointType::Control> _controlEp;

	DeviceDescriptor _deviceDescriptor;

public:

	UsbDevice() = default;
	explicit UsbDevice(UsbBase& usb) :
		_usb(usb)
	{
		_usb.Init();
		_usb.InitControlEp(EnpointSize_64);
	}

	virtual void HandleSetupPacket(UsbSetupPacket& setup) noexcept
	{
		UsbRequest req = static_cast<UsbRequest>(setup.bRequest);

		if (UsbRequest::DeviceDescriptor == req)
		{
			static_assert(18 == sizeof(_deviceDescriptor),"Wrong size of Device Descriptor");

			memcpy(_controlEp._buf, &_deviceDescriptor, sizeof(_deviceDescriptor));

			_usb.Send(_controlEp._buf, _deviceDescriptor.bLength);
		}
	}
};

struct UsbMock : UsbBase
{
	MOCK_METHOD(void, Init, (), (override));
	MOCK_METHOD(void, InitControlEp, (uint8_t const&), (const override));
	MOCK_METHOD(void, Send, (uint8_t const*, uint8_t const&), (const override));
};

struct UsbDevice_Should : ::testing::Test
{
	NiceMock<UsbMock> usb;
};

static uint8_t r = 0;
TEST_F(UsbDevice_Should, InitializeUsbHardware)
{
	EXPECT_CALL(usb, Init());
	UsbDevice usbDev{ usb };
}

TEST_F(UsbDevice_Should, InitializeControlEp)
{
	EXPECT_CALL(usb, InitControlEp(EnpointSize_64));
	UsbDevice usbDev{ usb };
}

TEST_F(UsbDevice_Should, SendDeviceDescriptor_WhenRequested)
{
	uint8_t b[sizeof(devDesc)] = {};
	UsbSetupPacket stp = { 0,6,0,0,0,0 };
	EXPECT_CALL(usb, Send(_, sizeof(devDesc)))
		.WillOnce([&](uint8_t const* buf, uint8_t const& size) {
		memcpy(b, buf, size);
			});
	UsbDevice usbDev{ usb };

	usbDev.HandleSetupPacket(stp);

	EXPECT_EQ(memcmp(b, devDesc, sizeof(devDesc)), 0);
}