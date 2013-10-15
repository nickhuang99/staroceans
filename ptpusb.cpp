/*
 * ptpusb.cpp
 *
 *  Created on: Apr 13, 2013
 *      Author: nick
 */
#define HAVE_ICONV 1


#include <iostream>
#include <cstdio>
#include <cstring>
#include <cstdlib>
#include <iconv.h>
#include "ptpusb.h"
#include "gphoto2-endian.h"
#include <string>
#include <sys/stat.h>
#include <sys/types.h>
#include <errno.h>
#include <stdarg.h>
#include <unistd.h>

using namespace std;
#pragma pack(push)
#pragma pack(2)

extern int errno;

#define MAX_PATH 256

#define LIBUSB_READ_TIMEOUT 5000
#define LIBUSB_WRITE_TIMEOUT 5000
#define LIBUSB_EVENT_TIMEOUT  1000

extern wchar_t *get_usb_string(libusb_device_handle *dev, uint8_t idx);

/* constants for GetObjectHandles */
#define PTP_GOH_ALL_STORAGE 0xffffffff
#define PTP_GOH_ALL_FORMATS 0x00000000
#define PTP_GOH_ALL_ASSOCS  0x00000000
#define PTP_GOH_ROOT_PARENT 0xffffffff
#define PTP_GOH_ROOT_OBJECT_PARENT 0x00000000



/* PTP Association Types */
#define PTP_AT_Undefined			0x0000
#define PTP_AT_GenericFolder			0x0001
#define PTP_AT_Album				0x0002
#define PTP_AT_TimeSequence			0x0003
#define PTP_AT_HorizontalPanoramic		0x0004
#define PTP_AT_VerticalPanoramic		0x0005
#define PTP_AT_2DPanoramic			0x0006
#define PTP_AT_AncillaryData			0x0007

#define PTP_USB_CONTAINER_UNDEFINED		0x0000
#define PTP_USB_CONTAINER_COMMAND		0x0001
#define PTP_USB_CONTAINER_DATA			0x0002
#define PTP_USB_CONTAINER_RESPONSE		0x0003
#define PTP_USB_CONTAINER_EVENT			0x0004

#define htod8a(a,x)	*(uint8_t*)(a) = x
#define htod16a(a,x)	htole16a(params,(unsigned char *)a,x)
#define htod32a(a,x)	htole32a((unsigned char *)a,x)
#define htod16(x)	htole16(x)
#define htod32(x)	htole32(x)
#define htod64(x)	htole64(x)

#define dtoh8a(x)	(*(uint8_t*)(x))
#define dtoh16a(a)	le16atoh((unsigned char *)a)
#define dtoh32a(a)	le32atoh((unsigned char *)a)
#define dtoh64a(a)	le64atoh((unsigned char *)a)

#define dtoh16(x)	le16toh(x)
#define dtoh32(x)	le32toh(x)
#define dtoh64(x)	le64toh(x)


/* PTP v1.0 operation codes */
#define PTP_OC_Undefined                0x1000
#define PTP_OC_GetDeviceInfo            0x1001
#define PTP_OC_OpenSession              0x1002
#define PTP_OC_CloseSession             0x1003
#define PTP_OC_GetStorageIDs            0x1004
#define PTP_OC_GetStorageInfo           0x1005
#define PTP_OC_GetNumObjects            0x1006
#define PTP_OC_GetObjectHandles         0x1007
#define PTP_OC_GetObjectInfo            0x1008
#define PTP_OC_GetObject                0x1009
#define PTP_OC_GetThumb                 0x100A
#define PTP_OC_DeleteObject             0x100B
#define PTP_OC_SendObjectInfo           0x100C
#define PTP_OC_SendObject               0x100D
#define PTP_OC_InitiateCapture          0x100E
#define PTP_OC_FormatStore              0x100F
#define PTP_OC_ResetDevice              0x1010
#define PTP_OC_SelfTest                 0x1011
#define PTP_OC_SetObjectProtection      0x1012
#define PTP_OC_PowerDown                0x1013
#define PTP_OC_GetDevicePropDesc        0x1014
#define PTP_OC_GetDevicePropValue       0x1015
#define PTP_OC_SetDevicePropValue       0x1016
#define PTP_OC_ResetDevicePropValue     0x1017
#define PTP_OC_TerminateOpenCapture     0x1018
#define PTP_OC_MoveObject               0x1019
#define PTP_OC_CopyObject               0x101A
#define PTP_OC_GetPartialObject         0x101B
#define PTP_OC_InitiateOpenCapture      0x101C
/* PTP v1.1 operation codes */
#define PTP_OC_StartEnumHandles		0x101D
#define PTP_OC_EnumHandles		0x101E
#define PTP_OC_StopEnumHandles		0x101F
#define PTP_OC_GetVendorExtensionMaps	0x1020
#define PTP_OC_GetVendorDeviceInfo	0x1021
#define PTP_OC_GetResizedImageObject	0x1022
#define PTP_OC_GetFilesystemManifest	0x1023
#define PTP_OC_GetStreamInfo		0x1024
#define PTP_OC_GetStream		0x1025

/* Response Codes */

#define PTP_RC_Undefined                0x2000
#define PTP_RC_OK                       0x2001
#define PTP_RC_GeneralError             0x2002
#define PTP_RC_SessionNotOpen           0x2003
#define PTP_RC_InvalidTransactionID	    0x2004
#define PTP_RC_OperationNotSupported    0x2005
#define PTP_RC_ParameterNotSupported    0x2006
#define PTP_RC_IncompleteTransfer       0x2007
#define PTP_RC_InvalidStorageId         0x2008
#define PTP_RC_InvalidObjectHandle      0x2009
#define PTP_RC_DevicePropNotSupported   0x200A
#define PTP_RC_InvalidObjectFormatCode  0x200B
#define PTP_RC_StoreFull                0x200C
#define PTP_RC_ObjectWriteProtected     0x200D
#define PTP_RC_StoreReadOnly            0x200E
#define PTP_RC_AccessDenied             0x200F
#define PTP_RC_NoThumbnailPresent       0x2010
#define PTP_RC_SelfTestFailed           0x2011
#define PTP_RC_PartialDeletion          0x2012
#define PTP_RC_StoreNotAvailable        0x2013
#define PTP_RC_SpecificationByFormatUnsupported         0x2014
#define PTP_RC_NoValidObjectInfo        0x2015
#define PTP_RC_InvalidCodeFormat        0x2016
#define PTP_RC_UnknownVendorCode        0x2017
#define PTP_RC_CaptureAlreadyTerminated 0x2018
#define PTP_RC_DeviceBusy               0x2019
#define PTP_RC_InvalidParentObject      0x201A
#define PTP_RC_InvalidDevicePropFormat  0x201B
#define PTP_RC_InvalidDevicePropValue   0x201C
#define PTP_RC_InvalidParameter         0x201D
#define PTP_RC_SessionAlreadyOpened     0x201E
#define PTP_RC_TransactionCanceled      0x201F
#define PTP_RC_SpecificationOfDestinationUnsupported            0x2020


