/*******************************************************************************
 * Copyright (C) 2018 Olivier Delaneau, University of Lausanne
 * 
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 * 
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 * 
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 ******************************************************************************/
#ifndef _HAPLOTYPE_SEGMENT_H
#define _HAPLOTYPE_SEGMENT_H

#include <utils/otools.h>
#include <objects/compute_job.h>
#include <objects/hmm_parameters.h>

#define HAP_SCALE	50

class haplotype_segment {
private:
	//EXTERNAL DATA
	bitmatrix & H;
	vector < unsigned int > & idxH;
	hmm_parameters & M;
	genotype * G;

	//COORDINATES & CONSTANTS
	int segment_first;
	int segment_last;
	int locus_first;
	int locus_last;
	int ambiguous_first;
	int ambiguous_last;
	int transition_first;
	unsigned int n_cond_haps;

	//CURSORS
	int curr_segment_index;
	int curr_segment_locus;
	int curr_abs_locus;
	int curr_rel_locus;
	int curr_rel_segment_index;
	int curr_abs_ambiguous;
	int curr_abs_transition;

	//DYNAMIC ARRAYS
	double probSumT1;
	double probSumT2;
	vector < double > prob1;
	vector < double > prob2;
	vector < double > probSumK1;
	vector < double > probSumK2;
	vector < double > probSumH1;
	vector < double > probSumH2;
	vector < vector < double > > Alpha;
	vector < vector < double > > Beta;
	vector < vector < double > > AlphaSum;
	vector < double > BetaSum;

	//STATIC ARRAYS
	double HProbs [HAP_NUMBER * HAP_NUMBER];
	double DProbs [HAP_NUMBER * HAP_NUMBER * HAP_NUMBER * HAP_NUMBER];
	double sumHProbs, sumDProbs;

	//INLINED AND UNROLLED ROUTINES
	void HOM1();
	void HOM2();
	void AMB1();
	void AMB2();
	void SUM1();
	void SUM2();
	void SUMK1();
	void SUMK2();
	void SCALE2();
	void SCALE1();
	void COLLAPSE1(bool);
	void COLLAPSE2(bool);
	void RUN1(bool);
	void RUN2(bool);
	bool TRANSH();
	bool TRANSD(int &);

public:
	//CONSTRUCTOR/DESTRUCTOR
	haplotype_segment(genotype *, bitmatrix &, vector < unsigned int > &, coordinates &, hmm_parameters &);
	~haplotype_segment();

	void forward();
	void backward();
	int expectation(vector < double > &);
};

inline
void haplotype_segment::HOM2() {
	bool ag = VAR_GET_HAP0(MOD2(curr_abs_locus), G->Variants[DIV2(curr_abs_locus)]);
	for(int k = 0, i = 0 ; k != n_cond_haps ; ++k, i += HAP_NUMBER) {
		bool ah = H.get(idxH[k], curr_abs_locus);
		if (ag != ah) fill(prob2.begin() + i, prob2.begin() + i + HAP_NUMBER, M.ed);
		else fill(prob2.begin() + i, prob2.begin() + i + HAP_NUMBER, M.ee);
	}
}

inline
void haplotype_segment::HOM1() {
	bool ag = VAR_GET_HAP0(MOD2(curr_abs_locus), G->Variants[DIV2(curr_abs_locus)]);
	for(int k = 0, i = 0 ; k != n_cond_haps ; ++k, i += HAP_NUMBER) {
		bool ah = H.get(idxH[k], curr_abs_locus);
		if (ag != ah) fill(prob1.begin() + i, prob1.begin() + i + HAP_NUMBER, M.ed);
		else fill(prob1.begin() + i, prob1.begin() + i + HAP_NUMBER, M.ee);
	}
}

