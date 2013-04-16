/*
 * ptpusb.cpp
 *
 *  Created on: Apr 13, 2013
 *      Author: nick
 */
#include "libusb.h"
#include <iostream>
#include <cstdio>
#include <cstring>
#include <cstdlib>


using namespace std;

extern wchar_t *get_usb_string(libusb_device_handle *dev, uint8_t idx);

class PTPUSB
{
private:
	libusb_context                * m_ctx;
	libusb_device                 * m_dev;
	libusb_device_handle          * m_dev_handle;
	libusb_device_descriptor        m_desc;

	libusb_endpoint_descriptor    * m_endpoint_desc;
	int                             m_configuration;
	int                             m_alternate_setting;
	int                             m_interface;
	int                             m_in_ep;
	int                             m_out_ep;
	int                             m_int_ep;
	int                             m_err;
	bool find_interface_and_endpoints();
	bool check_and_configure_usb();
	void init();
	void uninit();
	void printdev(libusb_device *dev);
	const char* get_usb_error_string();
	void print_usb_error();
public:
	PTPUSB();
	~PTPUSB();

	bool open();
	bool display();

};



bool PTPUSB::display()
{
	//libusb_get_string_descriptor(libusb_device_handle *dev,
	//uint8_t desc_index, uint16_t langid, unsigned char *data, int length)

	if (m_dev_handle)
	{
		wchar_t* str;
		str = get_usb_string(m_dev_handle, m_desc.iManufacturer);
		printf("iManufacturer:%ls\n", str);
		free(str);

		str = get_usb_string(m_dev_handle, m_desc.iProduct);
		printf("iProduct:%ls\n", str);
		free(str);

		str = get_usb_string(m_dev_handle, m_desc.iSerialNumber);
		printf("iSerialNumber:%ls\n", str);
		free(str);

		return true;
	}
	return false;
}

PTPUSB::PTPUSB()
{
	m_ctx = NULL;
	m_dev = NULL;
	m_dev_handle = NULL;
	memset(&m_desc, 0, sizeof(m_desc));
	m_interface = 0;
	m_configuration = 0;
	m_in_ep = 0;
	m_out_ep = 0;
	m_int_ep = 0;
}

PTPUSB::~PTPUSB()
{
	if (m_dev_handle)
	{
		libusb_release_interface(m_dev_handle, m_interface);
		libusb_close(m_dev_handle);
	}
	if (m_dev)
	{
		libusb_unref_device(m_dev);
		m_dev = NULL;
	}
	if (m_ctx)
	{
		libusb_exit(m_ctx);
		m_ctx = NULL;
	}
}

void PTPUSB::print_usb_error()
{
	printf("libusb error:%s\n", get_usb_error_string());
}


const char* PTPUSB::get_usb_error_string()
{
	libusb_error err = (libusb_error)(m_err);
	switch (err)
	{
	/** Success (no error) */
	case LIBUSB_SUCCESS:
		return "Success (no error)";
		break;
	/** Input/output error */
	case LIBUSB_ERROR_IO:
		return "Input/output error";
		break;
		/** Invalid parameter */
	case LIBUSB_ERROR_INVALID_PARAM:
		return "Invalid parameter";
		break;
	/** Access denied (insufficient permissions) */
	case LIBUSB_ERROR_ACCESS:
		return "Access denied (insufficient permissions)";
		break;
	/** No such device (it may have been disconnected) */
	case LIBUSB_ERROR_NO_DEVICE:
		return "No such device (it may have been disconnected)";
		break;
	/** Entity not found */
	case LIBUSB_ERROR_NOT_FOUND:
		return "Entity not found";
		break;
	/** Resource busy */
	case LIBUSB_ERROR_BUSY:
		return "Resource busy";
		break;
	/** Operation timed out */
	case LIBUSB_ERROR_TIMEOUT:
		return "Operation timed out";
		break;
	/** Overflow */
	case LIBUSB_ERROR_OVERFLOW:
		return "Overflow";
		break;
	/** Pipe error */
	case LIBUSB_ERROR_PIPE:
		return "Pipe error";
		break;
	/** System call interrupted (perhaps due to signal) */
	case LIBUSB_ERROR_INTERRUPTED:
		return "System call interrupted (perhaps due to signal)";
		break;
	/** Insufficient memory */
	case LIBUSB_ERROR_NO_MEM:
		return "Insufficient memory";
		break;
	/** Operation not supported or unimplemented on this platform */
	case LIBUSB_ERROR_NOT_SUPPORTED:
		return "Operation not supported or unimplemented on this platform";
		break;
	/** Other error */
	case LIBUSB_ERROR_OTHER:
		return "Other error";
		break;
	default:
		return "Invalid error code";
		break;
	}
	// should never come here!
	return NULL;
}