/* PTP Event Codes */

#define PTP_EC_Undefined		0x4000
#define PTP_EC_CancelTransaction	0x4001
#define PTP_EC_ObjectAdded		0x4002
#define PTP_EC_ObjectRemoved		0x4003
#define PTP_EC_StoreAdded		0x4004
#define PTP_EC_StoreRemoved		0x4005
#define PTP_EC_DevicePropChanged	0x4006
#define PTP_EC_ObjectInfoChanged	0x4007
#define PTP_EC_DeviceInfoChanged	0x4008
#define PTP_EC_RequestObjectTransfer	0x4009
#define PTP_EC_StoreFull		0x400A
#define PTP_EC_DeviceReset		0x400B
#define PTP_EC_StorageInfoChanged	0x400C
#define PTP_EC_CaptureComplete		0x400D
#define PTP_EC_UnreportedStatus		0x400E


/* PTP class specific requests */
#ifndef USB_REQ_DEVICE_RESET
#define USB_REQ_DEVICE_RESET		0x66
#endif
#ifndef USB_REQ_GET_DEVICE_STATUS
#define USB_REQ_GET_DEVICE_STATUS	0x67
#endif

#define DEBUG_ENTRY fprintf(m_log, "[%d][%s]\n", __LINE__, __PRETTY_FUNCTION__);
//ObjectInfoVector PTPUSB::m_object_info_vector;

int PTPUSB::unpack_string(const char* str, char* dst)
{
	unsigned char len = str[0];

	strncpy(dst, str+1, len);
	dst[len] = '\0';

	return len;
}



void PTPUSB::print_ptp_error(uint16_t err)
{
#define N_(str) (str)
	unsigned int i;
	/* PTP error descriptions */
	static struct
	{
		uint16_t error;
		const char *txt;
	} ptp_errors[] =
	{
		{PTP_RC_Undefined, 		N_("PTP: Undefined Error")},
		{PTP_RC_OK, 			N_("PTP: OK!")},
		{PTP_RC_GeneralError, 		N_("PTP: General Error")},
		{PTP_RC_SessionNotOpen, 	N_("PTP: Session Not Open")},
		{PTP_RC_InvalidTransactionID, 	N_("PTP: Invalid Transaction ID")},
		{PTP_RC_OperationNotSupported, 	N_("PTP: Operation Not Supported")},
		{PTP_RC_ParameterNotSupported, 	N_("PTP: Parameter Not Supported")},
		{PTP_RC_IncompleteTransfer, 	N_("PTP: Incomplete Transfer")},
		{PTP_RC_InvalidStorageId, 	N_("PTP: Invalid Storage ID")},
		{PTP_RC_InvalidObjectHandle, 	N_("PTP: Invalid Object Handle")},
		{PTP_RC_DevicePropNotSupported, N_("PTP: Device Prop Not Supported")},
		{PTP_RC_InvalidObjectFormatCode, N_("PTP: Invalid Object Format Code")},
		{PTP_RC_StoreFull, 		N_("PTP: Store Full")},
		{PTP_RC_ObjectWriteProtected, 	N_("PTP: Object Write Protected")},
		{PTP_RC_StoreReadOnly, 		N_("PTP: Store Read Only")},
		{PTP_RC_AccessDenied,		N_("PTP: Access Denied")},
		{PTP_RC_NoThumbnailPresent, 	N_("PTP: No Thumbnail Present")},
		{PTP_RC_SelfTestFailed, 	N_("PTP: Self Test Failed")},
		{PTP_RC_PartialDeletion, 	N_("PTP: Partial Deletion")},
		{PTP_RC_StoreNotAvailable, 	N_("PTP: Store Not Available")},
		{PTP_RC_SpecificationByFormatUnsupported,
					N_("PTP: Specification By Format Unsupported")},
		{PTP_RC_NoValidObjectInfo, 	N_("PTP: No Valid Object Info")},
		{PTP_RC_InvalidCodeFormat, 	N_("PTP: Invalid Code Format")},
		{PTP_RC_UnknownVendorCode, 	N_("PTP: Unknown Vendor Code")},
		{PTP_RC_CaptureAlreadyTerminated,
						N_("PTP: Capture Already Terminated")},
		{PTP_RC_DeviceBusy, 		N_("PTP: Device Busy")},
		{PTP_RC_InvalidParentObject, 	N_("PTP: Invalid Parent Object")},
		{PTP_RC_InvalidDevicePropFormat, N_("PTP: Invalid Device Prop Format")},
		{PTP_RC_InvalidDevicePropValue, N_("PTP: Invalid Device Prop Value")},
		{PTP_RC_InvalidParameter, 	N_("PTP: Invalid Parameter")},
		{PTP_RC_SessionAlreadyOpened, 	N_("PTP: Session Already Opened")},
		{PTP_RC_TransactionCanceled, 	N_("PTP: Transaction Canceled")},
		{PTP_RC_SpecificationOfDestinationUnsupported,
				N_("PTP: Specification Of Destination Unsupported")},

	};


	for (i=0; i < sizeof(ptp_errors)/sizeof(ptp_errors[0]); i++)
	{
		if (ptp_errors[i].error == err)
		{
			printf("ptp error: %s\n", ptp_errors[i].txt);
		}
	}
}


bool PTPUSB::recv_all_events()
{
	int actual_size = 0;
	PTPUSBEventContainer event;
	int rc;

	if ((rc = libusb_interrupt_transfer(m_dev_handle, m_int_ep, (unsigned char *)&event,
						sizeof(event), &actual_size, LIBUSB_EVENT_TIMEOUT))!= 0)
	{
		if (rc != LIBUSB_ERROR_TIMEOUT)
		{
			return false;
		}
		else
		{
			return true;
		}
	}
	if(actual_size != sizeof(event))
	{
		return false;
	}
	debug_msg("[recv event]code:%lu,length:%lu, param1:%lu, param2:%lu, param3:%lu\n",
			event.code, event.length, event.param1, event.param2, event.param3);
	event.code = dtoh16(event.code);
	event.length = dtoh32(event.length);
	event.trans_id = dtoh32(event.trans_id);
	event.type = dtoh16(event.type);
	event.param1 = dtoh32(event.param1);
	event.param2 = dtoh32(event.param2);
	event.param3 = dtoh32(event.param3);

	m_incoming_event_queue.push_back(event);
	return true;
}


void* ptp_event_proc(void *arg)
{
	PTPUSB* ptpusb = (PTPUSB*)arg;
	while (!ptpusb->m_stop_event_thread)
	{
		ptpusb->recv_all_events();
	}
	return NULL;
}

