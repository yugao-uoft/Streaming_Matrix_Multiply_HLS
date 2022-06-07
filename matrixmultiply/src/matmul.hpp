#pragma once

#include "weights.hpp"
#include "common.hpp"
#include "parameters.hpp"

template<typename config_t>
void PE(
		hls::stream<ap_uint<32> >& N_pipe_in,
		hls::stream<ap_uint<32> >& N_pipe_out,
		hls::stream<ap_uint<config_t::BUS_WIDTH / 2> > &a_in_1,
		hls::stream<ap_uint<config_t::BUS_WIDTH / 2> > &a_in_2,
		hls::stream<ap_uint<config_t::BUS_WIDTH / 2> > &a_in_3,
		hls::stream<ap_uint<config_t::BUS_WIDTH / 2> > &a_in_4,
		hls::stream<ap_uint<config_t::BUS_WIDTH / 2> > &b_in_1,
		hls::stream<ap_uint<config_t::BUS_WIDTH / 2> > &b_in_2,
		hls::stream<ap_uint<config_t::BUS_WIDTH / 2> > &b_out_1,
		hls::stream<ap_uint<config_t::BUS_WIDTH / 2> > &b_out_2,
		hls::stream<ap_uint<config_t::OUT_DATA_WIDTH> > &c_in_1,
		hls::stream<ap_uint<config_t::OUT_DATA_WIDTH> > &c_in_2,
		hls::stream<ap_uint<config_t::OUT_DATA_WIDTH> > &c_out_1,
		hls::stream<ap_uint<config_t::OUT_DATA_WIDTH> > &c_out_2,
		int pe_index)
{

	ap_int<24> temp_a1_int8[config_t::VEC_WIDTH];
	ap_int<24> temp_a2_int8[config_t::VEC_WIDTH];
	ap_int<8> temp_b_int8[config_t::VEC_WIDTH][config_t::ITER_PE];
	ap_int<24> temp_a1_a2[config_t::VEC_WIDTH];
	ap_int<32> temp_c_int8[config_t::VEC_WIDTH];
	ap_int<16> temp_c1_int8[config_t::VEC_WIDTH];
	ap_int<16> temp_c2_int8[config_t::VEC_WIDTH];
	ap_uint<config_t::BUS_WIDTH> a_read1;
	ap_uint<config_t::BUS_WIDTH> a_read2;
	ap_uint<config_t::BUS_WIDTH> b_read;
	ap_uint<config_t::BUS_WIDTH> b_read_out;
	DTYPE_IN temp_a1[config_t::VEC_WIDTH];
	DTYPE_IN temp_a2[config_t::VEC_WIDTH];
	DTYPE_IN temp_b[config_t::VEC_WIDTH][config_t::ITER_PE];
	DTYPE_OUT temp_c1[config_t::VEC_WIDTH];
	DTYPE_OUT temp_c2[config_t::VEC_WIDTH];
	ap_uint<config_t::OUT_DATA_WIDTH> c_read1;
	ap_uint<config_t::OUT_DATA_WIDTH> c_read2;
	DTYPE_OUT c_buffer1[config_t::ITER_PE];
	DTYPE_OUT c_buffer2[config_t::ITER_PE];
#pragma HLS ARRAY_PARTITION variable=temp_a1_int8 dim=0 complete
#pragma HLS ARRAY_PARTITION variable=temp_a2_int8 dim=0 complete
#pragma HLS ARRAY_PARTITION variable=temp_b_int8 dim=0 complete
#pragma HLS ARRAY_PARTITION variable=temp_a1_a2 dim=0 complete
#pragma HLS ARRAY_PARTITION variable=temp_c_int8 dim=0 complete
#pragma HLS ARRAY_PARTITION variable=temp_c1_int8 dim=0 complete
#pragma HLS ARRAY_PARTITION variable=temp_c2_int8 dim=0 complete
#pragma HLS ARRAY_PARTITION variable=c_buffer1 dim=0 complete
#pragma HLS ARRAY_PARTITION variable=c_buffer2 dim=0 complete
#pragma HLS ARRAY_PARTITION variable=temp_c1 dim=0 complete
#pragma HLS ARRAY_PARTITION variable=temp_c2 dim=0 complete
#pragma HLS ARRAY_PARTITION variable=temp_a1 dim=0 complete
#pragma HLS ARRAY_PARTITION variable=temp_a2 dim=0 complete
#pragma HLS ARRAY_PARTITION variable=temp_b dim=0 complete

	int i, j;

	unsigned int N = N_pipe_in.read();

	if (pe_index < config_t::NUM_PE - 1) {
		N_pipe_out.write(N);
	}

	Loop_1:
	for (int iter=0; iter <  N; iter++) {
#pragma HLS loop_tripcount min=1 max=4096 avg=32
		for (int iter2=0; iter2 < config_t::M  * config_t::ITER_PE / config_t::NUM_TILE; iter2++) {
			#pragma HLS PIPELINE ii=1

			i = iter2 / config_t::ITER_PE;
			j = iter2 % config_t::ITER_PE;

			a_read1.range(config_t::BUS_WIDTH / 2 - 1, 0) = a_in_1.read();
			a_read1.range(config_t::BUS_WIDTH - 1, config_t::BUS_WIDTH / 2) = a_in_2.read();
			a_read2.range(config_t::BUS_WIDTH / 2 - 1, 0) = a_in_3.read();
			a_read2.range(config_t::BUS_WIDTH - 1, config_t::BUS_WIDTH / 2) = a_in_4.read();


			if (config_t::DATA_WIDTH == 8) {
				for (int w=0; w < config_t::VEC_WIDTH; w++) {
					#pragma HLS UNROLL

					temp_a1_int8[w].range(7,0) = a_read1(config_t::DATA_WIDTH * w + config_t::DATA_WIDTH - 1,config_t::DATA_WIDTH * w);

					if (temp_a1_int8[w].bit(7)) {
						temp_a1_int8[w].range(23,8) = -1;
					}
					else {
						temp_a1_int8[w].range(23,8) = 0;
					}

					temp_a2_int8[w].range(23,16) = a_read2(config_t::DATA_WIDTH * w + config_t::DATA_WIDTH - 1,config_t::DATA_WIDTH * w);
					temp_a2_int8[w].range(15,0) = 0;

				}
			}
			else {
				for (int w=0; w < config_t::VEC_WIDTH; w++) {
					#pragma HLS UNROLL
					temp_a1[w] = a_read1(config_t::DATA_WIDTH * w + config_t::DATA_WIDTH - 1,config_t::DATA_WIDTH * w);
					temp_a2[w] = a_read2(config_t::DATA_WIDTH * w + config_t::DATA_WIDTH - 1,config_t::DATA_WIDTH * w);
				}
			}


			if (i == 0) {
				b_read.range(config_t::BUS_WIDTH / 2 - 1, 0) = b_in_1.read();
				b_read.range(config_t::BUS_WIDTH - 1, config_t::BUS_WIDTH / 2) = b_in_2.read();

				if (config_t::DATA_WIDTH == 8) {
					for (int w=0; w < config_t::VEC_WIDTH; w++) {
						#pragma HLS UNROLL
						temp_b_int8[w][j] = b_read(config_t::DATA_WIDTH * w + config_t::DATA_WIDTH - 1,config_t::DATA_WIDTH * w);
					}
				}
				else {
					for (int w=0; w < config_t::VEC_WIDTH; w++) {
						#pragma HLS UNROLL
						temp_b[w][j] = b_read(config_t::DATA_WIDTH * w + config_t::DATA_WIDTH - 1,config_t::DATA_WIDTH * w);
					}
				}
			}
			else if (i > 0 && i < config_t::NUM_PE - pe_index) {
				b_read_out.range(config_t::BUS_WIDTH / 2 - 1, 0) = b_in_1.read();
				b_read_out.range(config_t::BUS_WIDTH - 1, config_t::BUS_WIDTH / 2) = b_in_2.read();
				b_out_1.write(b_read_out.range(config_t::BUS_WIDTH / 2 - 1, 0));
				b_out_2.write(b_read_out.range(config_t::BUS_WIDTH - 1, config_t::BUS_WIDTH / 2));
			}

			c_buffer1[j] = 0;
			c_buffer2[j] = 0;

			if (config_t::DATA_WIDTH == 8) {
				for (int w=0; w < config_t::VEC_WIDTH; w++) {
					#pragma HLS UNROLL
					temp_c_int8[w] = (temp_a2_int8[w] + temp_a1_int8[w]) * temp_b_int8[w][j];
				}

				for (int w=0; w < config_t::VEC_WIDTH; w++) {
					#pragma HLS UNROLL
					temp_c1_int8[w] = temp_c_int8[w].range(15,0);
					temp_c2_int8[w] = temp_c_int8[w].range(31,16) + temp_c_int8[w].bit(31);
				}

				for (int w=0; w < config_t::VEC_WIDTH; w++) {
					#pragma HLS UNROLL
					c_buffer1[j] += temp_c1_int8[w];
					c_buffer2[j] += temp_c2_int8[w];
				}

			}
			else {
				for (int w=0; w < config_t::VEC_WIDTH; w++) {
					#pragma HLS UNROLL
					temp_c1[w] = temp_a1[w] * temp_b[w][j];
					temp_c2[w] = temp_a2[w] * temp_b[w][j];
				}

				for (int w=0; w < config_t::VEC_WIDTH; w++) {
					#pragma HLS UNROLL
					c_buffer1[j] += temp_c1[w];
					c_buffer2[j] += temp_c2[w];
				}
			}


			if (j == config_t::ITER_PE - 1) {

				if (pe_index > 0) {
					c_read1 = c_in_1.read();
					c_read2 = c_in_2.read();
				}
				else {
					c_read1 = 0;
					c_read2 = 0;
				}

				for (int w=0; w < config_t::ITER_PE; w++) {
					#pragma HLS UNROLL
					c_read1 += c_buffer1[w];
					c_read2 += c_buffer2[w];
				}

				c_out_1.write(c_read1);
				c_out_2.write(c_read2);
			}
		}
	}
}

