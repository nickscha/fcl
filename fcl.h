/* fcl.h - v0.1 - public domain data structures - nickscha 2025

A C89 standard compliant, single header, nostdlib (no C Standard Library) fast compression library (FCL).

LICENSE

  Placed in the public domain and also MIT licensed.
  See end of file for detailed license information.

*/
#ifndef FCL_H
#define FCL_H

/* #############################################################################
 * # COMPILER SETTINGS
 * #############################################################################
 */
/* Check if using C99 or later (inline is supported) */
#if __STDC_VERSION__ >= 199901L
#define FCL_INLINE inline
#define FCL_API extern
#elif defined(__GNUC__) || defined(__clang__)
#define FCL_INLINE __inline__
#define FCL_API static
#elif defined(_MSC_VER)
#define FCL_INLINE __inline
#define FCL_API static
#else
#define FCL_INLINE
#define FCL_API static
#endif

#define FCL_MIN_MATCH 4
#define FCL_HASH_LOG 12
#define FCL_HASH_SIZE (1 << FCL_HASH_LOG)
#define FCL_MAX_DISTANCE 65535
#define FCL_MAX_LITERAL 15
#define FCL_MAX_MATCH 19 /* 15 (token) + FCL_MIN_MATCH */

#define FCL_READ_U32(p)                                       \
  ((unsigned long)((p)[0]) | ((unsigned long)((p)[1]) << 8) | \
   ((unsigned long)((p)[2]) << 16) | ((unsigned long)((p)[3]) << 24))

#define FCL_WRITE_U16(p, v)             \
  do                                    \
  {                                     \
    (p)[0] = (unsigned char)(v);        \
    (p)[1] = (unsigned char)((v) >> 8); \
  } while (0)

#define FCL_COPY4(dst, src) \
  do                        \
  {                         \
    (dst)[0] = (src)[0];    \
    (dst)[1] = (src)[1];    \
    (dst)[2] = (src)[2];    \
    (dst)[3] = (src)[3];    \
  } while (0)

#define FCL_COPY_BYTES(dst, src, count)  \
  do                                     \
  {                                      \
    unsigned long _i = 0;                \
    while (_i + 4 <= (count))            \
    {                                    \
      FCL_COPY4((dst) + _i, (src) + _i); \
      _i += 4;                           \
    }                                    \
    while (_i < (count))                 \
    {                                    \
      (dst)[_i] = (src)[_i];             \
      ++_i;                              \
    }                                    \
  } while (0)

FCL_API FCL_INLINE unsigned long fcl_hash(unsigned long seq)
{
  return ((seq * 2654435761UL) >> (32 - FCL_HASH_LOG)) & (FCL_HASH_SIZE - 1);
}

FCL_API FCL_INLINE int fcl_compress(
    unsigned char *input,
    unsigned long input_size,
    unsigned char *output,
    unsigned long *output_size,
    unsigned long output_capacity)
{
  unsigned short hash_table[FCL_HASH_SIZE];
  unsigned long ip = 0, anchor = 0, op = 0;
  unsigned long i;
  unsigned long literal_len;

  for (i = 0; i < FCL_HASH_SIZE; ++i)
  {
    hash_table[i] = 0;
  }

  while (ip + FCL_MIN_MATCH < input_size)
  {
    unsigned long seq = FCL_READ_U32(&input[ip]);
    unsigned long hash = fcl_hash(seq);
    unsigned long ref = hash_table[hash];

    hash_table[hash] = (unsigned short)ip;

    if (ref < ip &&
        (ip - ref) <= FCL_MAX_DISTANCE &&
        FCL_READ_U32(&input[ref]) == seq)
    {
      unsigned long literal_len = ip - anchor;
      unsigned long match = ref;
      unsigned long match_len = FCL_MIN_MATCH;
      unsigned char token;
      unsigned char lit;
      unsigned char mat;

      while ((ip + match_len < input_size) && (input[match + match_len] == input[ip + match_len]))
      {
        ++match_len;

        if (match_len == FCL_MAX_MATCH)
        {
          break;
        }
      }

      if (op + 1 + literal_len + 2 > output_capacity)
      {
        return 1;
      }

      lit = (literal_len < FCL_MAX_LITERAL) ? (unsigned char)literal_len : FCL_MAX_LITERAL;
      mat = ((match_len - FCL_MIN_MATCH) < FCL_MAX_LITERAL) ? (unsigned char)(match_len - FCL_MIN_MATCH) : FCL_MAX_LITERAL;
      token = (unsigned char)((lit << 4) | mat);

      output[op++] = token;

        (&output[op], &input[anchor], literal_len);
      op += literal_len;

      FCL_WRITE_U16(&output[op], ip - match);
      op += 2;

      ip += match_len;
      anchor = ip;
    }
    else
    {
      ip++;
    }
  }

  literal_len = input_size - anchor;

  if (op + 1 + literal_len > output_capacity)
  {
    return 1;
  }

  output[op++] = (unsigned char)(literal_len << 4);

  FCL_COPY_BYTES(&output[op], &input[anchor], literal_len);
  op += literal_len;

  *output_size = op;

  return 0;
}

