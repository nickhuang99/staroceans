/*
 * ptpusb.h
 *
 *  Created on: Apr 20, 2013
 *      Author: nick
 */

#ifndef PTPUSB_H_
#define PTPUSB_H_

#include <libusb-1.0/libusb.h>
#include <deque>
#include <set>
#include <map>
#include <vector>
#include <string>
#include <pthread.h>

using namespace std;

#define PTP_USB_BULK_HS_MAX_PACKET_LEN_WRITE	512
#define PTP_USB_BULK_HS_MAX_PACKET_LEN_READ   512
#define PTP_USB_BULK_HDR_LEN		(2*sizeof(uint32_t)+2*sizeof(uint16_t))
#define PTP_USB_BULK_PAYLOAD_LEN_WRITE	(PTP_USB_BULK_HS_MAX_PACKET_LEN_WRITE-PTP_USB_BULK_HDR_LEN)
#define PTP_USB_BULK_PAYLOAD_LEN_READ	(PTP_USB_BULK_HS_MAX_PACKET_LEN_READ-PTP_USB_BULK_HDR_LEN)
#define PTP_USB_BULK_REQ_LEN	(PTP_USB_BULK_HDR_LEN+5*sizeof(uint32_t))
#define PTP_USB_BULK_HS_MAX_PACKET_LEN_READ_DOUBLE (2*PTP_USB_BULK_HS_MAX_PACKET_LEN_READ)
#define PTP_USB_BULK_PAYLOAD_RAW_DATA_LEN  (PTP_USB_BULK_HS_MAX_PACKET_LEN_READ_DOUBLE)

#pragma pack(push)
#pragma pack(2)
/* Transaction data phase description */
enum TransactionTypeEnum
{
	TransactionNoData,
	TransactionSendData,
	TransactionRecvData,
};

struct PTPStorageInfoHeader;

struct PTPContainer
{
	uint16_t Code;
	uint32_t SessionID;
	uint32_t Transaction_ID;
	/* params  may be of any type of size less or equal to uint32_t */
	uint32_t Param1;
	uint32_t Param2;
	uint32_t Param3;
	/* events can only have three parameters */
	uint32_t Param4;
	uint32_t Param5;
};

struct PTPContainerWrapper
{
	PTPContainer container;
	uint16_t param_number;
	uint16_t get_container_size()const
	{
		return param_number*sizeof(uint32_t);
	}
};

struct PTPUSBRequestContainer
{
	uint16_t length;
	uint16_t code;
	union
	{
		struct
		{
			uint32_t param1;
			uint32_t param2;
			uint32_t param3;
			uint32_t param4;
			uint32_t param5;
		}params;
		unsigned char data[20];
	}payload;
};

struct PTPUSBEventContainer
{
	uint32_t length;
	uint16_t type;
	uint16_t code;
	uint32_t trans_id;
	uint32_t param1;
	uint32_t param2;
	uint32_t param3;
};


struct PTPObjectInfoHead;

struct PTPUSBBulkContainer
{
	uint32_t length;
	uint16_t type;
	uint16_t code;
	uint32_t trans_id;
	union
	{
		struct
		{
			uint32_t param1;
			uint32_t param2;
			uint32_t param3;
			uint32_t param4;
			uint32_t param5;
		} params;
       /* this must be set to the maximum of PTP_USB_BULK_PAYLOAD_LEN_WRITE
        * and PTP_USB_BULK_PAYLOAD_LEN_READ */
		unsigned char data[PTP_USB_BULK_PAYLOAD_RAW_DATA_LEN];
		//unsigned char data[PTP_USB_BULK_PAYLOAD_LEN_READ];
	} payload;
	uint16_t get_container_header_size() const
	{
		return sizeof(uint32_t)*2+ sizeof(uint16_t)*2;
	}
	uint16_t get_container_body_size() const
	{
		return length - get_container_header_size();
	}
};

struct PTPStorageInfoHeader
{
	uint16_t StorageType;
	uint16_t FilesystemType;
	uint16_t AccessCapability;
	uint64_t MaxCapability;
	uint64_t FreeSpaceInBytes;
	uint32_t FreeSpaceInImages;
};

struct PTPStorageInfo
{
	PTPStorageInfoHeader header;
	string StorageDescription;
	string VolumeLabel;
	PTPStorageInfo()
	{

	}
	~PTPStorageInfo()
	{

	}
};

struct PTPObjectInfoHead
{
	uint32_t StorageID;
	uint16_t ObjectFormat;
	uint16_t ProtectionStatus;
	uint32_t ObjectCompressedSize;
	uint16_t ThumbFormat;
	uint32_t ThumbCompressedSize;
	uint32_t ThumbPixWidth;
	uint32_t ThumbPixHeight;
	uint32_t ImagePixWidth;
	uint32_t ImagePixHeight;
	uint32_t ImageBitDepth;
	uint32_t ParentObject;
	uint16_t AssociationType;
	uint32_t AssociationDesc;
	uint32_t SequenceNumber;
	void print()const
	{
		printf("StorageID:%4u,ObjectFormat:%2u, ProtectionStatus:%2u, ObjectCompressedSize:%4u,"
				"ThumbFormat:%2u,ThumbCompressedSize:%4u,ThumbPixWidth:%4u,ThumbPixHeight:%4u,"
				"ImageBitDepth:%4u,ParentObject:%4u,"
				"AssociationType:%2u,AssociationDesc:%4u,AssociationDesc:%4u,SequenceNumber:%4u\n",
				StorageID, ObjectFormat,ProtectionStatus,ObjectCompressedSize,ThumbFormat,
				ThumbCompressedSize,ThumbPixWidth,ThumbPixHeight,ImageBitDepth,ParentObject,
				AssociationType,AssociationDesc,AssociationDesc,SequenceNumber
				);
	}

};

