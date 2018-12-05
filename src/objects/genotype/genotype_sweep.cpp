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
#include <objects/genotype/genotype_header.h>

// TO DO: make it forward-backward
void genotype::sample(vector < double > & CurrentTransProbabilities) {
	double sumProbs = 0.0;
	unsigned int prev_sampled = 0;
	unsigned int curr_dipcount = 0, prev_dipcount = 1;
	vector < double > currProbs = vector < double > (64, 0.0);
	vector < unsigned char > DipSampled = vector < unsigned char >(n_segments, 0);
	for (unsigned int s = 0, toffset = 0 ; s < n_segments ; s ++) {
		sumProbs = 0.0;
		curr_dipcount = countDiplotypes(Diplotypes[s]);
		for (unsigned int tabs = toffset + prev_sampled*curr_dipcount, trel = 0 ; trel < curr_dipcount ; ++trel, ++tabs)
			sumProbs += (currProbs[trel] = CurrentTransProbabilities[tabs]);
		prev_sampled = rng.sample(currProbs, sumProbs);
		makeDiplotypes(Diplotypes[s]);
		DipSampled[s] = curr_dipcodes[prev_sampled];
		toffset += prev_dipcount * curr_dipcount;
		prev_dipcount = curr_dipcount;
	}
	make(DipSampled);
}

void genotype::solve() {
	unsigned int curr_dipcount = 0, prev_dipcount = 1;
	vector < vector < double > > maxProbs = vector < vector < double > > (n_segments, vector < double > ());
	vector < vector < int > > maxIndexes = vector < vector < int > > (n_segments, vector < int > ());

	for (int s = 0, toffset = 0, trel = 0 ; s < n_segments ; s ++) {
		curr_dipcount = countDiplotypes(Diplotypes[s]);
		maxProbs[s] = vector < double > (curr_dipcount, 0.0);
		maxIndexes[s] = vector < int > (curr_dipcount, 0);
		for (int t = 0 ; t < prev_dipcount * curr_dipcount ; t++) {
			int prev_dip = t/curr_dipcount;
			int next_dip = t%curr_dipcount;
			//double currProb = (s?maxProbs[s-1][prev_dip]:1.0) * StoredProbs[t+toffset];
			double currProb = (s?maxProbs[s-1][prev_dip]:1.0) * (ProbMask[t+toffset]?ProbStored[trel++]:5e-7);
			if (currProb > maxProbs[s][next_dip]) {
				maxProbs[s][next_dip] = currProb;
				maxIndexes[s][next_dip] = prev_dip;
			}
		}
		double sumProb = 0.0;
		for (int d = 0 ; d < curr_dipcount ; d ++) sumProb += maxProbs[s][d];
		for (int d = 0 ; d < curr_dipcount ; d ++) maxProbs[s][d] /= sumProb;
		toffset += prev_dipcount * curr_dipcount;
		prev_dipcount = curr_dipcount;
	}

	vector < unsigned char > DipSampled = vector < unsigned char >(n_segments, 0);
	unsigned int bestDip = alg.imax(maxProbs.back());
	makeDiplotypes(Diplotypes.back());
	DipSampled.back() = curr_dipcodes[bestDip];
	for (int s = DipSampled.size() - 2 ; s >= 0 ; s --) {
		bestDip = maxIndexes[s+1][bestDip];
		makeDiplotypes(Diplotypes[s]);
		DipSampled[s] = curr_dipcodes[bestDip];
	}
	make(DipSampled);
}

void genotype::store(vector < double > & CurrentTransProbabilities) {
        if (ProbMask.size() == 0) {
		unsigned int countProb = 0;
		ProbMask = vector < bool > (n_transitions, false);
		for (unsigned int t = 0 ; t < n_transitions ; t ++) if (CurrentTransProbabilities[t] >= 1e-6) {
			ProbMask[t] = true;
			countProb++;
		}
		ProbStored = vector  < float > (countProb, 0.0);
	}
	for (unsigned int t = 0, trel = 0 ; t < n_transitions ; t ++) if (ProbMask[t]) ProbStored[trel++] += CurrentTransProbabilities[t];
}

/*
void genotype::store(vector < double > & CurrentTransProbabilities) {
	if (StoredProbs.size() == 0) StoredProbs = vector < float > (n_transitions, 0.0);
	for (unsigned int t = 0 ; t < n_transitions ; t ++) {
		StoredProbs[t] += CurrentTransProbabilities[t];
		//cout << t << " " << StoredProbs[t] << " " << CurrentTransProbabilities[t] << endl;
	}
}
*/