FCL_API FCL_INLINE int fcl_decompress(
    unsigned char *input,
    unsigned long input_size,
    unsigned char *output,
    unsigned long *output_size,
    unsigned long output_capacity)
{
  unsigned long ip = 0, op = 0;

  while (ip < input_size)
  {
    unsigned char token = input[ip++];
    unsigned long literal_len = token >> 4;
    unsigned long match_len = (token & 0x0F) + FCL_MIN_MATCH;
    unsigned long i;
    unsigned long offset;
    unsigned char *src;
    unsigned char *dst;

    if (ip + literal_len > input_size || op + literal_len > output_capacity)
    {
      return 1;
    }

    FCL_COPY_BYTES(&output[op], &input[ip], literal_len);
    ip += literal_len;
    op += literal_len;

    if (ip >= input_size)
    {
      break;
    }

    if (ip + 2 > input_size)
    {
      return 1;
    }

    offset = (unsigned long)(input[ip] | (input[ip + 1] << 8));
    ip += 2;

    if (offset == 0 || offset > op || op + match_len > output_capacity)
    {
      return 1;
    }

    dst = &output[op];
    src = &output[op - offset];

    if (offset >= match_len)
    {
      FCL_COPY_BYTES(dst, src, match_len);
    }
    else
    {
      for (i = 0; i < match_len; ++i)
      {
        dst[i] = src[i];
      }
    }

    op += match_len;
  }

  *output_size = op;

  return 0;
}

#endif /* FCL_H */

/*
   ------------------------------------------------------------------------------
   This software is available under 2 licenses -- choose whichever you prefer.
   ------------------------------------------------------------------------------
   ALTERNATIVE A - MIT License
   Copyright (c) 2025 nickscha
   Permission is hereby granted, free of charge, to any person obtaining a copy of
   this software and associated documentation files (the "Software"), to deal in
   the Software without restriction, including without limitation the rights to
   use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies
   of the Software, and to permit persons to whom the Software is furnished to do
   so, subject to the following conditions:
   The above copyright notice and this permission notice shall be included in all
   copies or substantial portions of the Software.
   THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
   IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
   FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
   AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
   LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
   OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
   SOFTWARE.
   ------------------------------------------------------------------------------
   ALTERNATIVE B - Public Domain (www.unlicense.org)
   This is free and unencumbered software released into the public domain.
   Anyone is free to copy, modify, publish, use, compile, sell, or distribute this
   software, either in source code form or as a compiled binary, for any purpose,
   commercial or non-commercial, and by any means.
   In jurisdictions that recognize copyright laws, the author or authors of this
   software dedicate any and all copyright interest in the software to the public
   domain. We make this dedication for the benefit of the public at large and to
   the detriment of our heirs and successors. We intend this dedication to be an
   overt act of relinquishment in perpetuity of all present and future rights to
   this software under copyright law.
   THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
   IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
   FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
   AUTHORS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN
   ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION
   WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
   ------------------------------------------------------------------------------
*/