template<typename config_t>
void ReadA(
		hls::stream<ap_uint<32> > &N_pipe_in,
		hls::stream<ap_uint<32> > &N_pipe_out,
		const ap_uint<config_t::DATA_WIDTH * config_t::SCALE_FACTOR> a[config_t::M * config_t::ITER_PE / config_t::NUM_TILE][config_t::VEC_WIDTH * 2 / config_t::SCALE_FACTOR],
		hls::stream<ap_uint<config_t::BUS_WIDTH / 2> > &a_pipes_1,
		hls::stream<ap_uint<config_t::BUS_WIDTH / 2> > &a_pipes_2,
		hls::stream<ap_uint<config_t::BUS_WIDTH / 2> > &a_pipes_3,
		hls::stream<ap_uint<config_t::BUS_WIDTH / 2> > &a_pipes_4,
		int pe_index)
{

	ap_uint<config_t::DATA_WIDTH * config_t::VEC_WIDTH * 2> temp;

	unsigned int N = N_pipe_in.read();

	if (pe_index < config_t::NUM_PE - 1) {
		N_pipe_out.write(N);
	}

	for (int i=0; i < N; i++) {
#pragma HLS loop_tripcount min=1 max=4096 avg=32
		for (int n=0; n < config_t::M * config_t::ITER_PE / config_t::NUM_TILE; n++) {
			#pragma HLS PIPELINE ii=1

			for (int w=0; w < config_t::VEC_WIDTH * 2 / config_t::SCALE_FACTOR; w ++) {
				#pragma HLS UNROLL
				temp.range(config_t::DATA_WIDTH * config_t::SCALE_FACTOR * (w + 1) - 1, config_t::DATA_WIDTH * config_t::SCALE_FACTOR * w) = a[n][w];
			}
			a_pipes_1.write(temp.range(config_t::BUS_WIDTH / 2 - 1, 0));
			a_pipes_2.write(temp.range(config_t::BUS_WIDTH - 1, config_t::BUS_WIDTH / 2));
			a_pipes_3.write(temp.range(config_t::BUS_WIDTH + config_t::BUS_WIDTH / 2 - 1, config_t::BUS_WIDTH));
			a_pipes_4.write(temp.range(config_t::BUS_WIDTH + config_t::BUS_WIDTH - 1, config_t::BUS_WIDTH + config_t::BUS_WIDTH / 2));
		}
	}
}


