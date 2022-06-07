
#pragma once

#include "parameters.hpp"
#include "common.hpp"



void kernel(
		hls::stream<dataword>& in,
		hls::stream<dataword>& out
		);