bool PTPUSB::start_event_thread()
{
	//return true;
	if(pthread_create(&m_event_thread_ctx, NULL, ptp_event_proc, this) == 0)
	{
		m_event_thread_running = true;
		return true;
	}
	return false;
}

// let's try to reset device and close possible previous session
bool PTPUSB::init()
{
	bool bresult = false;
	if (!close_session(true))
	{
		printf("close session failed\n");
		//return false;
	}
	uint16_t status = 0;
	while ((bresult = get_device_status(status)))
	{
		switch (status)
		{
		case PTP_RC_OK:
		//case PTP_RC_DeviceBusy:
			self_test();
			printf("device is ok\n");
			return true;
		default:
			print_ptp_error(status);
			if (!reset_device())
			{
				printf("reset device failed\n");
				return false;
			}
			break;
		}
	}
	if (bresult)
	{
		self_test();
		return reset_device();
	}
	return bresult;
}

bool PTPUSB::open_session()
{
	PTPContainerWrapper ptp;
	PTPContainer& req = ptp.container;
	//m_session = rand()%65535 + 1;
	m_session = 500;
	m_transaction = 0;

	ptp.param_number = 1;
	req.Code = PTP_OC_OpenSession;
	req.SessionID = 0;
	req.Transaction_ID = m_transaction;
	req.Param1 = m_session;
	if (send_request(ptp))
	{
		PTPContainer resp;
		if (get_response(resp))
		{
			if(resp.Code == PTP_RC_OK)
			{
				m_session_opened = true;
				return start_event_thread();
			}
			print_ptp_error(resp.Code);
		}
	}
	return false;
}


bool PTPUSB::close_session(bool by_force)
{
	bool bResult = false;
	if (!by_force && !m_session_opened)
	{
		return true;
	}
	m_stop_event_thread = true;
	void* arg;
	//pthread_join(m_event_thread_ctx, &arg);

	PTPContainerWrapper ptp;
	PTPContainer& req = ptp.container;

	ptp.param_number = 0;
	req.Code = PTP_OC_CloseSession;
	//req.SessionID = 0;
	req.Transaction_ID = 0;
	req.SessionID = m_session;
	//req.Transaction_ID = m_transaction;
	if (send_request(ptp))
	{
		bResult = true;
	}
	PTPContainer resp;
	if (get_response(resp))
	{
		if( resp.Code == PTP_RC_OK)
		{
			m_session_opened = bResult;
		}
		else
		{
			bResult = false;
		}
		print_ptp_error(resp.Code);
	}
	return bResult;
}
//libusb_device_handle *dev_handle,
//uint8_t request_type, uint8_t bRequest, uint16_t wValue, uint16_t wIndex,
//unsigned char *data, uint16_t wLength, unsigned int timeout);

bool PTPUSB::reset_device()
{
	reset_device_by_request();
	reset_device_by_control();
	return true;
}

bool PTPUSB::reset_device_by_control()
{
	uint8_t req_type = LIBUSB_ENDPOINT_OUT
			|LIBUSB_REQUEST_TYPE_CLASS
			|LIBUSB_RECIPIENT_INTERFACE;
	uint8_t req_code = USB_REQ_DEVICE_RESET; //Get_Device_Status
	int rc = libusb_control_transfer(m_dev_handle, req_type, req_code, 0, 0,
			NULL,0, 0);
	if (rc == 0)
	{
		// successful
		printf("successfully reset device");
		return true;
	}
	return false;
}

bool PTPUSB::reset_device_by_request()
{
	bool bResult = false;

	PTPContainerWrapper ptp;
	PTPContainer& req = ptp.container;

	ptp.param_number = 0;
	req.Code = PTP_OC_ResetDevice;
	//req.SessionID = 0;
	req.Transaction_ID = 0;
	req.SessionID = m_session;
	//req.Transaction_ID = m_transaction;
	if (!send_request(ptp))
	{
		return false;
	}
	PTPContainer resp;
	if (get_response(resp))
	{
		if( resp.Code == PTP_RC_OK)
		{
			bResult = true;
		}
		else
		{
			print_ptp_error(resp.Code);
		}
	}
	return bResult;
}

bool PTPUSB::self_test()
{
	bool bResult = false;

	PTPContainerWrapper ptp = {0};
	PTPContainer& req = ptp.container;

	ptp.param_number = 1;
	req.Code = PTP_OC_SelfTest;
	//req.SessionID = 0;
	req.Transaction_ID = 0;
	req.SessionID = m_session;
	req.Param1 = 0x00000000;
	//req.Transaction_ID = m_transaction;
	if (!send_request(ptp))
	{
		return false;
	}
	PTPContainer resp;
	if (get_response(resp))
	{
		if( resp.Code == PTP_RC_OK)
		{
			bResult = true;
		}
		else
		{
			print_ptp_error(resp.Code);
		}
	}
	return bResult;
}

// need htod convertion!!!!!
bool PTPUSB::get_device_status(uint16_t& status)
{
	uint8_t req_type = LIBUSB_ENDPOINT_IN
			|LIBUSB_REQUEST_TYPE_CLASS
			|LIBUSB_RECIPIENT_INTERFACE;
	uint8_t req_code = USB_REQ_GET_DEVICE_STATUS ; //Get_Device_Status
	PTPUSBRequestContainer req = {0};
	int rc = libusb_control_transfer(m_dev_handle, req_type, req_code, 0, 0,
			(unsigned char*)&req, sizeof(req), 0);
	if (rc >= 0)
	{
		// successful
		status = dtoh16(req.code);
		print_ptp_error(req.code);
		printf("req length:%u, device status is:%04x\n", dtoh16(req.length), status);
		return true;
	}
	return false;
}

bool PTPUSB::get_storage_id()
{
	bool bResult = false;
	PTPContainerWrapper ptp;
	PTPContainer& req = ptp.container;

	ptp.param_number = 0;
	req.Code = PTP_OC_GetStorageIDs;
	req.SessionID = m_session;
	req.Transaction_ID = m_transaction;

	if (send_request(ptp))
	{
		PTPUSBBulkContainer resp;
		if (get_data(resp))
		{
			if(resp.code == PTP_OC_GetStorageIDs)
			{
				bResult = true;
				uint32_t* ptr = (uint32_t*)resp.payload.data;
				m_storage_count = dtoh32(*ptr);
				if (m_storage_count > MAX_STORAGE_COUNT)
				{
					m_storage_count = MAX_STORAGE_COUNT;
					printf("exceeds max storage count");
				}

				ptr ++;
				for (int i = 0; i < m_storage_count; i ++)
				{
					m_storage_array[i].StorageId = dtoh32(*ptr);
					printf("storage[%d]=%d\t", i, m_storage_array[i].StorageId);
					ptr ++;
				}
				printf("total storage number %d\n", m_storage_count);
			}
			//print_ptp_error(resp.code);
		}
		PTPContainer res;
		if (get_response(res))
		{
			if( res.Code != PTP_RC_OK)
			{
				bResult = false;
			}
			print_ptp_error(res.Code);
		}
		else
		{
			bResult = false;
		}
	}
	return bResult;
}