template<typename config_t>
void ReadN(
		hls::stream<ap_uint<32> > &N_in,
		hls::stream<ap_uint<32> > &N_pipe_1,
		hls::stream<ap_uint<32> > &N_pipe_2)
{
	unsigned int N = N_in.read();
	N_pipe_1.write(N);
	N_pipe_2.write(N);
}


template<typename config_t>
void TILE (
		const ap_uint<config_t::DATA_WIDTH * config_t::SCALE_FACTOR> a[config_t::NUM_PE][config_t::M * config_t::ITER_PE / config_t::NUM_TILE][config_t::VEC_WIDTH * 2 / config_t::SCALE_FACTOR],
		hls::stream<ap_uint<32> > &N_in,
		hls::stream<ap_uint<config_t::BUS_WIDTH / 2> > &b_in_1,
		hls::stream<ap_uint<config_t::BUS_WIDTH / 2> > &b_in_2,
		hls::stream<ap_uint<config_t::OUT_DATA_WIDTH> > &c_out_1,
		hls::stream<ap_uint<config_t::OUT_DATA_WIDTH> > &c_out_2)
{
	hls::stream<ap_uint<config_t::BUS_WIDTH / 2> > a_pipes_1[config_t::NUM_PE];
	hls::stream<ap_uint<config_t::BUS_WIDTH / 2> > a_pipes_2[config_t::NUM_PE];
	hls::stream<ap_uint<config_t::BUS_WIDTH / 2> > a_pipes_3[config_t::NUM_PE];
	hls::stream<ap_uint<config_t::BUS_WIDTH / 2> > a_pipes_4[config_t::NUM_PE];
	hls::stream<ap_uint<32> > N_pipes[config_t::NUM_PE+1];
	hls::stream<ap_uint<32> > N_pipes_reada[config_t::NUM_PE+1];
	hls::stream<ap_uint<config_t::OUT_DATA_WIDTH> > c_pipes_1[config_t::NUM_PE];
	hls::stream<ap_uint<config_t::OUT_DATA_WIDTH> > c_pipes_2[config_t::NUM_PE];
	hls::stream<ap_uint<config_t::BUS_WIDTH / 2> > b_pipes_1[config_t::NUM_PE+1];
	hls::stream<ap_uint<config_t::BUS_WIDTH / 2> > b_pipes_2[config_t::NUM_PE+1];

#pragma HLS ARRAY_PARTITION variable=a_pipes_1 dim=1 complete
#pragma HLS ARRAY_PARTITION variable=a_pipes_2 dim=1 complete
#pragma HLS ARRAY_PARTITION variable=a_pipes_3 dim=1 complete
#pragma HLS ARRAY_PARTITION variable=a_pipes_4 dim=1 complete
#pragma HLS ARRAY_PARTITION variable=N_pipes dim=1 complete
#pragma HLS ARRAY_PARTITION variable=N_pipes_reada dim=1 complete
#pragma HLS ARRAY_PARTITION variable=c_pipes_1 dim=1 complete
#pragma HLS ARRAY_PARTITION variable=c_pipes_2 dim=1 complete
#pragma HLS ARRAY_PARTITION variable=b_pipes_1 dim=1 complete
#pragma HLS ARRAY_PARTITION variable=b_pipes_2 dim=1 complete
#pragma HLS DATAFLOW

	ReadN<config_t>(N_in, N_pipes[0], N_pipes_reada[0]);

	if (config_t::NUM_PE == 1) {
		ReadA<config_t>(
				N_pipes_reada[0],
				N_pipes_reada[1],
				a[0],
				a_pipes_1[0], a_pipes_2[0], a_pipes_3[0], a_pipes_4[0],
				0);

		PE<config_t> (
				N_pipes[0],
				N_pipes[1],
				a_pipes_1[0], a_pipes_2[0], a_pipes_3[0], a_pipes_4[0],
				b_in_1, b_in_2,
				b_pipes_1[0], b_pipes_2[0],
				c_pipes_1[0], c_pipes_2[0],
				c_out_1, c_out_2,
				0);
	}
	else if (config_t::NUM_PE == 2) {
		ReadA<config_t>(
				N_pipes_reada[0],
				N_pipes_reada[1],
				a[0],
				a_pipes_1[0], a_pipes_2[0], a_pipes_3[0], a_pipes_4[0],
				0);

		PE<config_t> (
				N_pipes[0],
				N_pipes[1],
				a_pipes_1[0], a_pipes_2[0], a_pipes_3[0], a_pipes_4[0],
				b_in_1, b_in_2,
				b_pipes_1[1], b_pipes_2[1],
				c_pipes_1[0], c_pipes_2[0],
				c_pipes_1[1], c_pipes_2[1],
				0);

		ReadA<config_t>(
				N_pipes_reada[1],
				N_pipes_reada[2],
				a[1],
				a_pipes_1[1], a_pipes_2[1], a_pipes_3[1], a_pipes_4[1],
				1);

		PE<config_t> (
				N_pipes[1],
				N_pipes[2],
				a_pipes_1[1], a_pipes_2[1], a_pipes_3[1], a_pipes_4[1],
				b_pipes_1[1], b_pipes_2[1],
				b_pipes_1[2], b_pipes_2[2],
				c_pipes_1[1], c_pipes_2[1],
				c_out_1, c_out_2,
				1);
	}
	else {
		ReadA<config_t>(
				N_pipes_reada[0],
				N_pipes_reada[1],
				a[0],
				a_pipes_1[0], a_pipes_2[0], a_pipes_3[0], a_pipes_4[0],
				0);

		PE<config_t> (
				N_pipes[0],
				N_pipes[1],
				a_pipes_1[0], a_pipes_2[0], a_pipes_3[0], a_pipes_4[0],
				b_in_1, b_in_2,
				b_pipes_1[1], b_pipes_2[1],
				c_pipes_1[0], c_pipes_2[0],
				c_pipes_1[1], c_pipes_2[1],
				0);

		for (int i=1; i < config_t::NUM_PE - 1; i++) {
			#pragma HLS UNROLL
			ReadA<config_t>(
					N_pipes_reada[i],
					N_pipes_reada[i + 1],
					a[i],
					a_pipes_1[i], a_pipes_2[i], a_pipes_3[i], a_pipes_4[i],
					i);

			PE<config_t> (
					N_pipes[i],
					N_pipes[i + 1],
					a_pipes_1[i], a_pipes_2[i], a_pipes_3[i], a_pipes_4[i],
					b_pipes_1[i], b_pipes_2[i],
					b_pipes_1[i + 1], b_pipes_2[i + 1],
					c_pipes_1[i], c_pipes_2[i],
					c_pipes_1[i + 1], c_pipes_2[i + 1],
					i);
		}

		ReadA<config_t>(
				N_pipes_reada[config_t::NUM_PE - 1],
				N_pipes_reada[config_t::NUM_PE],
				a[config_t::NUM_PE - 1],
				a_pipes_1[config_t::NUM_PE - 1], a_pipes_2[config_t::NUM_PE - 1], a_pipes_3[config_t::NUM_PE - 1], a_pipes_4[config_t::NUM_PE - 1],
				config_t::NUM_PE - 1);

		PE<config_t> (
				N_pipes[config_t::NUM_PE - 1],
				N_pipes[config_t::NUM_PE],
				a_pipes_1[config_t::NUM_PE - 1], a_pipes_2[config_t::NUM_PE - 1], a_pipes_3[config_t::NUM_PE - 1], a_pipes_4[config_t::NUM_PE - 1],
				b_pipes_1[config_t::NUM_PE - 1], b_pipes_2[config_t::NUM_PE - 1],
				b_pipes_1[config_t::NUM_PE], b_pipes_2[config_t::NUM_PE],
				c_pipes_1[config_t::NUM_PE - 1], c_pipes_2[config_t::NUM_PE - 1],
				c_out_1, c_out_2,
				config_t::NUM_PE - 1);
	}


}