inline
void haplotype_segment::AMB2() {
	double galleles0[HAP_NUMBER], galleles1[HAP_NUMBER];
	galleles0[0] = HAP_GET(G->Ambiguous[curr_abs_ambiguous],0)?M.ed:M.ee;
	galleles0[1] = HAP_GET(G->Ambiguous[curr_abs_ambiguous],1)?M.ed:M.ee;
	galleles0[2] = HAP_GET(G->Ambiguous[curr_abs_ambiguous],2)?M.ed:M.ee;
	galleles0[3] = HAP_GET(G->Ambiguous[curr_abs_ambiguous],3)?M.ed:M.ee;
	galleles0[4] = HAP_GET(G->Ambiguous[curr_abs_ambiguous],4)?M.ed:M.ee;
	galleles0[5] = HAP_GET(G->Ambiguous[curr_abs_ambiguous],5)?M.ed:M.ee;
	galleles0[6] = HAP_GET(G->Ambiguous[curr_abs_ambiguous],6)?M.ed:M.ee;
	galleles0[7] = HAP_GET(G->Ambiguous[curr_abs_ambiguous],7)?M.ed:M.ee;
	galleles1[0] = HAP_GET(G->Ambiguous[curr_abs_ambiguous],0)?M.ee:M.ed;
	galleles1[1] = HAP_GET(G->Ambiguous[curr_abs_ambiguous],1)?M.ee:M.ed;
	galleles1[2] = HAP_GET(G->Ambiguous[curr_abs_ambiguous],2)?M.ee:M.ed;
	galleles1[3] = HAP_GET(G->Ambiguous[curr_abs_ambiguous],3)?M.ee:M.ed;
	galleles1[4] = HAP_GET(G->Ambiguous[curr_abs_ambiguous],4)?M.ee:M.ed;
	galleles1[5] = HAP_GET(G->Ambiguous[curr_abs_ambiguous],5)?M.ee:M.ed;
	galleles1[6] = HAP_GET(G->Ambiguous[curr_abs_ambiguous],6)?M.ee:M.ed;
	galleles1[7] = HAP_GET(G->Ambiguous[curr_abs_ambiguous],7)?M.ee:M.ed;
	for(int k = 0, i = 0 ; k != n_cond_haps ; ++k, i += HAP_NUMBER) {
		bool a = H.get(idxH[k], curr_abs_locus);
		if (a) memcpy(&prob2[i], &galleles1[0], HAP_NUMBER*sizeof(double));
		else memcpy(&prob2[i], &galleles0[0], HAP_NUMBER*sizeof(double));
	}
}

inline
void haplotype_segment::AMB1() {
	double galleles0[HAP_NUMBER], galleles1[HAP_NUMBER];
	galleles0[0] = HAP_GET(G->Ambiguous[curr_abs_ambiguous],0)?M.ed:M.ee;
	galleles0[1] = HAP_GET(G->Ambiguous[curr_abs_ambiguous],1)?M.ed:M.ee;
	galleles0[2] = HAP_GET(G->Ambiguous[curr_abs_ambiguous],2)?M.ed:M.ee;
	galleles0[3] = HAP_GET(G->Ambiguous[curr_abs_ambiguous],3)?M.ed:M.ee;
	galleles0[4] = HAP_GET(G->Ambiguous[curr_abs_ambiguous],4)?M.ed:M.ee;
	galleles0[5] = HAP_GET(G->Ambiguous[curr_abs_ambiguous],5)?M.ed:M.ee;
	galleles0[6] = HAP_GET(G->Ambiguous[curr_abs_ambiguous],6)?M.ed:M.ee;
	galleles0[7] = HAP_GET(G->Ambiguous[curr_abs_ambiguous],7)?M.ed:M.ee;
	galleles1[0] = HAP_GET(G->Ambiguous[curr_abs_ambiguous],0)?M.ee:M.ed;
	galleles1[1] = HAP_GET(G->Ambiguous[curr_abs_ambiguous],1)?M.ee:M.ed;
	galleles1[2] = HAP_GET(G->Ambiguous[curr_abs_ambiguous],2)?M.ee:M.ed;
	galleles1[3] = HAP_GET(G->Ambiguous[curr_abs_ambiguous],3)?M.ee:M.ed;
	galleles1[4] = HAP_GET(G->Ambiguous[curr_abs_ambiguous],4)?M.ee:M.ed;
	galleles1[5] = HAP_GET(G->Ambiguous[curr_abs_ambiguous],5)?M.ee:M.ed;
	galleles1[6] = HAP_GET(G->Ambiguous[curr_abs_ambiguous],6)?M.ee:M.ed;
	galleles1[7] = HAP_GET(G->Ambiguous[curr_abs_ambiguous],7)?M.ee:M.ed;
	for(int k = 0, i = 0 ; k != n_cond_haps ; ++k, i += HAP_NUMBER) {
		bool a = H.get(idxH[k], curr_abs_locus);
		if (a) memcpy(&prob1[i], &galleles1[0], HAP_NUMBER*sizeof(double));
		else memcpy(&prob1[i], &galleles0[0], HAP_NUMBER*sizeof(double));
	}
}

