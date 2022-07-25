/* -*- c++ -*- */
/*
 * Copyright 2017 Clayton Smith.
 *
 * This is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3, or (at your option)
 * any later version.
 *
 * This software is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this software; see the file COPYING.  If not, write to
 * the Free Software Foundation, Inc., 51 Franklin Street,
 * Boston, MA 02110-1301, USA.
 */

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <gnuradio/io_signature.h>
#include "fox_encoder_impl.h"

namespace gr {
  namespace nrsc5 {

    fox_encoder::sptr
    fox_encoder::make(const std::string& long_name)
    {
      return gnuradio::get_initial_sptr
        (new fox_encoder_impl(long_name));
    }


    /*
     * The private constructor
     */
    fox_encoder_impl::fox_encoder_impl(const std::string& long_name)
      : gr::sync_block("fox_encoder",
              gr::io_signature::make(0, 0, 0),
              gr::io_signature::make(1, 1, sizeof(unsigned char) * SIS_BITS))
    {
      set_output_multiple(BLOCKS_PER_FRAME);
      this->long_name = long_name;
      alfn = 800000000;
    }

    /*
     * Our virtual destructor.
     */
    fox_encoder_impl::~fox_encoder_impl()
    {
    }

    int
    fox_encoder_impl::work(int noutput_items,
        gr_vector_const_void_star &input_items,
        gr_vector_void_star &output_items)
    {
      unsigned char *out = (unsigned char *) output_items[0];

      bit = out;
      while (bit < out + (noutput_items * SIS_BITS)) {
        for (int block = 0; block < BLOCKS_PER_FRAME; block++) {
          unsigned char *start = bit;

          write_bit(PIDS_FORMATTED);
          write_bit(NO_EXTENSION);
          write_station_name_long();

          while (bit < start + 64) {
            write_bit(0);
          }
          write_bit(0); // Reserved
          write_bit(TIME_NOT_LOCKED);
          write_int((alfn >> (block*2)) & 0x3, 2);
          write_int(crc12(start), 12);
        }
        alfn++;
      }

      // Tell runtime system how many output items we produced.
      return noutput_items;
    }

    /* 1020s.pdf section 4.10
     * Note: The specified CRC is incorrect. It's actually a 16-bit CRC
     * truncated to 12 bits, and g(x) = X^16 + X^11 + X^3 + X + 1 */
    int
    fox_encoder_impl::crc12(unsigned char *sis)
    {
      unsigned short poly = 0xD010;
      unsigned short reg = 0x0000;
      int i, lowbit;

      for (i = 67; i >= 0; i--) {
        lowbit = reg & 1;
        reg >>= 1;
        reg ^= ((unsigned short)sis[i] << 15);
        if (lowbit) reg ^= poly;
      }
      for (i = 0; i < 16; i++) {
        lowbit = reg & 1;
        reg >>= 1;
        if (lowbit) reg ^= poly;
      }
      return reg ^ 0x955;
    }

    void
    fox_encoder_impl::write_bit(int b)
    {
      *(bit++) = b;
    }

    void
    fox_encoder_impl::write_int(int n, int len)
    {
      for (int i = 0; i < len; i++) {
        write_bit((n >> (len - i - 1)) & 1);
      }
    }

    void
    fox_encoder_impl::write_char5(char c)
    {
      int n;
      if (c >= 'A' && c <= 'Z') {
        n = (c - 'A');
      } else if (c >= 'a' && c <= 'z') {
        n = (c - 'a');
      } else {
        switch (c) {
          case '?': n = 27; break;
          case '-': n = 28; break;
          case '*': n = 29; break;
          case '$': n = 30; break;
          default: n = 26;
        }
      }
      write_int(n, 5);
    }

    void
    fox_encoder_impl::write_station_name_long()
    {
      write_int(STATION_NAME_LONG, 32);
      for (int i = 0; i < 32; i++) {
        write_char5(long_name[i]);
      }
      write_int(EXTENSION_FM, 2);
    }

  } /* namespace nrsc5 */
} /* namespace gr */