bool PTPUSB::get_storage_object_number()
{
	for (int i = 0; i < m_storage_count; i ++)
	{
		PTPContainerWrapper ptp;
		PTPContainer& req = ptp.container;

		ptp.param_number = 1;
		req.Code = PTP_OC_GetNumObjects;
		req.SessionID = m_session;
		req.Transaction_ID = m_transaction;
		req.Param1 = m_storage_array[i].StorageId;
		req.Param2 = 0xFFFFFFFF;
		req.Param3 = 0x00000000;

		if (send_request(ptp))
		{
			PTPContainer res;
			if (get_response(res))
			{
				if( res.Code == PTP_RC_OK)
				{
					m_storage_array[i].ObjectNumber = res.Param1;

				}
				print_ptp_error(res.Code);
			}
		}
	}
	return true;
}


bool PTPUSB::get_storage_info()
{
	for (int i = 0; i < m_storage_count; i ++)
	{
		PTPContainerWrapper ptp;
		PTPContainer& req = ptp.container;

		ptp.param_number = 1;
		req.Code = PTP_OC_GetStorageInfo;
		req.SessionID = m_session;
		req.Transaction_ID = m_transaction;
		req.Param1 = m_storage_array[i].StorageId;
		if (send_request(ptp))
		{
			PTPUSBBulkContainer resp;
			if (get_data(resp))
			{
				if(resp.code == PTP_OC_GetStorageInfo)
				{
					PTPStorageInfoHeader* ptr = (PTPStorageInfoHeader*)resp.payload.data;
					/*
					struct PTPStorageInfo {
						uint16_t StorageType;
						uint16_t FilesystemType;
						uint16_t AccessCapability;
						uint64_t MaxCapability;
						uint64_t FreeSpaceInBytes;
						uint32_t FreeSpaceInImages;
						char 	*StorageDescription;
						char	*VolumeLabel;
					};
					*/
					PTPStorageInfoHeader& header = m_storage_array[i].info.header;
					header.StorageType = dtoh16(ptr->StorageType);
					header.FilesystemType = dtoh16(ptr->FilesystemType);
					header.AccessCapability = dtoh16(ptr->AccessCapability);
					header.MaxCapability = dtoh64(ptr->MaxCapability);
					header.FreeSpaceInBytes = dtoh64(ptr->FreeSpaceInBytes);
					header.FreeSpaceInImages = dtoh32(ptr->FreeSpaceInImages);

					uint8_t len = 0;
					m_storage_array[i].info.StorageDescription =
							ptp_unpack_string(resp.payload.data, sizeof(PTPStorageInfoHeader), len);

					m_storage_array[i].info.VolumeLabel =
							ptp_unpack_string(resp.payload.data, sizeof(PTPStorageInfoHeader)+len, len);


					printf("StorageType=%2u, FilesystemType=%2u, AccessCapability=%2u"
							"MaxCapability=%8u, FreeSpaceInBytes=%8u, FreeSpaceInImages=%4u\n",
							header.StorageType,
							header.FilesystemType,
							header.AccessCapability,
							header.MaxCapability,
							header.FreeSpaceInBytes,
							header.FreeSpaceInImages
							);

					printf("storage description:%s\n", m_storage_array[i].info.StorageDescription.c_str());


					printf("volume label: %s\n", m_storage_array[i].info.VolumeLabel.c_str());


				}

			}
			PTPContainer res;
			if (get_response(res))
			{
				if( res.Code == PTP_RC_OK)
				{
					return true;
				}
				print_ptp_error(res.Code);
			}
		}
	}

	return false;
}

bool PTPUSB::get_object(const PTPObjectInfoWrapper& objInfo, const char*file_name)
{
	if (objInfo.info.head.AssociationType == PTP_AT_GenericFolder
			|| objInfo.info.head.AssociationType == PTP_AT_Album)
	{
		return true;
	}
	PTPContainerWrapper ptp;
	PTPContainer& req = ptp.container;

	memset(&ptp,0,sizeof(ptp));
	ptp.param_number = 1;
	req.Code = PTP_OC_GetObject;
	req.SessionID = m_session;
	req.Transaction_ID = m_transaction;
	req.Param1 = objInfo.Handle;

	if (send_request(ptp))
	{
		if (!get_object_transaction(objInfo, file_name))
		{
			printf("get_object_handles failed\n");
			return false;
		}

	}

	return true;
}

#define PTP_HANDLER_ROOT	0x00000000

bool PTPUSB::get_all_objects(uint32_t storage_id, bool to_download_object)
{
	PTPContainerWrapper ptp;
	PTPContainer& req = ptp.container;
	HandleQueue handleQueue;
	uint32_t current_handle = PTP_HANDLER_ROOT;
	if (!get_object_handles(storage_id, current_handle, handleQueue))
	{
		return false;
	}
	while (true)
	{
		printf("queue size = %lu\n", handleQueue.size());
		if (handleQueue.empty())
		{
			break;
		}
		current_handle = handleQueue.front();
		handleQueue.pop_front();
		PTPObjectInfoWrapper infoWrapper;
		PTPObjectInfo& objInfo = infoWrapper.info;
		infoWrapper.Handle = current_handle;
		if (!get_object_info(current_handle, objInfo))
		{
			break;
		}
		printf("obj filename:%s\n", objInfo.Filename.c_str());
		char file_name[MAX_PATH];
		if (!prepare_file_write(infoWrapper, file_name))
		{
			printf("failed to prepare write for %s\n", infoWrapper.info.Filename.c_str());
			return false;
		}
		if (to_download_object && !get_object(infoWrapper, file_name))
		{
			printf("get object %s failed", objInfo.Filename.c_str());
			break;
		}
		if (objInfo.Filename.compare("journal") == 0)
		{
			printf("it is\n");
		}
		m_objInfoSet.insert(infoWrapper);
		m_name_handle_map.insert(make_pair(infoWrapper.info.Filename, infoWrapper.Handle));
		printf("infoset size=%lu\n", m_objInfoSet.size());
		if (m_objInfoSet.size() == 27)
		{
			printf("here it is\n");
		}
		if (objInfo.head.AssociationType == PTP_AT_GenericFolder
				|| objInfo.head.AssociationType == PTP_AT_Album)
		{
			if (!get_object_handles(storage_id, current_handle, handleQueue))
			{
				return false;
			}
		}

	}
	return true;
}