bool PTPUSB::check_and_configure_usb()
{
	// check driver claimable
	if (libusb_kernel_driver_active(m_dev_handle, m_interface) == 1)
	{
		if ((m_err = libusb_detach_kernel_driver(m_dev_handle, m_interface)) != 0)
		{
			print_usb_error();
			return false;
		}
		if ((m_err=libusb_claim_interface(m_dev_handle, m_interface)) != 0)
		{
			print_usb_error();
			return false;
		}
	}
	return true;
}

bool PTPUSB::find_interface_and_endpoints()
{
	uint8_t i, j, k, l;

	// Loop over the device configurations
	for (i = 0; i < m_desc.bNumConfigurations; i++)
	{
		libusb_config_descriptor* _config;
		if ((m_err=libusb_get_config_descriptor(m_dev, i, &_config)) != 0)
		{
			print_usb_error();
			continue;
		}
		m_configuration = i;
		// Loop over each configurations interfaces
		for (j = 0; j < _config->bNumInterfaces; j++)
		{
			uint8_t no_ep;
			int found_inep = 0;
			int found_outep = 0;
			int found_intep = 0;

			m_interface = j;
			// Inspect the altsettings of this interface...
			for (k = 0; k < _config->interface[j].num_altsetting; k++)
			{
				m_alternate_setting = k;
				// MTP devices shall have 3 endpoints, ignore those interfaces
				// that haven't.
				no_ep = _config->interface[j].altsetting[k].bNumEndpoints;
				if (no_ep != 3)
				{
					continue;
				}

				const libusb_endpoint_descriptor* ep = _config->interface[j].altsetting[k].endpoint;

				// Loop over the three endpoints to locate two bulk and
				// one interrupt endpoint and FAIL if we cannot, and continue.
				for (l = 0; l < no_ep; l++)
				{
					if (ep[l].bmAttributes == LIBUSB_TRANSFER_TYPE_BULK)
					{
						if ((ep[l].bEndpointAddress & LIBUSB_ENDPOINT_DIR_MASK)
								== LIBUSB_ENDPOINT_DIR_MASK)
						{
							m_in_ep = ep[l].bEndpointAddress;
							found_inep = 1;
						}
						if ((ep[l].bEndpointAddress & LIBUSB_ENDPOINT_DIR_MASK)
								== 0)
						{
							m_out_ep = ep[l].bEndpointAddress;

							found_outep = 1;
						}
					}
					else if (ep[l].bmAttributes
							== LIBUSB_TRANSFER_TYPE_INTERRUPT)
					{
						if ((ep[l].bEndpointAddress & LIBUSB_ENDPOINT_DIR_MASK)
								== LIBUSB_ENDPOINT_DIR_MASK)
						{
							m_int_ep = ep[l].bEndpointAddress;
							found_intep = 1;
						}
					}
				}
				if (found_inep && found_outep && found_intep)
				{
					libusb_free_config_descriptor(_config);
					// We assigned the endpoints so return here.
					return true;
				}
			} // Next altsetting

		}// Next interface
		libusb_free_config_descriptor(_config);
	} // Next config
	return false;
}


