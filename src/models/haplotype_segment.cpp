////////////////////////////////////////////////////////////////////////////////
// Copyright (C) 2018 Olivier Delaneau, University of Lausanne
// 
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
// 
// The above copyright notice and this permission notice shall be included in
// all copies or substantial portions of the Software.
// 
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
// SOFTWARE.
////////////////////////////////////////////////////////////////////////////////
#include <models/haplotype_segment.h>

haplotype_segment::haplotype_segment(genotype * _G, bitmatrix & _H, vector < unsigned int > & _idxH, coordinates & C, hmm_parameters & _M) : G(_G), H(_H), idxH(_idxH), M(_M) {
	segment_first = C.start_segment;
	segment_last = C.stop_segment;
	locus_first = C.start_locus;
	locus_last = C.stop_locus;
	ambiguous_first = C.start_ambiguous;
	ambiguous_last = C.stop_ambiguous;
	transition_first = C.start_transition;
	n_cond_haps = idxH.size();
	prob1 = vector < double > (HAP_NUMBER * n_cond_haps, 1.0);
	prob2 = vector < double > (HAP_NUMBER * n_cond_haps, 1.0);
	probSumH1 = vector < double > (HAP_NUMBER, 1.0);
	probSumH2 = vector < double > (HAP_NUMBER, 1.0);
	probSumK1 = vector < double > (n_cond_haps, 1.0);
	probSumK2 = vector < double > (n_cond_haps, 1.0);
	probSumT1 = 1.0;
	probSumT2 = 1.0;
	Alpha = vector < vector < double > > (segment_last - segment_first + 1, vector < double > (HAP_NUMBER * n_cond_haps, 0.0));
	Beta = vector < vector < double > > (segment_last - segment_first + 1, vector < double > (HAP_NUMBER * n_cond_haps, 0.0));
	AlphaSum = vector < vector < double > > (segment_last - segment_first + 1, vector < double > (HAP_NUMBER, 0.0));
	BetaSum = vector < double > (HAP_NUMBER, 0.0);
}

haplotype_segment::~haplotype_segment() {
	G = NULL;
	segment_first = 0;
	segment_last = 0;
	locus_first = 0;
	locus_last = 0;
	ambiguous_first = 0;
	ambiguous_last = 0;
	transition_first = 0;
	n_cond_haps = 0;
	curr_segment_index = 0;
	curr_segment_locus = 0;
	curr_abs_locus = 0;
	curr_rel_locus = 0;
	curr_rel_segment_index = 0;
	curr_abs_ambiguous = 0;
	curr_abs_transition = 0;
	probSumT1 = 0.0;
	probSumT2 = 0.0;
	prob1.clear();
	prob2.clear();
	probSumK1.clear();
	probSumK2.clear();
	probSumH1.clear();
	probSumH2.clear();
	Alpha.clear();
	Beta.clear();
	AlphaSum.clear();
	BetaSum.clear();
}

void haplotype_segment::forward() {
	curr_segment_index = segment_first;
	curr_segment_locus = 0;
	curr_abs_ambiguous = ambiguous_first;
	for (curr_abs_locus = locus_first ; curr_abs_locus <= locus_last ; curr_abs_locus++) {
		curr_rel_locus = curr_abs_locus - locus_first;
		bool scale = (curr_rel_locus % HAP_SCALE == 0);
		bool paired = (curr_rel_locus % 2 == 0);
		bool amb = VAR_GET_AMB(MOD2(curr_abs_locus), G->Variants[DIV2(curr_abs_locus)]);

		if (amb) paired?AMB2():AMB1();
		else paired?HOM2():HOM1();
		if (curr_rel_locus != 0) {
			if (curr_segment_locus == 0) paired?COLLAPSE2(true):COLLAPSE1(true);
			else paired?RUN2(true):RUN1(true);
		}
		paired?SUM2():SUM1();
		if (curr_segment_locus == (G->Lengths[curr_segment_index] - 1)) paired?SUMK2():SUMK1();
		if (scale) paired?SCALE2():SCALE1();
		if (curr_segment_locus == G->Lengths[curr_segment_index] - 1) {
			Alpha[curr_segment_index - segment_first] = (paired?prob2:prob1);
			AlphaSum[curr_segment_index - segment_first] = (paired?probSumH2:probSumH1);
		}
		curr_segment_locus ++;
		curr_abs_ambiguous += amb;
		if (curr_segment_locus >= G->Lengths[curr_segment_index]) {
			curr_segment_index++;
			curr_segment_locus = 0;
		}
	}
}