bool PTPUSB::search_object_info_by_handle(uint32_t handle, PTPObjectInfo& objInfo)
{
	PTPObjectInfoWrapper tmp;
	tmp.Handle = handle;
	ObjectInfoSet::iterator it = m_objInfoSet.find(tmp);

}
bool PTPUSB::search_object_info_by_name(const string& fileName, ObjectInfoVector& objInfoVect)
{

}

bool PTPUSB::get_object_handles(uint32_t storage_id, uint32_t parent_handle, HandleQueue& handleQueue)
{
	PTPContainerWrapper ptp;
	PTPContainer& req = ptp.container;

	memset(&ptp,0,sizeof(ptp));
	ptp.param_number = 3;
	req.Code = PTP_OC_GetObjectHandles;
	req.SessionID = m_session;
	req.Transaction_ID = m_transaction;
	req.Param1 = storage_id;
	req.Param2 = 0x00000000;
	req.Param3 = parent_handle;
	if (send_request(ptp))
	{
		if (!get_object_handles_transaction(handleQueue))
		{
			printf("get_object_handles failed\n");
			return false;
		}
	}

	return true;
}

bool PTPUSB::get_object_info(uint32_t object_handle, PTPObjectInfo& objInfo)
{
	PTPContainerWrapper ptp;
	PTPContainer& req = ptp.container;


	memset(&ptp,0,sizeof(ptp));
	ptp.param_number = 1;
	req.Code = PTP_OC_GetObjectInfo;
	req.SessionID = m_session;
	req.Transaction_ID = m_transaction;
	req.Param1 = object_handle;

	if (send_request(ptp))
	{
		if (!get_object_info_transaction(objInfo))
		{
			printf("get_object_handles failed\n");
			return false;
		}

	}
	return true;
}

bool PTPUSB::send_request(const PTPContainerWrapper& ptp)
{
	int ret;
	PTPUSBBulkContainer usbreq;
	const PTPContainer& req = ptp.container;
	int written = 0;
	int towrite;
	/* build appropriate USB container */
	usbreq.length=htod32(PTP_USB_BULK_REQ_LEN- (sizeof(uint32_t)*(5-ptp.param_number)));
	usbreq.type=htod16(PTP_USB_CONTAINER_COMMAND);
	usbreq.code=htod16(req.Code);
	usbreq.trans_id=htod32(req.Transaction_ID);
	usbreq.payload.params.param1=htod32(req.Param1);
	usbreq.payload.params.param2=htod32(req.Param2);
	usbreq.payload.params.param3=htod32(req.Param3);
	usbreq.payload.params.param4=htod32(req.Param4);
	usbreq.payload.params.param5=htod32(req.Param5);
	/* send it to responder */
	towrite = usbreq.get_container_header_size() + ptp.get_container_size();

	ret=libusb_bulk_transfer(m_dev_handle, m_out_ep, (unsigned char*)&usbreq,
		towrite, &written, LIBUSB_WRITE_TIMEOUT);

	if (ret != 0 || written != towrite)
	{
		return false;
	}
	return true;
}

bool PTPUSB::get_response(PTPContainer& resp)
{
	int try_counter = 0;
	int ret;
	PTPUSBBulkContainer usbresp = {0};
	int actual_size = 0;
	bool bResult = false;
	while (try_counter < 3 && !bResult)
	{
		/* read response, it should never be longer than sizeof(usbresp) */
		ret=libusb_bulk_transfer(m_dev_handle, m_in_ep, (unsigned char *)&usbresp,
				PTP_USB_BULK_HS_MAX_PACKET_LEN_READ, &actual_size, LIBUSB_READ_TIMEOUT);

		try_counter++;
		if (ret != 0 )
		{
			continue;
		}
		if (dtoh16(usbresp.type)!=PTP_USB_CONTAINER_RESPONSE)
		{
			continue;
		}

		bResult = true;
	}

	if (bResult)
	{
		resp.Code=dtoh16(usbresp.code);
		resp.Transaction_ID=dtoh32(usbresp.trans_id);
		resp.Param1=dtoh32(usbresp.payload.params.param1);
		resp.Param2=dtoh32(usbresp.payload.params.param2);
		resp.Param3=dtoh32(usbresp.payload.params.param3);
		resp.Param4=dtoh32(usbresp.payload.params.param4);
		resp.Param5=dtoh32(usbresp.payload.params.param5);
		m_transaction = resp.Transaction_ID + 1;
	}
	return bResult;
}

bool PTPUSB::get_data(PTPUSBBulkContainer& resp)
{
	int ret;

	int actual_size = 0;

	/* read response, it should never be longer than sizeof(usbresp) */
	ret=libusb_bulk_transfer(m_dev_handle, m_in_ep, (unsigned char *)&resp,
			PTP_USB_BULK_HS_MAX_PACKET_LEN_READ, &actual_size, LIBUSB_READ_TIMEOUT);

	if (ret != 0 )
	{
		return false;
	}
	if (dtoh16(resp.type)!=PTP_USB_CONTAINER_DATA)
	{
		return false;
	}
	if (actual_size != dtoh32(resp.length))
	{
		return false;
	}

	return true;
}

bool PTPUSB::get_raw_data(uint8_t* buffer, unsigned int size)
{
	//unsigned char buffer[PTP_USB_BULK_HS_MAX_PACKET_LEN_READ];
	int ret;
	int actual_size = 0;
	uint8_t* ptr = buffer;
	unsigned int size_in_byte = size;
	while (size_in_byte > 0)
	{
		ret=libusb_bulk_transfer(m_dev_handle, m_in_ep, ptr,
				PTP_USB_BULK_HS_MAX_PACKET_LEN_READ, &actual_size, LIBUSB_READ_TIMEOUT);
		if (ret != 0)
		{
			return false;
		}

		if (actual_size > size_in_byte)
		{
			printf("actual size bigger than expected\n");
			return false;
		}

		size_in_byte -= actual_size;
		ptr += actual_size;
	}

	return true;
}

bool PTPUSB::get_object_helper(FILE*stream, uint32_t size_in_byte)
{
	unsigned char buffer[PTP_USB_BULK_HS_MAX_PACKET_LEN_READ];
	int ret;
	int actual_size = 0;
	while (size_in_byte > 0)
	{
		ret=libusb_bulk_transfer(m_dev_handle, m_in_ep, buffer,
				PTP_USB_BULK_HS_MAX_PACKET_LEN_READ, &actual_size, LIBUSB_READ_TIMEOUT);
		if (ret != 0)
		{
			return false;
		}

		if (actual_size > size_in_byte)
		{
			printf("actual size bigger than expected\n");
			return false;
		}
		if (fwrite(buffer, 1, actual_size, stream) != actual_size)
		{
			printf("write to stream error\n");
			return false;
		}
		size_in_byte -= actual_size;
	}

	return true;
}

