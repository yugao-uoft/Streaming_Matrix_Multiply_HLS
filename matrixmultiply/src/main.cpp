
#include <iostream>
#include <fstream>
#include "top.hpp"
#include <bitset>
using namespace std;

void generate_weights(
		DTYPE_IN b_cols[Config_MatMul::M * Config_MatMul::K],
		ap_uint<Config_MatMul::DATA_WIDTH * Config_MatMul::SCALE_FACTOR> b[Config_MatMul::NUM_TILE / 2][Config_MatMul::NUM_PE][Config_MatMul::M * Config_MatMul::ITER_PE / Config_MatMul::NUM_TILE][Config_MatMul::VEC_WIDTH * 2/ Config_MatMul::SCALE_FACTOR])
{


	int cnt = 0;
	for (int i=0; i<Config_MatMul::M / Config_MatMul::NUM_TILE; i++) {

		for (int j=0; j < Config_MatMul::NUM_TILE/2; j++) {

			for (int p=0; p < 2; p++) {

				for (int k=0; k < Config_MatMul::NUM_PE; k++) {

					for (int m=0; m < Config_MatMul::ITER_PE; m++) {

						for (int w=0; w < Config_MatMul::VEC_WIDTH / Config_MatMul::SCALE_FACTOR; w++) {

							for (int s=0; s < Config_MatMul::SCALE_FACTOR; s ++) {
								b[j][k][i * Config_MatMul::ITER_PE + m][w + p * Config_MatMul::VEC_WIDTH / Config_MatMul::SCALE_FACTOR].range(Config_MatMul::DATA_WIDTH * (1+s) - 1, Config_MatMul::DATA_WIDTH * s) = b_cols[cnt];
								cnt++;
							}
						}
					}
				}
			}
		}
	}
}

int main() {

	ofstream fout;
	fout.open("weights.hpp");

	hls::stream<dataword> in;
	hls::stream<dataword> out;

	dataword temp;
	int N=1;

	// define matrices
	ap_uint<Config_MatMul::DATA_WIDTH * Config_MatMul::SCALE_FACTOR> b[Config_MatMul::NUM_TILE / 2][Config_MatMul::NUM_PE][Config_MatMul::M * Config_MatMul::ITER_PE / Config_MatMul::NUM_TILE][Config_MatMul::VEC_WIDTH * 2/ Config_MatMul::SCALE_FACTOR];
	DTYPE_IN b_cols[Config_MatMul::M * Config_MatMul::K];
	DTYPE_IN a_cols[Config_MatMul::K * N];
	DTYPE_OUT c[Config_MatMul::M * N];
	DTYPE_OUT c_test[Config_MatMul::M * N];

	// column first storage
	for (int i=0; i < Config_MatMul::K; i++) {
		for (int j=0; j < N; j++) {
			a_cols[j + i * N] = rand()%256;
		}
	}

	for (int i=0; i < Config_MatMul::M; i++) {
		for (int j=0; j < Config_MatMul::K; j++) {
			b_cols[j + i * Config_MatMul::K] = rand() % 256;
		}
	}

	generate_weights(b_cols, b);


	cout << "\nb" << endl;
	for (int i=0; i < Config_MatMul::NUM_TILE / 2; i++) {
		for (int j=0; j < Config_MatMul::NUM_PE; j++) {
			for (int k=0; k < Config_MatMul::M * Config_MatMul::ITER_PE / Config_MatMul::NUM_TILE; k++) {
				for (int w=0; w < Config_MatMul::VEC_WIDTH * 2/ Config_MatMul::SCALE_FACTOR; w++) {
					cout << b[i][j][k][w] << " " ;
				}
			}
			cout << endl;
		}
		cout << endl;
	}



	fout << "\n\nap_uint<64> weights1["<<Config_MatMul::NUM_TILE / 2<<"]["<<Config_MatMul::NUM_PE<<"]["<<Config_MatMul::M * Config_MatMul::ITER_PE / Config_MatMul::NUM_TILE<<"]["<<Config_MatMul::VEC_WIDTH * 2/ Config_MatMul::SCALE_FACTOR<<"] = { ";
	for (int i=0; i<Config_MatMul::NUM_TILE / 2; i++) {
		fout << "{ ";
		for (int j=0; j < Config_MatMul::NUM_PE; j++) {
			fout << "{ ";
			for (int k=0; k < Config_MatMul::M * Config_MatMul::ITER_PE /Config_MatMul::NUM_TILE; k++) {
				fout << "{ ";
				for (int w=0; w < Config_MatMul::VEC_WIDTH * 2 / Config_MatMul::SCALE_FACTOR; w++) {
					if (w == Config_MatMul::VEC_WIDTH  * 2 / Config_MatMul::SCALE_FACTOR-1)
						fout << b[i][j][k][w];
					else {
						fout << b[i][j][k][w] <<", ";
					}
				}
				if (k == Config_MatMul::M * Config_MatMul::ITER_PE / Config_MatMul::NUM_TILE-1)
					fout  << " } ";
				else
					fout  << " }, ";
			}

			if (j == Config_MatMul::NUM_PE-1 )
				fout << " } ";
			else
				fout << " }, ";
		}

		if (i == Config_MatMul::NUM_TILE/2-1)
			fout << " } ";
		else
			fout << " }, ";
	}
	fout << " };"<< endl;


	temp.data = N;
	in.write(temp);
	int cnt=0;
	for (int i=0; i<N; i++) {
		cnt=0;
		for (int j=0; j < Config_MatMul::K/Config_MatMul::VEC_WIDTH; j++) {
			for (int w=0; w < Config_MatMul::VEC_WIDTH; w++) {
				temp.data(w*Config_MatMul::DATA_WIDTH + Config_MatMul::DATA_WIDTH-1, w*Config_MatMul::DATA_WIDTH) = a_cols[cnt*N + i];
				cnt++;
			}
			in.write(temp);
		}
	}

	// simulation
	for (int i=0; i < N; i++) {
		for (int j=0; j < Config_MatMul::M; j++) {
			c[i * Config_MatMul::M + j] = 0;
			for (int k=0; k < Config_MatMul::Config_MatMul::K; k++) {
				ap_int<16> tmp = b_cols[j * Config_MatMul::K + k] * a_cols[k * N + i];
				c[i * Config_MatMul::M + j] += tmp;
			}
		}

	}

	cout << "c" <<endl;
	for (int i=0; i < N; i++) {
		for (int j=0; j <Config_MatMul:: M; j++) {
			cout << c[i * Config_MatMul::M + j] << " ";
		}
		cout << endl;
	}
	cout << endl;


	kernel(in, out);


	for (int i=0; i < N; i++) {
		for (int j=0; j < Config_MatMul::M / Config_MatMul::OUT_VEC_WIDTH; j++) {
			temp = out.read();
			for (int k=0; k < Config_MatMul::OUT_VEC_WIDTH; k++) {
				c_test[k + j * Config_MatMul::OUT_VEC_WIDTH + i * Config_MatMul::M] = temp.data(k*Config_MatMul::OUT_DATA_WIDTH + Config_MatMul::OUT_DATA_WIDTH-1, k*Config_MatMul::OUT_DATA_WIDTH);
			}
		}
	}

	cout << "c_test" <<endl;
	for (int i=0; i < N; i++) {
		for (int j=0; j < Config_MatMul::M; j++) {
			cout << c_test[i * Config_MatMul::M + j] << " ";
		}
		cout << endl;
	}
	cout << endl;



	return 0;
}
















