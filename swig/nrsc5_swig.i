/* -*- c++ -*- */

#define NRSC5_API

%include "gnuradio.i"			// the common stuff

//load generated python docstrings
%include "nrsc5_swig_doc.i"

%{
#include "nrsc5/l1_fm_encoder.h"
%}


%include "nrsc5/l1_fm_encoder.h"
GR_SWIG_BLOCK_MAGIC2(nrsc5, l1_fm_encoder);