bool PTPUSB::get_object_handle_helper(HandleQueue& handleQueue, uint32_t size_in_byte)
{
	unsigned char buffer[PTP_USB_BULK_HS_MAX_PACKET_LEN_READ];
	int ret;
	int actual_size = 0;
	while (size_in_byte > 0)
	{
		ret=libusb_bulk_transfer(m_dev_handle, m_in_ep, buffer,
				PTP_USB_BULK_HS_MAX_PACKET_LEN_READ, &actual_size, LIBUSB_READ_TIMEOUT);
		if (ret != 0)
		{
			return false;
		}
		if (actual_size % sizeof(uint32_t) != 0)
		{
			printf("actual_size not multiple of int size\n");
			return false;
		}
		if (actual_size > size_in_byte)
		{
			printf("actual size bigger than expected\n");
			return false;
		}
		int element_size = actual_size / sizeof(uint32_t);
		uint32_t* data_ptr = (uint32_t*) buffer;
		for (uint32_t i = 0; i < element_size; i ++)
		{
			handleQueue.push_back(dtoh32(data_ptr[i]));
		}
		size_in_byte -= actual_size;
	}

	return true;
}

#define PTP_USB_BULK_PAYLOAD_MAX_HANDLE_COUNT  (PTP_USB_BULK_PAYLOAD_LEN_READ/sizeof(uint32_t))
bool PTPUSB::get_object_handles_transaction(HandleQueue& handleQueue)
{
	int ret;

	int actual_size = 0;
	PTPUSBBulkContainer resp;

	do
	{
		ret=libusb_bulk_transfer(m_dev_handle, m_in_ep, (unsigned char *)&resp,
				PTP_USB_BULK_HS_MAX_PACKET_LEN_READ, &actual_size, LIBUSB_READ_TIMEOUT);

		if (ret != 0 )
		{
			return false;
		}
		uint16_t resp_code = dtoh16(resp.code);
		uint16_t resp_type = dtoh16(resp.type);
		uint32_t resp_trans_id = dtoh32(resp.trans_id);
		uint32_t resp_length = dtoh32(resp.length);

		switch (resp_type)
		{
		case PTP_USB_CONTAINER_DATA:
			if(resp_code == PTP_OC_GetObjectHandles)
			{
				uint32_t* array = (uint32_t*) resp.payload.data;

				uint32_t size_in_bytes = resp_length - actual_size;
				uint32_t current_length;
				current_length = (actual_size - PTP_USB_BULK_HDR_LEN - 1) / sizeof(uint32_t);

				for (uint32_t i = 1; i < current_length; i ++)
				{
					handleQueue.push_back(dtoh32(array[i]));
				}
				if (size_in_bytes > 0)
				{
					if (!get_object_handle_helper(handleQueue, size_in_bytes))
					{
						return false;
					}
				}
			}
			break;
		case PTP_USB_CONTAINER_RESPONSE:
			if (resp_code == PTP_RC_OK)
			{
				m_transaction = resp_trans_id + 1;
				return true;
			}
			break;
		default:
			return false;
		}
	}
	while (true);

	return false;
}


typedef deque<string> StringQueue;

bool PTPUSB::prepare_file_write(const PTPObjectInfoWrapper&objInfo, char*file_name)
{
	//let's just fake here
	StringQueue strQueue;

	uint32_t parentHandle = objInfo.info.head.ParentObject;
	while (parentHandle != PTP_GOH_ROOT_OBJECT_PARENT)
	{
		PTPObjectInfoWrapper search_obj;
		search_obj.Handle = parentHandle;
		ObjectInfoSet::const_iterator it = m_objInfoSet.find(search_obj);
		if (it == m_objInfoSet.end())
		{
			printf("cannot find parent obj handle\n");
			return false;
		}
		strQueue.push_front(string(it->info.Filename));
		parentHandle = it->info.head.ParentObject;

	}
	string cur_dir = m_target_dir;
	while (!strQueue.empty())
	{
		string cur_name = strQueue.front();
		strQueue.pop_front();
		cur_dir.append("/");
		cur_dir.append(cur_name);
		if (mkdir(cur_dir.c_str(), S_IRWXU | S_IRWXG| S_IRWXO) == -1)
		{
			if (errno != EEXIST)
			{
				printf("error create dir %s", cur_name.c_str());
				return false;
			}
		}
	}
	cur_dir.append("/");
	cur_dir.append(objInfo.info.Filename);
	strncpy(file_name, cur_dir.c_str(), MAX_PATH);
	return true;
}

bool PTPUSB::get_object_transaction(const PTPObjectInfoWrapper& objInfo, const char* file_name)
{
	DEBUG_ENTRY
	int ret;

	int actual_size = 0;
	PTPUSBBulkContainer resp;

	do
	{
		ret=libusb_bulk_transfer(m_dev_handle, m_in_ep, (unsigned char *)&resp,
				PTP_USB_BULK_HS_MAX_PACKET_LEN_READ, &actual_size, LIBUSB_READ_TIMEOUT);

		if (ret != 0 )
		{
			return false;
		}
		uint16_t resp_code = dtoh16(resp.code);
		uint16_t resp_type = dtoh16(resp.type);
		uint32_t resp_trans_id = dtoh32(resp.trans_id);
		uint32_t resp_length = dtoh32(resp.length);

		switch (resp_type)
		{
		case PTP_USB_CONTAINER_DATA:
			if(resp_code == PTP_OC_GetObject)
			{
				int size_in_bytes;
				uint32_t current_length;


				FILE* stream = fopen(file_name, "w");

				if (!stream)
				{
					printf("failed to open file %s\n", file_name);
					return false;
				}
				if (actual_size > PTP_USB_BULK_HDR_LEN)
				{
					current_length = actual_size - PTP_USB_BULK_HDR_LEN;
				}
				else
				{
					printf("missing data\n");
					fclose(stream);
					return false;
				}

				if (fwrite(resp.payload.data, 1, current_length, stream) != current_length)
				{
					printf("write file %s failed", objInfo.info.Filename.c_str());
					fclose(stream);
					return false;
				}
				size_in_bytes = resp_length - PTP_USB_BULK_HDR_LEN - current_length;
				if (size_in_bytes > 0)
				{
					if (!get_object_helper(stream, size_in_bytes))
					{
						fclose(stream);
						return false;
					}
				}
				fclose(stream);
			}
			break;
		case PTP_USB_CONTAINER_RESPONSE:
			if (resp_code == PTP_RC_OK)
			{
				m_transaction = resp_trans_id + 1;
				return true;
			}
			break;
		default:
			return false;
		}
	}
	while (true);

	return false;
}

