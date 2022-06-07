#pragma once

#include "hls_stream.h"
#include <ap_int.h>
#include "ap_utils.h"



# define PACKET_DATA_LENGTH 512
# define PACKET_KEEP_LENGTH 64
# define PACKET_LAST
# define PACKET_USER_LENGTH 16
# define PACKET_DEST_LENGTH 8




struct dataword
{
	ap_uint<PACKET_DATA_LENGTH> data;
	ap_uint<PACKET_DEST_LENGTH> id;
	ap_uint<PACKET_DEST_LENGTH> dest;
	ap_uint<PACKET_USER_LENGTH> user;
	ap_uint<1> last;
};


