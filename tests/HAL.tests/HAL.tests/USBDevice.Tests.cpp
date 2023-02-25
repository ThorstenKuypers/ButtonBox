
#include <gtest/gtest.h>
#include <gmock/gmock.h>

using namespace ::testing;

struct UsbSetupPacket
{
	uint8_t bmRequestType;
	uint8_t bRequest;
	uint8_t wValueL;
	uint8_t wValueH;
	uint16_t wIndex;
	uint16_t wLength;
};

struct UsbDeviceObserver
{
	virtual void SetupPacketReceived(UsbSetupPacket const&) = 0;
};

struct UsbBase
{
	virtual ~UsbBase() = default;

	virtual void Init() = 0;
	virtual void InitControlEp(uint8_t const& size) const = 0;
	virtual void WriteToControlEp(uint8_t const* buf, uint8_t const& size) = 0;
	virtual void ReadFromControlEp(uint8_t const* buf, uint8_t const& size) = 0;
	virtual void RegisterUsbDeviceObserver(UsbDeviceObserver& observer) = 0;
};

enum class UsbRequest :uint8_t
{
	SetAddress = 5,
	DeviceDescriptor = 6,
	GetConfiguration = 8,
	SetConfiguration = 9
};

enum class UsbRequestType :uint8_t
{
	DeviceRequest = 0,
	InterfaceRequest = 0x1,
	EndpointRequest = 0x2
};

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

enum class EnpointType : uint8_t
{
	Control = 0,
	Interrupt = 3
};

enum class EndpointDir :uint8_t
{
	Out = 0,
	In = 1
};

constexpr static uint8_t EnpointSize_8 = 8;
constexpr static uint8_t EnpointSize_16 = 16;
constexpr static uint8_t EnpointSize_32 = 32;
constexpr static uint8_t EnpointSize_64 = 64;

template<uint8_t size, EnpointType ep_type, EndpointDir ep_dir>
class UsbEnpoint
{
	EnpointType _type{ ep_type };
	EndpointDir _dir{ ep_dir };
	uint8_t _buf[size]{};

public:
	uint8_t inline dir() { return _dir; }

	uint8_t inline type() { return _type; }

	uint8_t inline size() { return size; }

	uint8_t& buffer() { return *_buf; }
};

class UsbDevice :public UsbDeviceObserver
{
	UsbBase& _usb;
	DeviceDescriptor _deviceDescriptor;
	uint8_t _ep0[EnpointSize_64];

	virtual void SetupPacketReceived(UsbSetupPacket const& setupPacket) noexcept override
	{
		HandleSetupPacket(setupPacket);
	}

public:

	UsbDevice() = default;
	explicit UsbDevice(UsbBase& usb) :
		_usb(usb),
		_ep0{ 0 }
	{
		_usb.Init();
		_usb.InitControlEp(EnpointSize_64);
		_usb.RegisterUsbDeviceObserver(*this);
	}

	virtual void HandleSetupPacket(UsbSetupPacket const& setup) noexcept
	{
		UsbRequest req = static_cast<UsbRequest>(setup.bRequest);
		UsbRequestType reqType = static_cast<UsbRequestType>(setup.bmRequestType & 0b00001111);

		if (UsbRequestType::DeviceRequest == reqType)
		{
			if (UsbRequest::DeviceDescriptor == req)
			{
				static_assert(18 == sizeof(_deviceDescriptor), "Wrong size of Device Descriptor");

				memcpy(_ep0, &_deviceDescriptor, sizeof(_deviceDescriptor));

				_usb.WriteToControlEp(_ep0, _deviceDescriptor.bLength);
			}
		}
	}
};

struct UsbMock : UsbBase
{
	MOCK_METHOD(void, Init, (), (override));
	MOCK_METHOD(void, InitControlEp, (uint8_t const&), (const override));
	MOCK_METHOD(void, WriteToControlEp, (uint8_t const*, uint8_t const&), (override));
	MOCK_METHOD(void, ReadFromControlEp, (uint8_t const*, uint8_t const&), (override));
	MOCK_METHOD(void, RegisterUsbDeviceObserver, (UsbDeviceObserver&), (override));
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
	uint8_t b[sizeof(DeviceDescriptor)] = {};
	DeviceDescriptor devDesc;
	UsbSetupPacket stp = { 0,6,0,0,0,0 };
	EXPECT_CALL(usb, WriteToControlEp(_, sizeof(DeviceDescriptor)))
		.WillOnce([&](uint8_t const* buf, uint8_t const& size)
			{
				memcpy(b, buf, size);
			}
	);
	UsbDevice usbDev{ usb };

	usbDev.HandleSetupPacket(stp);

	EXPECT_EQ(memcmp(b, &devDesc, sizeof(DeviceDescriptor)), 0);
}

TEST_F(UsbDevice_Should, SendAllDescriptors_WhenConfigurationRequestedByUsb)
{
	UsbSetupPacket stp = { 0 };
	//EXPECT_CALL(usb, WriteToControlEp(_, sizeof()))
}

TEST_F(UsbDevice_Should, TestObserver)
{
	UsbDeviceObserver* obs;
	UsbSetupPacket stp{ 0,6,0,0,0,0 };
	EXPECT_CALL(usb, RegisterUsbDeviceObserver(_))
		.WillOnce(
			[&](UsbDeviceObserver& _obs)
			{
				_obs.SetupPacketReceived(stp);
			}
	);
	usb.gmock_Init();
	UsbDevice usbDev{ usb };
}