bool PTPUSB::get_object_info_transaction(PTPObjectInfo& objInfo)
{
	int ret;

	int actual_size = 0;
	PTPUSBBulkContainer resp;
	do
	{
		ret=libusb_bulk_transfer(m_dev_handle, m_in_ep, (unsigned char *)&resp,
				PTP_USB_BULK_HS_MAX_PACKET_LEN_READ, &actual_size, LIBUSB_READ_TIMEOUT);

		if (ret != 0 )
		{
			return false;
		}
		uint16_t resp_code = dtoh16(resp.code);
		uint16_t resp_type = dtoh16(resp.type);
		uint32_t resp_trans_id = dtoh32(resp.trans_id);
		uint32_t resp_length = dtoh32(resp.length);

		switch (resp_type)
		{
		case PTP_USB_CONTAINER_DATA:
			if(resp_code == PTP_OC_GetObjectInfo)
			{
				PTPObjectInfoHead* infoPtr = (PTPObjectInfoHead*) resp.payload.data;
				PTPObjectInfoHead& objInfoHead = objInfo.head;
				objInfoHead.StorageID = dtoh32(infoPtr->StorageID);
				objInfoHead.ObjectFormat = dtoh16(infoPtr->ObjectFormat);
				objInfoHead.ProtectionStatus = dtoh16(infoPtr->ProtectionStatus);
				objInfoHead.ObjectCompressedSize = dtoh32(infoPtr->ObjectCompressedSize);
				objInfoHead.ThumbFormat = dtoh16(infoPtr->ThumbFormat);
				objInfoHead.ThumbCompressedSize = dtoh32(infoPtr->ThumbFormat);

				objInfoHead.ThumbPixWidth = dtoh32(infoPtr->ThumbPixWidth);
				objInfoHead.ImagePixHeight = dtoh32(infoPtr->ImagePixHeight);
				objInfoHead.ImagePixWidth = dtoh32(infoPtr->ImagePixWidth);

				objInfoHead.ImagePixHeight = dtoh32(infoPtr->ImagePixHeight);
				objInfoHead.ImageBitDepth = dtoh32(infoPtr->ImageBitDepth);
				objInfoHead.ParentObject = dtoh32(infoPtr->ParentObject);

				objInfoHead.AssociationType = dtoh16(infoPtr->AssociationType);
				objInfoHead.AssociationDesc = dtoh32(infoPtr->AssociationDesc);
				objInfoHead.SequenceNumber = dtoh32(infoPtr->SequenceNumber);


#ifndef NICK_TEST

				if (resp.length < actual_size)
				{
					printf("serious error!!!\n");
					return false;
				}
				uint8_t size_to_read = resp.length - actual_size;
				int data_offset = resp.get_container_header_size() + sizeof(PTPObjectInfoHead);
				uint8_t size_data_read = actual_size - data_offset;
				uint8_t size_of_data = resp.length - data_offset;

				uint8_t* ptr = resp.payload.data + sizeof(PTPObjectInfoHead);

				if (size_of_data + sizeof(PTPObjectInfoHead) > PTP_USB_BULK_HS_MAX_PACKET_LEN_READ_DOUBLE)
				{
					printf("!!!!oopse, such big data size!!!\n");
					return false;
				}
				if (size_to_read > 0)
				{
					if (!get_raw_data(ptr, size_to_read))
					{
						printf("get raw data failed!!!\n");
						return false;
					}
				}
				uint8_t len = 0;
				uint16_t offset = 0;
				objInfo.Filename = ptp_unpack_string(ptr, offset, len);


#else
				uint32_t size_to_read = resp.length - actual_size;
				while (size_to_read > 0)
				{
					uint8_t tmp_buf[512];
					uint32_t current_to_read = size_to_read;
					if (size_to_read > 512)
					{
						current_to_read = 512;
					}
					if (!get_raw_data(tmp_buf, current_to_read))
					{
						printf("failed to read raw data\n");
						return false;
					}
					size_to_read -= current_to_read;
				}
#endif
				/*
				offset += len*2+1;
				objInfo.CaptureDate = strdup((char*)(resp.payload.data + offset));
				offset += strlen(objInfo.CaptureDate);
				objInfo.ModificationDate = strdup((char*)(resp.payload.data + offset));
				offset += strlen(objInfo.ModificationDate);
				objInfo.Keywords = ptp_unpack_string(resp.payload.data, offset, len);
				*/

			}
			break;
		case PTP_USB_CONTAINER_RESPONSE:
			if (resp_code == PTP_RC_OK)
			{
				m_transaction = resp_trans_id + 1;
				return true;
			}
			break;
		default:
			return false;
		}

	}
	while (true);

	return false;
}


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

void PTPUSB::debug_msg(const char* format, ...)
{
	va_list vl;
	va_start(vl, format);
	vfprintf(m_log, format, vl);
	va_end(vl);
}

PTPUSB::PTPUSB()
{
	m_event_thread_running = false;
	m_ctx = NULL;
	m_dev = NULL;
	m_dev_handle = NULL;
	memset(&m_desc, 0, sizeof(m_desc));
	m_interface = 0;
	m_configuration = 0;
	m_in_ep = 0;
	m_out_ep = 0;
	m_int_ep = 0;
	m_transaction = 0;
	m_session_opened = false;
	m_target_dir = "/tmp";
	m_stop_event_thread = false;
	m_int_packet_size = 0;
	m_log = fopen("ptpusb.log", "w");

}

PTPUSB::~PTPUSB()
{
	//send_event(PTP_EC_DeviceReset);
	if (m_event_thread_running)
	{
		void* ptr = NULL;
		pthread_join(m_event_thread_ctx, &ptr);
	}
	reset_device_by_request();
	close_session(true);
	reset_device_by_control();

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

	fclose(m_log);
}

