#ifndef PARSER_HPP
#define PARSER_HPP

#include <algorithm>
#include <string>
#include <zlib.h>
#include <cstdlib>
#include <iostream>
#include <vector>
#include <time.h>
#include "ifaststream.hpp"
#include "strobe_read.hpp"

using namespace std;

/*
 * reads 'read's from multiple FASTQ files (simultaneously)
 *
 * skips any reads with Ns!
 */

template <int size, int cnt = 1, typename T = char> // size of reads in base pairs
class ireadstream {
private:
	vector<ifaststream*> ifs_;
	bool eof_;
	bool is_open_;
public:
	ireadstream(string filenames[]) {
		for (size_t i = 0; i < cnt; ++i) {
			ifs_.push_back(new ifaststream(filenames[i]));
		}
		is_open_ = true;
		eof_ = false;
		read_ahead();
	}

//	ireadstream(const char *filename, ...) {
//		va_list ap;
//		va_start(ap, filename);
//		for (size_t i = 0; i < cnt; ++i) {
//			ifs_.push_back(new ifaststream(filename));
//			filename = va_arg(ap, const char *);
//		}
//		va_end(ap);
//		is_open_ = true;
//		eof_ = false;
//		read_ahead();
//	}

	void reset() {
		for (int i = 0; i < cnt; ++i) {
			ifs_[i]->reset();
		}
		is_open_ = true;
		eof_ = false;
		read_ahead();
	}
	virtual ~ireadstream() {
		close();
	}

	void close() {
		if (is_open()) {
			for (size_t i = 0; i < cnt; ++i) {
				delete ifs_[i];
				ifs_[i] = NULL;
			}
			is_open_ = false;
		}
	}

	ireadstream& operator>>(strobe_read<size,cnt,T> &sr) {
		if (!is_open() || eof()) {
			return *this;
		}
		sr = next_sr_;
		read_ahead();
		return *this;
	}

	inline bool is_open() const {
		/*for (size_t i = 0; i < cnt; ++i) {
			if (ifs_[i] == NULL || !ifs_[i]->is_open()) {
				return false;
			}
		}
		return true;*/
		return is_open_;
	}

	inline bool eof() const {
		/*for (size_t i = 0; i < cnt; ++i) {
			if (ifs_[i] == NULL || ifs_[i]->eof()) {
				return true;
			}
		}
		return false;*/
		return eof_;
	}

	vector<strobe_read<size,cnt,T> >* readAll(int number = -1) { // read count `reads`, default: 2^32 - 1
		vector<strobe_read<size,cnt,T> > *v = new vector<strobe_read<size,cnt,T> >();
		strobe_read<size,cnt,T> sr;
		while (!eof_ && number--) {
			this->operator>>(sr);
			v->push_back(sr);
		}
		return v;
	}

private:
	strobe_read<size,cnt,T> next_sr_;

	inline void read_ahead() {
		if (!is_open_) {
			return;
		}
		long time0 = time(0);
		while (!eof() && !read(next_sr_)) {
			if (time(0) > time0 + 3) {
				std::cerr << "Unable to read data";
				exit(0);
			}
		}
	}

	inline bool read(strobe_read<size,cnt,T> &sr) {
		if (!is_open() || eof()) {
			return false;
		}
		bool valid = true;
		string name, seq, qual;
		for (size_t i = 0; i < cnt; ++i) {
			*ifs_[i] >> name >> seq >> qual;
			for (size_t j = 0; valid && j < size; ++j) { // if at least one letter isn't ACGT
				if (!is_nucl(seq[j])) {
					valid = false;
				}
			}
			if (ifs_[i]->eof()) {
				eof_ = true;
			}
			if (valid) {
				sr.put(i, seq);
			}
		}
		return valid;
	}
};

#endif