void haplotype_segment::backward() {
	curr_segment_index = segment_last;
	curr_segment_locus = G->Lengths[segment_last] - 1;
	curr_abs_ambiguous = ambiguous_last;
	for (curr_abs_locus = locus_last ; curr_abs_locus >= locus_first ; curr_abs_locus--) {
		curr_rel_locus = curr_abs_locus - locus_first;
		bool scale = (curr_rel_locus % HAP_SCALE == 0);
		bool paired = (curr_rel_locus % 2 == 0);
		bool amb = VAR_GET_AMB(MOD2(curr_abs_locus), G->Variants[DIV2(curr_abs_locus)]);
		if (amb) paired?AMB2():AMB1();
		else paired?HOM2():HOM1();
		if (curr_abs_locus != locus_last) {
			if (curr_segment_locus == G->Lengths[curr_segment_index] - 1) paired?COLLAPSE2(false):COLLAPSE1(false);
			else paired?RUN2(false):RUN1(false);
		}
		paired?SUM2():SUM1();
		if (curr_segment_locus == 0) paired?SUMK2():SUMK1();
		if (scale) paired?SCALE2():SCALE1();
		if (curr_segment_locus == 0 && curr_abs_locus != locus_first) Beta[curr_segment_index - segment_first] = (paired?prob2:prob1);
		if (curr_abs_locus == 0) BetaSum=(paired?probSumH2:probSumH1);
		curr_segment_locus--;
		curr_abs_ambiguous -= amb;
		if (curr_segment_locus < 0 && curr_segment_index > 0) {
			curr_segment_index--;
			curr_segment_locus = G->Lengths[curr_segment_index] - 1;
		}
	}
}

int haplotype_segment::expectation(vector < double > & transition_probabilities) {
	forward();
	backward();

	unsigned int n_transitions = 0;
	if (!segment_first) {
		double sumHap = 0.0, sumDip = 0.0;
		n_transitions = G->countDiplotypes(G->Diplotypes[0]);
		for (int h = 0 ; h < HAP_NUMBER ; h ++) sumHap += BetaSum[h];
		vector < double > cprobs = vector < double > (n_transitions, 0.0);
		for (unsigned int d = 0, t = 0 ; d < 64 ; ++d) {
			if (DIP_GET(G->Diplotypes[0], d)) {
				cprobs[t] = (BetaSum[DIP_HAP0(d)]/sumHap) * (BetaSum[DIP_HAP1(d)]/sumHap);
				sumDip += cprobs[t];
				t++;
			}
		}
		for (unsigned int t = 0 ; t < n_transitions ; t ++) transition_probabilities[t] = (cprobs[t] / sumDip);
		curr_abs_transition += n_transitions;
	}

	unsigned int curr_abs_transition = transition_first;
	unsigned int curr_dipcount = 0, prev_dipcount = G->countDiplotypes(G->Diplotypes[segment_first]);

	curr_segment_index = segment_first;
	curr_segment_locus = 0;
	int n_underflow_recovered = 0;
	for (curr_abs_locus = locus_first ; curr_abs_locus <= locus_last ; curr_abs_locus ++) {
		curr_rel_locus = curr_abs_locus - locus_first;
		curr_rel_segment_index = curr_segment_index - segment_first;

		if (curr_rel_locus != 0 && curr_segment_locus == 0) {
			if (TRANSH()) return -1;
			if (TRANSD(n_underflow_recovered)) return -1;
			curr_dipcount = G->countDiplotypes(G->Diplotypes[curr_segment_index]);
			n_transitions = curr_dipcount * prev_dipcount;
			for (int t = 0 ; t < n_transitions ; t ++) transition_probabilities[curr_abs_transition + t] = DProbs[t] / sumDProbs;
			curr_abs_transition += n_transitions;
			prev_dipcount = curr_dipcount;
		}

		curr_segment_locus ++;
		if (curr_segment_locus >= G->Lengths[curr_segment_index]) {
			curr_segment_index++;
			curr_segment_locus = 0;
		}
	}
	return n_underflow_recovered;
}