inline
void haplotype_segment::SUM2() {
	double sum0 = 0.0, sum1 = 0.0, sum2 = 0.0, sum3 = 0.0, sum4 = 0.0, sum5 = 0.0, sum6 = 0.0, sum7 = 0.0;
	for(int k = 0, i = 0 ; k != n_cond_haps ; ++k, i += HAP_NUMBER) {
		sum0 += prob2[i + 0];
		sum1 += prob2[i + 1];
		sum2 += prob2[i + 2];
		sum3 += prob2[i + 3];
		sum4 += prob2[i + 4];
		sum5 += prob2[i + 5];
		sum6 += prob2[i + 6];
		sum7 += prob2[i + 7];
	}
	probSumH2[0] = sum0;
	probSumH2[1] = sum1;
	probSumH2[2] = sum2;
	probSumH2[3] = sum3;
	probSumH2[4] = sum4;
	probSumH2[5] = sum5;
	probSumH2[6] = sum6;
	probSumH2[7] = sum7;
	probSumT2 = sum0 + sum1 + sum2 + sum3 + sum4 + sum5 + sum6 + sum7;
}

inline
void haplotype_segment::SUM1() {
	double sum0 = 0.0, sum1 = 0.0, sum2 = 0.0, sum3 = 0.0, sum4 = 0.0, sum5 = 0.0, sum6 = 0.0, sum7 = 0.0;
	for(int k = 0, i = 0 ; k != n_cond_haps ; ++k, i += HAP_NUMBER) {
		sum0 += prob1[i + 0];
		sum1 += prob1[i + 1];
		sum2 += prob1[i + 2];
		sum3 += prob1[i + 3];
		sum4 += prob1[i + 4];
		sum5 += prob1[i + 5];
		sum6 += prob1[i + 6];
		sum7 += prob1[i + 7];
	}
	probSumH1[0] = sum0;
	probSumH1[1] = sum1;
	probSumH1[2] = sum2;
	probSumH1[3] = sum3;
	probSumH1[4] = sum4;
	probSumH1[5] = sum5;
	probSumH1[6] = sum6;
	probSumH1[7] = sum7;
	probSumT1 = sum0 + sum1 + sum2 + sum3 + sum4 + sum5 + sum6 + sum7;
}

inline
void haplotype_segment::SUMK2() {
	for(int k = 0, i = 0 ; k != n_cond_haps ; ++k, i += HAP_NUMBER) {
		probSumK2[k] = prob2[i+0] + prob2[i+1] + prob2[i+2] + prob2[i+3] + prob2[i+4] + prob2[i+5] + prob2[i+6] + prob2[i+7];
	}
}

