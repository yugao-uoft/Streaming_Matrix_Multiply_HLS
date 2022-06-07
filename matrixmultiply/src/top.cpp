
#include "weights.hpp"
#include "common.hpp"
#include "parameters.hpp"
#include "matmul.hpp"




void kernel(
		hls::stream<dataword>& in,
		hls::stream<dataword>& out
		)
{
#pragma HLS interface ap_ctrl_none port=return
#pragma HLS INTERFACE axis port=in
#pragma HLS INTERFACE axis port=out

	MatMul<Config_MatMul>(weights1, in, out);
}