struct PTPObjectInfo
{
	PTPObjectInfoHead head;
	/*
	char*   Filename;
	char*	CaptureDate;
	char*	ModificationDate;
	char*   Keywords;
	*/
	string   Filename;
	string	 CaptureDate;
	string	 ModificationDate;
	string   Keywords;
	PTPObjectInfo()
	{

	}
	~PTPObjectInfo()
	{

	}

	void print() const
	{
		head.print();

		//printf("Filename:%s,CaptureDate:%s,ModificationDate:%s,Keywords:%s\n",
		//		Filename, CaptureDate, ModificationDate, Keywords);
	}
};

struct PTPObjectInfoWrapper
{
	uint32_t Handle;
	PTPObjectInfo info;
};

struct PTPObjectInfoComp
{
	bool operator()(const PTPObjectInfoWrapper& left, const PTPObjectInfoWrapper& right)const
	{
		return left.Handle < right.Handle;
	}
};

typedef deque<uint32_t> HandleQueue;
typedef vector<uint32_t> HandleVector;

typedef multiset<PTPObjectInfoWrapper, PTPObjectInfoComp> ObjectInfoSet;
typedef vector<PTPObjectInfoWrapper> ObjectInfoVector;

typedef multimap<string, uint32_t> NameHandleMap;

struct PTPStorageDescriptor
{
	uint32_t StorageId;
	PTPStorageInfo info;
	uint32_t ObjectNumber;
	ObjectInfoSet Handles;
};


typedef deque<PTPUSBEventContainer> EventQueue;
typedef basic_string<unsigned char> ByteBuffer;

#define MAX_STORAGE_COUNT 16

class PTPUSB
{
public:
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
	uint16_t                        m_in_packet_size;
	uint16_t                        m_out_packet_size;
	uint16_t                        m_int_packet_size;
	int                             m_err;
	uint32_t                        m_session;
	uint32_t						m_transaction;

	bool							m_session_opened;
	int 							m_storage_count;
	PTPStorageDescriptor            m_storage_array[MAX_STORAGE_COUNT];
	ObjectInfoSet                   m_objInfoSet;
	string                          m_target_dir;
	EventQueue                      m_incoming_event_queue;
	EventQueue                      m_outgoing_event_queue;
	pthread_t                       m_event_thread_ctx;
	bool 							m_event_thread_running;
	bool 							m_stop_event_thread;
	FILE*                           m_log;
	NameHandleMap                   m_name_handle_map;
	//static ObjectInfoVector         m_object_info_vector;

	bool send_all_events();

	bool recv_all_events();

	int unpack_string(const char* str, char* dest);
	bool find_interface_and_endpoints();
	bool check_and_configure_usb();
	void uninit();
	void printdev(libusb_device *dev);
	const char* get_usb_error_string();
	void print_usb_error();
	void print_ptp_error(uint16_t err);

	bool get_response(PTPContainer& ptp);
	bool send_request(const PTPContainerWrapper& ptp);
	bool get_data(PTPUSBBulkContainer& ptp);
	char* ptp_unpack_string(unsigned char* data, uint16_t offset, uint8_t& len);
	bool get_object_handles_transaction(HandleQueue& handleQueue);
	bool get_object_info_transaction(PTPObjectInfo& objInfo);
	bool get_object_handle_helper(HandleQueue& handleQueue, uint32_t size_in_byte);
	bool get_object_transaction(const PTPObjectInfoWrapper& objInfo, const char* file_name);
	bool get_object_helper(FILE*stream, uint32_t size_in_byte);
	bool prepare_file_write(const PTPObjectInfoWrapper&objInfo, char*file_name);
	void debug_msg(const char* format, ...);
	void send_event(uint16_t event_code);
	bool get_raw_data(uint8_t* buffer, unsigned int size);
public:
	PTPUSB();
	~PTPUSB();

	bool open();
	bool init();
	bool display();
	bool get_storage_id();
	bool get_storage_info();
	bool get_storage_object_number();
	bool get_object_handles(uint32_t storage_id, uint32_t parent_handle, HandleQueue& handleQueue);

	bool get_all_objects(uint32_t storage_id, bool to_download_object=false);
	bool get_object_info(uint32_t object_handle, PTPObjectInfo& objInfo);
	//bool put_object_info(uint32_t storage_id, uint32_t parent_handle, const PTPObjectInfo& objInfo)

	bool open_session();
	bool close_session(bool by_force = false);
	bool reset_device();
	bool self_test();
	bool reset_device_by_request();
	bool reset_device_by_control();
	bool get_device_status(uint16_t& status);
	bool get_object(const PTPObjectInfoWrapper& objInfo, const char* file_name);
	bool start_event_thread();
	bool search_object_info_by_handle(uint32_t handle, PTPObjectInfo& objInfo);
	bool search_object_info_by_name(const string& fileName, ObjectInfoVector& objInfoVect);
};

#pragma pack(pop)
#endif /* PTPUSB_H_ */