inline
void haplotype_segment::SUMK1() {
	for(int k = 0, i = 0 ; k != n_cond_haps ; ++k, i += HAP_NUMBER) {
		probSumK1[k] = prob1[i+0] + prob1[i+1] + prob1[i+2] + prob1[i+3] + prob1[i+4] + prob1[i+5] + prob1[i+6] + prob1[i+7];
	}
}

inline
void haplotype_segment::SCALE2() {
	double scaling = 1.0 / probSumT2;
	for(int k = 0, i = 0 ; k != n_cond_haps ; ++k, i += HAP_NUMBER) {
		prob2[i+0] *= scaling;
		prob2[i+1] *= scaling;
		prob2[i+2] *= scaling;
		prob2[i+3] *= scaling;
		prob2[i+4] *= scaling;
		prob2[i+5] *= scaling;
		prob2[i+6] *= scaling;
		prob2[i+7] *= scaling;
		probSumK2[k] *= scaling;
	}
	probSumH2[0] *= scaling;
	probSumH2[1] *= scaling;
	probSumH2[2] *= scaling;
	probSumH2[3] *= scaling;
	probSumH2[4] *= scaling;
	probSumH2[5] *= scaling;
	probSumH2[6] *= scaling;
	probSumH2[7] *= scaling;
	probSumT2 = 1.0;
}

inline
void haplotype_segment::SCALE1() {
	double scaling = 1.0 / probSumT1;
	for(int k = 0, i = 0 ; k != n_cond_haps ; ++k, i += HAP_NUMBER) {
		prob1[i+0] *= scaling;
		prob1[i+1] *= scaling;
		prob1[i+2] *= scaling;
		prob1[i+3] *= scaling;
		prob1[i+4] *= scaling;
		prob1[i+5] *= scaling;
		prob1[i+6] *= scaling;
		prob1[i+7] *= scaling;
		probSumK1[k] *= scaling;
	}
	probSumH1[0] *= scaling;
	probSumH1[1] *= scaling;
	probSumH1[2] *= scaling;
	probSumH1[3] *= scaling;
	probSumH1[4] *= scaling;
	probSumH1[5] *= scaling;
	probSumH1[6] *= scaling;
	probSumH1[7] *= scaling;
	probSumT1 = 1.0;
}

inline
void haplotype_segment::COLLAPSE2(bool forward) {
	double tmp_prob0 = M.nt[curr_abs_locus-forward];
	//double tmp_prob1 = probSumT1 * M.tfreq[curr_abs_locus-forward];
	double tmp_prob1 = probSumT1 * M.t[curr_abs_locus-forward] / n_cond_haps;
	for(int k = 0, i = 0 ; k != n_cond_haps ; ++k, i += HAP_NUMBER) {
		double factor = probSumK1[k] * tmp_prob0 + tmp_prob1;
		prob2[i + 0] *= factor;
		prob2[i + 1] *= factor;
		prob2[i + 2] *= factor;
		prob2[i + 3] *= factor;
		prob2[i + 4] *= factor;
		prob2[i + 5] *= factor;
		prob2[i + 6] *= factor;
		prob2[i + 7] *= factor;
	}
}

inline
void haplotype_segment::COLLAPSE1(bool forward) {
	double tmp_prob0 = M.nt[curr_abs_locus-forward];
	//double tmp_prob1 = probSumT2 * M.tfreq[curr_abs_locus-forward];
	double tmp_prob1 = probSumT2 * M.t[curr_abs_locus-forward] / n_cond_haps;
	for(int k = 0, i = 0 ; k != n_cond_haps ; ++k, i += HAP_NUMBER) {
		double factor = probSumK2[k] * tmp_prob0 + tmp_prob1;
		prob1[i + 0] *= factor;
		prob1[i + 1] *= factor;
		prob1[i + 2] *= factor;
		prob1[i + 3] *= factor;
		prob1[i + 4] *= factor;
		prob1[i + 5] *= factor;
		prob1[i + 6] *= factor;
		prob1[i + 7] *= factor;
	}
}