template<typename config_t>
void ReadB (
		hls::stream<dataword>& in,
		hls::stream<ap_uint<config_t::BUS_WIDTH / 2> > b_pipes_1[config_t::NUM_TILE / 2],
		hls::stream<ap_uint<config_t::BUS_WIDTH / 2> > b_pipes_2[config_t::NUM_TILE / 2],
		hls::stream<ap_uint<32> > N_pipes[2 + config_t::NUM_TILE / 2])
{
	dataword temp;

	temp = in.read();
	unsigned int N = temp.data;

	for (int i=0; i < 2 + config_t::NUM_TILE / 2; i++) {
		#pragma HLS UNROLL
		N_pipes[i].write(temp.data);
	}

	for (int i=0; i < N; i++) {
#pragma HLS loop_tripcount min=1 max=4096 avg=32
		for (int j=0; j < config_t::K / config_t::VEC_WIDTH; j++) {
			#pragma HLS PIPELINE ii=1
			temp = in.read();
			for (int t=0; t < config_t::NUM_TILE / 2; t++) {
				#pragma HLS UNROLL
				b_pipes_1[t].write(temp.data.range(config_t::BUS_WIDTH / 2 - 1, 0));
				b_pipes_2[t].write(temp.data.range(config_t::BUS_WIDTH - 1, config_t::BUS_WIDTH / 2));
			}
		}
	}
}