//PTP_EC_DeviceReset
void PTPUSB::send_event(uint16_t code)
{
	PTPUSBEventContainer event;
	memset(&event, 0, sizeof(event));
	event.code = code;
	event.type = PTP_USB_CONTAINER_EVENT;
	event.length = 12;
	event.trans_id = m_transaction;
	m_outgoing_event_queue.push_back(event);

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
							m_in_packet_size = ep[l].wMaxPacketSize;
							found_inep = 1;
						}
						if ((ep[l].bEndpointAddress & LIBUSB_ENDPOINT_DIR_MASK)
								== 0)
						{
							m_out_ep = ep[l].bEndpointAddress;
							m_out_packet_size = ep[l].wMaxPacketSize;

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
							m_int_packet_size = ep[l].wMaxPacketSize;
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
{/*
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
	*/
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
				if (m_desc.idVendor == 0x04e8 && (m_desc.idProduct == 0x6865
						|| m_desc.idProduct == 0x6860)) //
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
	if (_result)
	{
		start_event_thread();
	}
	return _result;
}

#define PTP_MAXSTRLEN 255

char* PTPUSB::ptp_unpack_string(unsigned char* data, uint16_t offset, uint8_t& len)
{

	//uint16_t string[PTP_MAXSTRLEN+1];
	/* allow for UTF-8: max of 3 bytes per UCS-2 char, plus final null */
	char loclstr[PTP_MAXSTRLEN*3+1];
	size_t nconv, srclen, destlen;
	char *src, *dest;

	len = dtoh8a(&data[offset]) * sizeof(uint16_t);	/* PTP_MAXSTRLEN == 255, 8 bit len */

	if (len == 0)		/* nothing to do? */
		return(NULL);

	/* copy to string[] to ensure correct alignment for iconv(3) */
	//memcpy(string, &data[offset+1], length * sizeof(string[0]));
	//string[length] = 0x0000U;   /* be paranoid!  add a terminator. */
	loclstr[0] = '\0';

	/* convert from camera UCS-2 to our locale */
	//src = (char *)string;
	src = (char *)(data + offset + 1);
	srclen = len;
	dest = loclstr;
	destlen = sizeof(loclstr)-1;
	nconv = (size_t)-1;
#ifdef HAVE_ICONV
	//if (params->cd_ucs2_to_locale != (iconv_t)-1)
	static iconv_t cd = (iconv_t)(-1);
	//const char * EUCSET = "EUC-JP";
	const char * INSET = "UCS-2LE" ;
	const char * OUTSET = "UTF-8";  //"WCHAR_T"iconv_open("UTF-8", "UCS-2LE");
	if (cd == (iconv_t)(-1))
	{
		cd = iconv_open (OUTSET, INSET);

	}
	if (cd != (iconv_t)(-1))
	{
		nconv = iconv(cd, &src, &srclen, &dest, &destlen);
	}
#endif
	if (nconv == (size_t) -1)
	{ /* do it the hard way */
		printf("******************************failed*******************************\n");
	}
	else
	{
		printf("success %s\n", dest);
		*dest = '\0';
	}
	loclstr[sizeof(loclstr)-1] = '\0';   /* be safe? */
	return loclstr;
}

/*
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
*/

void test2()
{
	FILE* stream = fopen("myfile.txt", "w");

	if (!stream)
	{
		printf("cannot open file\n");
		return;
	}

	PTPUSB ptpusb;
	if (ptpusb.open())
	{
		if (ptpusb.display())
		{
			int counter = 3;
			while (true)
			{
				if (!ptpusb.open_session())
				{
					ptpusb.init();
				}
				else
				{
					break;
				}
				counter --;
				if (counter == 0)
				{
					return;
				}
			}

			if (ptpusb.get_storage_id())
			{
				//for (uint32_t index = 0; index< ptpusb.m_storage_count; index ++)
				for (uint32_t index = 0; index < 1; index ++)
				{
					if (ptpusb.get_all_objects(ptpusb.m_storage_array[index].StorageId))
					{
						printf("objInfoSet size=%lu\n", ptpusb.m_objInfoSet.size());
						for (ObjectInfoSet::const_iterator it = ptpusb.m_objInfoSet.begin(); it != ptpusb.m_objInfoSet.end(); it++)
						{
							fprintf(stream, "filename:%s\n", it->info.Filename.c_str());
						}
					}
				}
			}
			if (ptpusb.close_session())
			{
				cout << "open successfully"<<endl;
			}

		}
	}
	fclose(stream);
}

void test1()
{
	PTPUSB ptpusb;
	if (ptpusb.open())
	{
		if (ptpusb.display())
		{
			if (ptpusb.open_session())
			{
				if (ptpusb.get_storage_id())
				{
					if (ptpusb.get_storage_info())
					{
						if (ptpusb.get_storage_object_number())
						{
							for (uint32_t index = 0; index< ptpusb.m_storage_count; index ++)
							{
								uint32_t storage_id = ptpusb.m_storage_array[index].StorageId;
								uint32_t parent_handle = 0x00000000;
								HandleQueue handle_queue;
								if (ptpusb.get_object_handles(storage_id, parent_handle, handle_queue))
								{
									for (unsigned int i =0; i < handle_queue.size(); i ++)
									{
										uint32_t handle = handle_queue[i];
										PTPObjectInfo objInfo;
										if (ptpusb.get_object_info(handle, objInfo))
										{
											if (!objInfo.Filename.empty())
											{
												printf("filename=%s\n", objInfo.Filename.c_str());
											}
											//objInfo.print();
										}

									}

								}
							}
						}
					}
				}
				if (ptpusb.close_session())
				{
					cout << "open successfully"<<endl;
				}
			}
		}
	}
}

void test3()
{
	PTPUSB ptpusb;
	if (ptpusb.open())
	{
		if (ptpusb.display())
		{
			if (ptpusb.init())
			{
				if (ptpusb.open_session())
				{
					if (ptpusb.get_storage_id())
					{
						if (ptpusb.get_storage_info())
						{
							printf("ok\n");
						}
					}
					if (ptpusb.close_session())
					{
						cout << "open successfully"<<endl;
					}
				}
			}
		}
	}
}

void test4()
{
	PTPUSB ptpusb;

	if (ptpusb.open())
	{
		if (ptpusb.display())
		{
			if (ptpusb.open_session())
			{
				if (ptpusb.get_storage_id())
				{
					if (ptpusb.get_storage_info())
					{
						printf("ok\n");
						int counter = 0;
						while (counter < 1000)
						{
							sleep(1);
						}
					}
				}
				if (ptpusb.close_session())
				{
					cout << "open successfully"<<endl;
				}
			}
		}
	}
}

void test5()
{
	PTPUSB ptpusb;
	if (ptpusb.open())
	{
		if (ptpusb.display())
		{
			if (ptpusb.open_session())
			{
				if (ptpusb.close_session())
				{
					cout << "open successfully"<<endl;
				}
				ptpusb.start_event_thread();
				if (!ptpusb.reset_device())
				{
					return;
				}
				if (ptpusb.open_session())
				{
					if (ptpusb.get_storage_id())
					{
						if (ptpusb.get_storage_info())
						{
							printf("ok\n");
						}
					}
				}
			}
		}
	}
}

void test6()
{
	PTPUSB ptpusb;
	if (ptpusb.open())
	{
		if (ptpusb.display())
		{
			if (ptpusb.reset_device())
			{
				printf("success\n");
			}
		}
	}
}


void test7()
{
	PTPUSB ptpusb;
	if (ptpusb.open())
	{
		if (ptpusb.display())
		{
			do
			{
				uint16_t status;
				if (!ptpusb.get_device_status(status))
				{
					break;
				}
				if (status != PTP_RC_OK)
				{
					if (!ptpusb.reset_device())
					{
						break;
					}
				}
				else
				{
					printf("ok\n");
					break;
				}
			}
			while (true);
		}
	}
}

int main()
{
	test2();
	return 0;

}
#pragma pack(pop)