inline
void haplotype_segment::RUN2(bool forward) {
	double nt = M.nt[curr_abs_locus-forward];
	//double tfreq = M.tfreq[curr_abs_locus-forward];
	double tfreq = M.t[curr_abs_locus-forward] / n_cond_haps;
	double tFreq0 = probSumH1[0] * tfreq;
	double tFreq1 = probSumH1[1] * tfreq;
	double tFreq2 = probSumH1[2] * tfreq;
	double tFreq3 = probSumH1[3] * tfreq;
	double tFreq4 = probSumH1[4] * tfreq;
	double tFreq5 = probSumH1[5] * tfreq;
	double tFreq6 = probSumH1[6] * tfreq;
	double tFreq7 = probSumH1[7] * tfreq;
	for(int k = 0, i = 0 ; k != n_cond_haps ; ++k, i += HAP_NUMBER) {
		prob2[i + 0] *= prob1[i + 0] * nt + tFreq0;
		prob2[i + 1] *= prob1[i + 1] * nt + tFreq1;
		prob2[i + 2] *= prob1[i + 2] * nt + tFreq2;
		prob2[i + 3] *= prob1[i + 3] * nt + tFreq3;
		prob2[i + 4] *= prob1[i + 4] * nt + tFreq4;
		prob2[i + 5] *= prob1[i + 5] * nt + tFreq5;
		prob2[i + 6] *= prob1[i + 6] * nt + tFreq6;
		prob2[i + 7] *= prob1[i + 7] * nt + tFreq7;
	}
}


inline
void haplotype_segment::RUN1(bool forward) {
	double nt = M.nt[curr_abs_locus-forward];
	//double tfreq = M.tfreq[curr_abs_locus-forward];
	double tfreq = M.t[curr_abs_locus-forward] / n_cond_haps;
	double tFreq0 = probSumH2[0] * tfreq;
	double tFreq1 = probSumH2[1] * tfreq;
	double tFreq2 = probSumH2[2] * tfreq;
	double tFreq3 = probSumH2[3] * tfreq;
	double tFreq4 = probSumH2[4] * tfreq;
	double tFreq5 = probSumH2[5] * tfreq;
	double tFreq6 = probSumH2[6] * tfreq;
	double tFreq7 = probSumH2[7] * tfreq;
	for(int k = 0, i = 0 ; k != n_cond_haps ; ++k, i += HAP_NUMBER) {
		prob1[i + 0] *= prob2[i + 0] * nt + tFreq0;
		prob1[i + 1] *= prob2[i + 1] * nt + tFreq1;
		prob1[i + 2] *= prob2[i + 2] * nt + tFreq2;
		prob1[i + 3] *= prob2[i + 3] * nt + tFreq3;
		prob1[i + 4] *= prob2[i + 4] * nt + tFreq4;
		prob1[i + 5] *= prob2[i + 5] * nt + tFreq5;
		prob1[i + 6] *= prob2[i + 6] * nt + tFreq6;
		prob1[i + 7] *= prob2[i + 7] * nt + tFreq7;
	}
}