template<typename config_t>
void ConvertWidthC (
		hls::stream<ap_uint<32> > &N_pipe,
		hls::stream<ap_uint<config_t::OUT_DATA_WIDTH> > c_pipes[config_t::NUM_TILE / 2][2],
		hls::stream<ap_uint<config_t::OUT_DATA_WIDTH * config_t::NUM_TILE> > &out
		)
{
	dataword out_data;
	ap_uint<config_t::OUT_DATA_WIDTH * config_t::NUM_TILE> temp;


	unsigned int N = N_pipe.read();

	for (int i=0; i < N; i++) {
#pragma HLS loop_tripcount min=1 max=4096 avg=32
		for (int n=0; n < config_t::M / config_t::OUT_VEC_WIDTH; n++) {
			for (int j=0; j < config_t::OUT_VEC_WIDTH / config_t::NUM_TILE; j++) {
				#pragma HLS PIPELINE ii=1

				for (int t=0; t < config_t::NUM_TILE / 2; t++) {
					#pragma HLS UNROLL
					temp.range(2*t*config_t::OUT_DATA_WIDTH + config_t::OUT_DATA_WIDTH-1, 2*t*config_t::OUT_DATA_WIDTH) = c_pipes[t][0].read();
					temp.range((2*t + 1)*config_t::OUT_DATA_WIDTH + config_t::OUT_DATA_WIDTH-1, (2*t + 1)*config_t::OUT_DATA_WIDTH) = c_pipes[t][1].read();
				}

				out.write(temp);
			}
		}
	}
}