void PTPUSB::printdev(libusb_device *dev)
{
	libusb_device_descriptor desc;
	int r = libusb_get_device_descriptor(dev, &desc);
	if (r < 0) {
		cout<<"failed to get device descriptor"<<endl;
		return;
	}
	cout<<"Number of possible configurations: "<<(int)desc.bNumConfigurations<<"  ";
	cout<<"Device Class: "<<(int)desc.bDeviceClass<<"  ";
	cout<<"VendorID: "<<desc.idVendor<<"  ";
	cout<<"ProductID: "<<desc.idProduct<<endl;
	libusb_config_descriptor *config;
	libusb_get_config_descriptor(dev, 0, &config);
	cout<<"Interfaces: "<<(int)config->bNumInterfaces<<" ||| ";
	const libusb_interface *inter;
	const libusb_interface_descriptor *interdesc;
	const libusb_endpoint_descriptor *epdesc;
	for(int i=0; i<(int)config->bNumInterfaces; i++) {
		inter = &config->interface[i];
		cout<<"Number of alternate settings: "<<inter->num_altsetting<<" | ";
		for(int j=0; j<inter->num_altsetting; j++) {
			interdesc = &inter->altsetting[j];
			cout<<"Interface Number: "<<(int)interdesc->bInterfaceNumber<<" | ";
			cout<<"Number of endpoints: "<<(int)interdesc->bNumEndpoints<<" | ";
			for(int k=0; k<(int)interdesc->bNumEndpoints; k++) {
				epdesc = &interdesc->endpoint[k];
				cout<<"Descriptor Type: "<<(int)epdesc->bDescriptorType<<" | ";
				cout<<"EP Address: "<<(int)epdesc->bEndpointAddress<<" | ";
			}
		}
	}
	cout<<endl<<endl<<endl;
	libusb_free_config_descriptor(config);
}

bool PTPUSB::open()
{
	bool _result = false;
	if ((m_err = libusb_init(&m_ctx)) != 0)
	{
		print_usb_error();
		return false;
	}
	libusb_set_debug(m_ctx, 3);
	libusb_device** _dev_list = NULL;
	int _list_size = libusb_get_device_list(m_ctx, &_dev_list);
	if (_list_size)
	{
		for (int i = 0; i < _list_size; i ++)
		{
			if ((m_err=libusb_get_device_descriptor(_dev_list[i], &m_desc)) == 0)
			{
				//if (m_desc.idVendor == 0x04e8 && m_desc.idProduct == 0x6860)
				if (m_desc.idVendor == 0x04e8 && m_desc.idProduct == 0x6865)
				{
					m_dev = libusb_ref_device(_dev_list[i]);
					break;
				}
			}
			else
			{
				print_usb_error();
			}

		}
		// we can free it,as we already ref the device
		libusb_free_device_list(_dev_list, 1);
	}
	if (m_dev)
	{
		printdev(m_dev);
		if ((m_err=libusb_open(m_dev, &m_dev_handle)) == 0)
		{
			if (find_interface_and_endpoints())
			{
				if (check_and_configure_usb())
				{
					_result = true;
				}
			}
		}
		else
		{
			print_usb_error();
		}
	}

	return _result;
}

uint16_t
ptp_getobjecthandles (PTPParams* params, uint32_t storage,
			uint32_t objectformatcode, uint32_t associationOH,
			PTPObjectHandles* objecthandles)
{
	uint16_t ret;
	PTPContainer ptp;
	char* oh=NULL;

	ptp_debug(params,"PTP: Obtaining ObjectHandles");

	PTP_CNT_INIT(ptp);
	ptp.Code=PTP_OC_GetObjectHandles;
	ptp.Param1=storage;
	ptp.Param2=objectformatcode;
	ptp.Param3=associationOH;
	ptp.Nparam=3;
	ret=ptp_transaction(params, &ptp, PTP_DP_GETDATA, 0, &oh);
	if (ret == PTP_RC_OK) ptp_unpack_OH(params, oh, objecthandles);
	free(oh);
	return ret;
}


int main()
{
	PTPUSB ptpusb;
	if (ptpusb.open())
	{
		if (ptpusb.display())
		{
			cout << "open successfully"<<endl;
		}
	}
	return 0;

}