inline
bool haplotype_segment::TRANSH() {
	sumHProbs = 0.0;
	for (int h1 = 0 ; h1 < HAP_NUMBER ; h1++) {
		double sum0 = 0.0, sum1 = 0.0, sum2 = 0.0, sum3 = 0.0, sum4 = 0.0, sum5 = 0.0, sum6 = 0.0, sum7 = 0.0;
		for (int k = 0 ; k < n_cond_haps ; k ++) {
			//double alpha = Alpha[curr_rel_segment_index - 1][k*HAP_NUMBER + h1] * M.nt[curr_abs_locus-1] + AlphaSum[curr_rel_segment_index - 1][h1] * M.tfreq[curr_abs_locus - 1];
			double alpha = Alpha[curr_rel_segment_index - 1][k*HAP_NUMBER + h1] * M.nt[curr_abs_locus-1] + AlphaSum[curr_rel_segment_index - 1][h1] * M.t[curr_abs_locus - 1] / n_cond_haps;
			sum0 += alpha * Beta[curr_rel_segment_index][k*HAP_NUMBER + 0];
			sum1 += alpha * Beta[curr_rel_segment_index][k*HAP_NUMBER + 1];
			sum2 += alpha * Beta[curr_rel_segment_index][k*HAP_NUMBER + 2];
			sum3 += alpha * Beta[curr_rel_segment_index][k*HAP_NUMBER + 3];
			sum4 += alpha * Beta[curr_rel_segment_index][k*HAP_NUMBER + 4];
			sum5 += alpha * Beta[curr_rel_segment_index][k*HAP_NUMBER + 5];
			sum6 += alpha * Beta[curr_rel_segment_index][k*HAP_NUMBER + 6];
			sum7 += alpha * Beta[curr_rel_segment_index][k*HAP_NUMBER + 7];
		}
		HProbs[h1*HAP_NUMBER+0] = sum0;
		HProbs[h1*HAP_NUMBER+1] = sum1;
		HProbs[h1*HAP_NUMBER+2] = sum2;
		HProbs[h1*HAP_NUMBER+3] = sum3;
		HProbs[h1*HAP_NUMBER+4] = sum4;
		HProbs[h1*HAP_NUMBER+5] = sum5;
		HProbs[h1*HAP_NUMBER+6] = sum6;
		HProbs[h1*HAP_NUMBER+7] = sum7;
		sumHProbs += sum0 + sum1 + sum2 + sum3 + sum4 + sum5 + sum6 + sum7;
	}
	return (isnan(sumHProbs) || sumHProbs < numeric_limits<double>::min());
}

inline
bool haplotype_segment::TRANSD(int & n_underflows_recovered) {
	sumDProbs= 0.0;
	double scaling = 1.0 / sumHProbs;
	for (int pd = 0, t = 0 ; pd < 64 ; ++pd) {
		if (DIP_GET(G->Diplotypes[curr_segment_index-1], pd)) {
			for (int nd = 0 ; nd < 64 ; ++nd) {
				if (DIP_GET(G->Diplotypes[curr_segment_index], nd)) {
					unsigned int prev_hap0 = DIP_HAP0(pd);
					unsigned int prev_hap1 = DIP_HAP1(pd);
					unsigned int next_hap0 = DIP_HAP0(nd);
					unsigned int next_hap1 = DIP_HAP1(nd);
					DProbs[t] = (HProbs[prev_hap0*HAP_NUMBER+next_hap0] * scaling) * (HProbs[prev_hap1*HAP_NUMBER+next_hap1] * scaling);;
					sumDProbs += DProbs[t];
					t++;
				}
			}
		}
	}
	if (sumDProbs < numeric_limits<double>::min()) {
		sumDProbs = 0.0;
		n_underflows_recovered++;
		for (int pd = 0, t = 0 ; pd < 64 ; ++pd) {
			if (DIP_GET(G->Diplotypes[curr_segment_index-1], pd)) {
				for (int nd = 0 ; nd < 64 ; ++nd) {
					if (DIP_GET(G->Diplotypes[curr_segment_index], nd)) {
						unsigned int prev_hap0 = DIP_HAP0(pd);
						unsigned int prev_hap1 = DIP_HAP1(pd);
						unsigned int next_hap0 = DIP_HAP0(nd);
						unsigned int next_hap1 = DIP_HAP1(nd);
						DProbs[t] = (HProbs[prev_hap0*HAP_NUMBER+next_hap0] * scaling) + (HProbs[prev_hap1*HAP_NUMBER+next_hap1] * scaling);
						sumDProbs += DProbs[t];
						t++;
					}
				}
			}
		}
	}
	return (isnan(sumDProbs) || sumDProbs < numeric_limits<double>::min());
}

#endif
