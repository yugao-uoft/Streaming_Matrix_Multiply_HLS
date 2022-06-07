#pragma once

struct Config_MatMul
{
	static const unsigned id=0;
	static const unsigned dest=1;
	static const unsigned M=256;
	static const unsigned K=256;
	static const unsigned BUS_WIDTH=512;
	static const unsigned DATA_WIDTH=8;
	static const unsigned OUT_DATA_WIDTH=32;
	static const unsigned ITER_PE=2;
	static const unsigned NUM_TILE=4;
	static const unsigned SCALE_FACTOR=8;
	static const unsigned VEC_WIDTH=BUS_WIDTH/DATA_WIDTH;
	static const unsigned OUT_VEC_WIDTH=BUS_WIDTH/OUT_DATA_WIDTH;
	static const unsigned NUM_PE=K/(ITER_PE * VEC_WIDTH);
};

#define DTYPE_IN  ap_int<Config_MatMul::DATA_WIDTH>
#define DTYPE_OUT ap_int<Config_MatMul::OUT_DATA_WIDTH>