template<typename config_t>
void WriteC (
		hls::stream<ap_uint<32> > &N_pipe,
		hls::stream<ap_uint<config_t::OUT_DATA_WIDTH * config_t::NUM_TILE> > &in,
		hls::stream<dataword>& out)
{
	dataword out_data;

	unsigned int N = N_pipe.read();

	for (int i=0; i < N; i++) {
#pragma HLS loop_tripcount min=1 max=4096 avg=32
		for (int n=0; n < config_t::M / config_t::OUT_VEC_WIDTH; n++) {
			for (int j=0; j < config_t::OUT_VEC_WIDTH / config_t::NUM_TILE; j++) {
				#pragma HLS PIPELINE ii=1
				out_data.data(config_t::OUT_DATA_WIDTH * config_t::NUM_TILE * j + config_t::OUT_DATA_WIDTH * config_t::NUM_TILE - 1, config_t::OUT_DATA_WIDTH * config_t::NUM_TILE * j) = in.read();
			}

			out_data.id = config_t::id;
			out_data.last = n == config_t::M / config_t::OUT_VEC_WIDTH - 1 ? 1 : 0;
			out_data.user = 0;
			out_data.dest = config_t::dest;
			out.write(out_data);
		}
	}
}

template<typename config_t>
void MatMul(
		const ap_uint<config_t::DATA_WIDTH * config_t::SCALE_FACTOR> a[config_t::NUM_TILE / 2][config_t::NUM_PE][config_t::M * config_t::ITER_PE / config_t::NUM_TILE][config_t::VEC_WIDTH * 2 / config_t::SCALE_FACTOR],
		hls::stream<dataword>& b,
		hls::stream<dataword>& c)
{
	hls::stream<ap_uint<config_t::BUS_WIDTH/2> > b_pipes_1[config_t::NUM_TILE / 2];
	hls::stream<ap_uint<config_t::BUS_WIDTH/2> > b_pipes_2[config_t::NUM_TILE / 2];
	hls::stream<ap_uint<config_t::OUT_DATA_WIDTH> > c_pipes[config_t::NUM_TILE / 2][2];
	hls::stream<ap_uint<config_t::OUT_DATA_WIDTH * config_t::NUM_TILE> > conv_pipe;
	hls::stream<ap_uint<32> > N_pipes[2 + config_t::NUM_TILE / 2];
	hls::stream<dataword> b_in;

#pragma HLS ARRAY_PARTITION variable=a dim=1 complete
#pragma HLS ARRAY_PARTITION variable=a dim=2 complete
#pragma HLS ARRAY_PARTITION variable=a dim=4 complete
#pragma HLS ARRAY_PARTITION variable=b_pipes_1 dim=0 complete
#pragma HLS ARRAY_PARTITION variable=b_pipes_2 dim=0 complete
#pragma HLS ARRAY_PARTITION variable=c_pipes dim=0 complete
#pragma HLS ARRAY_PARTITION variable=N_pipes dim=0 complete

#pragma HLS DATAFLOW

	ReadB<config_t>(b, b_pipes_1, b_pipes_2, N_pipes);

	for (int i=0; i < config_t::NUM_TILE / 2; i++) {
		#pragma HLS UNROLL

		TILE<config_t>(
				a[i],
				N_pipes[2 + i],
				b_pipes_1[i], b_pipes_2[i],
				c_pipes[i][0], c_pipes[i][1]);
	}

	ConvertWidthC<config_t>(N_pipes[0], c_pipes, conv_pipe);
	WriteC<config_t>(N_pipes[1], conv_pipe, c);

}
