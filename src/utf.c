//
// "$Id: utf.c,v 1.13 2004/08/01 22:28:24 spitzak Exp $"
//
// Copyright 2004 by Bill Spitzak and others.
//
// Unlike the rest of FLTK, these UTF-8 functions are explicitly
// released into the public domain, with no restrictions on copying
// or reuse for any purpose, in open or closed-source software.
// These functions are completely original, written by Bill Spitzak
// from the UTF-8 RFC documents, and have no legal encumberances.
//
// This library is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
// Library General Public License for more details.
//
// You should have received a copy of the GNU Library General Public
// License along with this library; if not, write to the Free Software
// Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307
// USA.
//
// Please report all bugs and problems to "fltk-bugs@fltk.org".
//

// Modified to obey rfc3629, which limits unicode to 0-0x10ffff

#include <fltk/utf.h>
#include <string.h>
#include <stdlib.h>

#ifdef __cplusplus
extern "C" {
#endif

// Codes 0x80..0x9f from the Microsoft CP1252 character set, translated
// to Unicode:
static unsigned short cp1252[32] = {
  0x20ac, 0x0081, 0x201a, 0x0192, 0x201e, 0x2026, 0x2020, 0x2021,
  0x02c6, 0x2030, 0x0160, 0x2039, 0x0152, 0x008d, 0x017d, 0x008f,
  0x0090, 0x2018, 0x2019, 0x201c, 0x201d, 0x2022, 0x2013, 0x2014,
  0x02dc, 0x2122, 0x0161, 0x203a, 0x0153, 0x009d, 0x017e, 0x0178
};

/*! Decode a single UTF-8 encoded character starting at \e p. The
    resulting Unicode value (in the range 0-0x10ffff) is returned,
    and \e len is set the the number of bytes in the UTF-8 encoding
    (adding \e len to \e p will point at the next character).

    If \a p points at an illegal UTF-8 encoding, including one that
    would go past \e end, or where a code is uses more bytes than
    necessary, then *(unsigned char*)p is translated as though it is
    in the Microsoft CP1252 character set and \e len is set to 1.
    Treating errors this way allows this to decode almost any
    ISO-8859-1 or CP1252 text that has been mistakenly placed where
    UTF-8 is expected, and has proven very useful.

    To distinguish the error result from the legal 1-byte UTF-8
    encodings you must also check *p&0x80.

    Calling code can be speeded up by checking the high bit and
    directly treating the common 1-byte case:

\code
    if (!(*p&0x80)) {
      code = *p;
      len = 1;
    } else {
      code = utf8decode(p,end,&len);
    }
\endcode

    If you want to test if \a p points at the start of a legal utf-8
    character, the following code will work:

\code
    if (!(*p&0x80)) {
      legal = true;
    } else {
      int len; utf8decode(p,end,&len);
      legal = len > 1;
    }
\endcode

    It is also useful to know that this will always set \a len to 1
    if *p is not in the range 0xc2 through 0xf4.
*/
unsigned utf8decode(const char* p, const char* end, int* len)
{
  unsigned char c = *(unsigned char*)p;
  if (c < 0x80) {
    *len = 1;
    return c;
  } else if (c < 0xa0) {
    *len = 1;
    return cp1252[c-0x80];
  } else if (c < 0xc2) {
    *len = 1;
    return c;
  }
  if (p+1 >= end || (p[1]&0xc0) != 0x80) goto FAIL;
  if (c < 0xe0) {
    *len = 2;
    return
      ((p[0] & 0x1f) << 6) +
      ((p[1] & 0x3f));
  } else if (c == 0xe0) {
    if (((unsigned char*)p)[1] < 0xa0) goto FAIL;
    goto UTF8_3;
#if 0
  } else if (c == 0xed) {
    // RFC 3629 says surrogate chars are illegal.
    // I don't check this so that all 16-bit values are preserved
    // when going through utf8encode/utf8decode.
    if (((unsigned char*)p)[1] >= 0xa0) goto FAIL;
    goto UTF8_3;
  } else if (c == 0xef) {
    // 0xfffe and 0xffff are also illegal characters
    // Again I don't check this so 16-bit values are preserved
    if (((unsigned char*)p)[1]==0xbf &&
	((unsigned char*)p)[2]>=0xbe) goto FAIL;
    goto UTF8_3;
#endif
  } else if (c < 0xf0) {
  UTF8_3:
    if (p+2 >= end || (p[2]&0xc0) != 0x80) goto FAIL;
    *len = 3;
    return
      ((p[0] & 0x0f) << 12) +
      ((p[1] & 0x3f) << 6) +
      ((p[2] & 0x3f));
  } else if (c == 0xf0) {
    if (((unsigned char*)p)[1] < 0x90) goto FAIL;
    goto UTF8_4;
  } else if (c < 0xf4) {
  UTF8_4:
    if (p+3 >= end || (p[2]&0xc0) != 0x80 || (p[3]&0xc0) != 0x80) goto FAIL;
    *len = 4;
#if 0
    // All codes ending in fffe or ffff are illegal:
    if ((p[1]&0xf)==0xf &&
	((unsigned char*)p)[2] == 0xbf &&
	((unsigned char*)p)[3] >= 0xbe) goto FAIL;
#endif
    return
      ((p[0] & 0x07) << 18) +
      ((p[1] & 0x3f) << 12) +
      ((p[2] & 0x3f) << 6) +
      ((p[3] & 0x3f));
  } else if (c == 0xf4) {
    if (((unsigned char*)p)[1] > 0x8f) goto FAIL; // after 0x10ffff
    goto UTF8_4;
  } else {
  FAIL:
    *len = 1;
    return c;
  }
}

/*! Return the length of a legal UTF-8 encoding that starts with
    this byte. Returns 1 for illegal bytes (0xc0, 0xc1, 0xf5 to 0xff).

    <i>This function is depreciated</i>. If the following bytes are
    not legal UTF-8 then using this to step forward to the next
    character will produce different positions than utf8decode()
    will produce.
*/
int utf8len(char cc) {
  unsigned char c = (unsigned char)cc;
  if (c < 0xc2) return 1;
  else if (c < 0xe0) return 2;
  else if (c < 0xf0) return 3;
  else if (c < 0xf5) return 4;
  else return 1;
}

/*! If p points into (not at) a utf-8 character, return
  a pointer to after the character. Otherwise return p. This will
  move p to a point that is at the start of a glyph.

  \e end is the end of the string and is assummed to be a break
  between characters. It is assummed to be greater than p.

  \e start is the start of the string and is used to limit the
  backwards search for the start of a utf8 character.

  If you wish to increment a random pointer into a utf8 string, pass
  p+1 to this. If you wish to step through a string starting at a
  known legal starting point you can do this somewhat faster code:

  \code
  for (const char* p = start; p < end;) {
    if (*(unsigned char*)p < 0xc2) {
      // fast handler for single-byte utf-8 character, or an error byte
      thecode = *(unsigned char*)p;
      ++p;
    } else {
      int len;
      thecode = utf8decode(p, end, &len);
      p += len;
    }
  }
  \endcode
*/
const char* utf8fwd(const char* p, const char* start, const char* end)
{
  const char* a;
  int len;
  // if we are not pointing at a continuation character, we are done:
  if ((*p&0xc0) != 0x80) return p;
  // search backwards for a 0xc0 starting the character:
  for (a = p-1; ; --a) {
    if (a < start) return p;
    if (!(a[0]&0x80)) return p;
    if ((a[0]&0x40)) break;
  }
  utf8decode(a,end,&len);
  a += len;
  if (a > p) return a;
  return p;
}

/*! If p points into (not at) a legal UTF-8 character, return a
  pointer to the character (a value less than p). Otherwise return
  p. This will move p to a point that is the start of a character.

  If you wish to decrement a UTF-8 pointer, pass p-1 to this.

  \e end is the end of the string and is assummed to be a break
  between characters. It is assummed to be greater than p.

  \e start is the start of the string and is used to limit the
  backwards search for the start of a UTF-8 character.
*/
const char* utf8back(const char* p, const char* start, const char* end)
{
  const char* a;
  int len;
  // if we are not pointing at a continuation character, we are done:
  if ((*p&0xc0) != 0x80) return p;
  // search backwards for a 0xc0 starting the character:
  for (a = p-1; ; --a) {
    if (a < start) return p;
    if (!(a[0]&0x80)) return p;
    if ((a[0]&0x40)) break;
  }
  utf8decode(a,end,&len);
  if (a+len > p) return a;
  return p;
}

/*! Returns number of bytes that utf8encode() will use to encode the
  Unicode point \a ucs. */
int utf8bytes(unsigned ucs) {
  if (ucs < 0x000080U) {
    return 1;
  } else if (ucs < 0x000800U) {
    return 2;
  } else if (ucs < 0x010000U) {
    return 3;
  } else if (ucs < 0x10ffffU) {
    return 4;
  } else {
    return 3; // length of the illegal character encoding
  }
}

/*! Write the UTF-8 encoding of \e ucs into \e buf and return the
    number of bytes written. Up to 4 bytes may be written. If you know
    that \a ucs is less than 0x10000 then at most 3 bytes will be written.
    If you wish to speed this up, remember that anything less than 0x80
    is written as a single byte.

    If ucs is greater than 0x10ffff this is an illegal character
    according to RFC 3629. These are converted as though they are
    0xFFFD (REPLACEMENT CHARACTER).

    \a ucs in the range 0xd800 to 0xdfff, or ending with 0xfffe or
    0xffff are also illegal according to RFC 3629. However I encode
    these as though they are legal, so that utf8encode/utf8decode will
    be the identity for all codes between 0 and 0x10ffff.
*/
int utf8encode(unsigned ucs, char* buf) {
  if (ucs < 0x000080U) {
    buf[0] = ucs;
    return 1;
  } else if (ucs < 0x000800U) {
    buf[0] = 0xc0 | (ucs >> 6);
    buf[1] = 0x80 | (ucs & 0x3F);
    return 2;
  } else if (ucs < 0x010000U) {
    buf[0] = 0xe0 | (ucs >> 12);
    buf[1] = 0x80 | ((ucs >> 6) & 0x3F);
    buf[2] = 0x80 | (ucs & 0x3F);
    return 3;
  } else if (ucs < 0x0010ffffU) {
    buf[0] = 0xf0 | (ucs >> 18);
    buf[1] = 0x80 | ((ucs >> 12) & 0x3F);
    buf[2] = 0x80 | ((ucs >> 6) & 0x3F);
    buf[3] = 0x80 | (ucs & 0x3F);
    return 4;
  } else {
    // encode 0xfffd:
    buf[0] = 0xefU;
    buf[1] = 0xbfU;
    buf[2] = 0xbdU;
    return 3;
  }
}

/*! Convert a UTF-8 sequence into an array of "wide characters", which
    are used by some system calls, especially on Windows.

    \a src points at the UTF-8, and \a srclen is the number of bytes to
    convert.

    \a dst points at an array to write, and \a dstlen is the number of
    locations in this array. At most \a dstlen-1 words will be
    written there, plus a 0 terminating word. Thus this function
    will never overwrite the buffer and will always return a
    zero-terminated string. If \a dstlen is zero then \a dst can be
    null and no data is written, but the length is returned.

    The return value is the number of words that \e would be written
    to \a dst if it were long enough, not counting the terminating
    zero. If the return value is greater or equal to \a dstlen it
    indicates truncation, you can then allocate a new array of size
    return+1 and call this again.

    Errors in the UTF-8 are converted as though each byte in the
    erroneous string is in the Microsoft CP1252 encoding. This allows
    ISO-8859-1 text mistakenly identified as UTF-8 to be printed
    correctly.

    On Unix one Unicode character is put into each location in the
    output array. On Windows, where wchar_t is 16 bits, Unicode
    characers in the range 0x10000 to 0x10ffff are converted to
    "surrogate pairs" which take two words each (this is called UTF-16
    encodign). Because of this incompatability it is strongly
    recommended you use wchar_t only when absolutely necessary for
    passing values to the operating system. Store internal values in
    UTF-8.  */
unsigned utf8towc(const char* src, unsigned srclen,
		  wchar_t* dst, unsigned dstlen)
{		  
  const char* p = src;
  const char* e = src+srclen;
  unsigned count = 0;
  if (dstlen) for (;;) {
    if (p >= e) {dst[count] = 0; return count;}
    if (!(*p & 0x80)) { // ascii
      dst[count] = *p++;
    } else {
      int len; unsigned ucs = utf8decode(p,e,&len);
      p += len;
#ifdef _WIN32
      if (ucs < 0x10000) {
	dst[count] = ucs;
      } else {
	// make a surrogate pair:
	if (count+2 >= dstlen) {dst[count] = 0; count += 2; break;}
	dst[count] = (((ucs-0x10000u)>>10)&0x3ff) | 0xd800;
	dst[++count] = (ucs&0x3ff) | 0xdc00;
      }
#else
      dst[count] = ucs;
#endif
    }
    if (++count == dstlen) {dst[count-1] = 0; break;}
  }
  // we filled dst, measure the rest:
  while (p < e) {
    if (!(*p & 0x80)) p++;
    else {
#ifdef _WIN32
      int len; unsigned ucs = utf8decode(p,e,&len);
      p += len;
      if (ucs >= 0x10000) ++count;
#else
      int len; utf8decode(p,e,&len);
      p += len;
#endif
    }
    ++count;
  }
  return count;
}

/*! Convert a UTF-8 sequence into an array of 1-byte characters.

    If the UTF-8 decodes to a character greater than 0xff then it is
    replaced with '?'.

    Errors in the UTF-8 are converted as individual bytes, same as
    utf8decode() does. This allows ISO-8859-1 text mistakenly identified
    as UTF-8 to be printed correctly (and possibly CP1512 on Windows).

    \a src points at the UTF-8, and \a srclen is the number of bytes to
    convert.

    Up to \a dstlen bytes are written to \a dst, including a null
    terminator. The return value is the number of bytes that would be
    written, not counting the null terminator. If greater or equal to
    \a dstlen then if you malloc a new array of size n+1 you will have
    the space needed for the entire string. If \a dstlen is zero then
    nothing is written and this call just measures the storage space
    needed.
*/
unsigned utf8toa(const char* src, unsigned srclen,
		 char* dst, unsigned dstlen)
{
  const char* p = src;
  const char* e = src+srclen;
  unsigned count = 0;
  if (dstlen) for (;;) {
    unsigned char c;
    if (p >= e) {dst[count] = 0; return count;}
    c = *(unsigned char*)p;
    if (c < 0xC2) { // ascii or bad code
      dst[count] = c;
      p++;
    } else {
      int len; unsigned ucs = utf8decode(p,e,&len);
      p += len;
      if (ucs < 0x100) dst[count] = ucs;
      else dst[count] = '?';
    }
    if (++count >= dstlen) {dst[count-1] = 0; break;}
  }	
  // we filled dst, measure the rest:
  while (p < e) {
    if (!(*p & 0x80)) p++;
    else {
      int len;
      utf8decode(p,e,&len);
      p += len;
    }
    ++count;
  }
  return count;
}

/*! Turn "wide characters" as returned by some system calls
    (especially on Windows) into UTF-8.

    Up to \a dstlen bytes are written to \a dst, including a null
    terminator. The return value is the number of bytes that would be
    written, not counting the null terminator. If greater or equal to
    \a dstlen then if you malloc a new array of size n+1 you will have
    the space needed for the entire string. If \a dstlen is zero then
    nothing is written and this call just measures the storage space
    needed.

    \a srclen is the number of words in \a src to convert. On Windows
    this is not necessairly the number of characters, due to there
    possibly being "surrogate pairs" in the UTF-16 encoding used.
    On Unix wchar_t is 32 bits and each location is a character.

    On Unix if a src word is greater than 0x10ffff then this is an
    illegal character according to RFC 3629. These are converted as
    though they are 0xFFFD (REPLACEMENT CHARACTER). Characters in the
    range 0xd800 to 0xdfff, or ending with 0xfffe or 0xffff are also
    illegal according to RFC 3629. However I encode these as though
    they are legal, so that utf8towc will return the original data.

    On Windows "surrogate pairs" are converted to a single character
    and UTF-8 encoded (as 4 bytes). Mismatched halves of surrogate
    pairs are converted as though they are individual characters.
*/
unsigned utf8fromwc(char* dst, unsigned dstlen,
		    const wchar_t* src, unsigned srclen) {
  unsigned i = 0;
  unsigned count = 0;
  if (dstlen) for (;;) {
    unsigned ucs;
    if (i >= srclen) {dst[count] = 0; return count;}
    ucs = src[i++];
    if (ucs < 0x80U) {
      dst[count++] = ucs;
      if (count >= dstlen) {dst[count-1] = 0; break;}
    } else if (ucs < 0x800U) { // 2 bytes
      if (count+2 >= dstlen) {dst[count] = 0; count += 2; break;}
      dst[count++] = 0xc0 | (ucs >> 6);
      dst[count++] = 0x80 | (ucs & 0x3F);
#ifdef _WIN32
    } else if (ucs >= 0xd800 && ucs <= 0xdbff && i < srclen &&
	       src[i] >= 0xdc00 && src[i] <= 0xdfff) {
      // surrogate pair
      unsigned ucs2 = src[i++];
      ucs = 0x10000U + ((ucs&0x3ff)<<10) + (ucs2&0x3ff);
      // all surrogate pairs turn into 4-byte utf8
#else
    } else if (ucs >= 0x10000) {
      if (ucs > 0x10ffff) {
	ucs = 0xfffd;
	goto J1;
      }
#endif
      if (count+4 >= dstlen) {dst[count] = 0; count += 4; break;}
      dst[count++] = 0xf0 | (ucs >> 18);
      dst[count++] = 0x80 | ((ucs >> 12) & 0x3F);
      dst[count++] = 0x80 | ((ucs >> 6) & 0x3F);
      dst[count++] = 0x80 | (ucs & 0x3F);
    } else {
#ifndef _WIN32
    J1:
#endif
      // all others are 3 bytes:
      if (count+3 >= dstlen) {dst[count] = 0; count += 3; break;}
      dst[count++] = 0xe0 | (ucs >> 12);
      dst[count++] = 0x80 | ((ucs >> 6) & 0x3F);
      dst[count++] = 0x80 | (ucs & 0x3F);
    }
  }	
  // we filled dst, measure the rest:
  while (i < srclen) {
    unsigned ucs = src[i++];
    if (ucs < 0x80U) {
      count++;
    } else if (ucs < 0x800U) { // 2 bytes
      count += 2;
#ifdef _WIN32
    } else if (ucs >= 0xd800 && ucs <= 0xdbff && i < srclen-1 &&
	       src[i+1] >= 0xdc00 && src[i+1] <= 0xdfff) {
      // surrogate pair
      ++i;
#else
    } else if (ucs >= 0x10000 && ucs <= 0x10ffff) {
#endif
      count += 4;
    } else {
      count += 3;
    }
  }
  return count;
}

/*! Convert an ISO-8859-1 (ie normal c-string) byte stream to UTF-8.

    It is possible this should convert Microsoft's CP1252 to UTF-8
    instead. This would translate the codes in the range 0x80-0x9f
    to different characters. Currently it does not do this.

    Up to \a dstlen bytes are written to \a dst, including a null
    terminator. The return value is the number of bytes that would be
    written, not counting the null terminator. If greater or equal to
    \a dstlen then if you malloc a new array of size n+1 you will have
    the space needed for the entire string. If \a dstlen is zero then
    nothing is written and this call just measures the storage space
    needed.

    \a srclen is the number of bytes in \a src to convert.

    If the return value equals \a srclen then this indicates that
    no conversion is necessary, as only ASCII characters are in the
    string.
*/
unsigned utf8froma(char* dst, unsigned dstlen,
		   const char* src, unsigned srclen) {
  const char* p = src;
  const char* e = src+srclen;
  unsigned count = 0;
  if (dstlen) for (;;) {
    unsigned char ucs;
    if (p >= e) {dst[count] = 0; return count;}
    ucs = *(unsigned char*)p++;
    if (ucs < 0x80U) {
      dst[count++] = ucs;
      if (count >= dstlen) {dst[count-1] = 0; break;}
    } else { // 2 bytes (note that CP1252 translate could make 3 bytes!)
      if (count+2 >= dstlen) {dst[count] = 0; count += 2; break;}
      dst[count++] = 0xc0 | (ucs >> 6);
      dst[count++] = 0x80 | (ucs & 0x3F);
    }
  }	
  // we filled dst, measure the rest:
  while (p < e) {
    unsigned char ucs = *(unsigned char*)p++;
    if (ucs < 0x80U) {
      count++;
    } else {
      count += 2;
    }
  }
  return count;
}

#ifdef _WIN32
# include <windows.h>
#endif

/*! Return true if the "locale" seems to indicate that UTF-8 encoding
    is used. If true the utf8tomb and utf8frommb don't do anything
    useful.

    <i>It is highly recommended that you change your system so this
    does return true.</i> On Windows this is done by setting the
    "codepage" to CP_UTF8.  On Unix this is done by setting $LC_CTYPE
    to a string containing the letters "utf" or "UTF" in it, or by
    deleting all $LC* and $LANG environment variables. In the future
    it is likely that all non-Asian Unix systems will return true,
    due to the compatability of UTF-8 with ISO-8859-1.
*/
int utf8locale() {
  static int ret = 2;
  if (ret == 2) {
#ifdef _WIN32
    ret = GetACP() == CP_UTF8;
#else
    char* s;
    ret = 1; // assumme UTF-8 if no locale
    if (((s = getenv("LC_CTYPE")) && *s) ||
	((s = getenv("LC_ALL"))   && *s) ||
	((s = getenv("LANG"))     && *s)) {
      ret = (strstr(s,"utf") || strstr(s,"UTF"));
    }
#endif
  }
  return ret;
}

/*! Convert the UTF-8 used by FLTK to the locale-specific encoding
    used for filenames (and sometimes used for data in files).
    Unfortunatley due to stupid design you will have to do this as
    needed for filenames. This is a bug on both Unix and Windows.

    Up to \a dstlen bytes are written to \a dst, including a null
    terminator. The return value is the number of bytes that would be
    written, not counting the null terminator. If greater or equal to
    \a dstlen then if you malloc a new array of size n+1 you will have
    the space needed for the entire string. If \a dstlen is zero then
    nothing is written and this call just measures the storage space
    needed.

    If utf8locale() returns true then this does not change the data.
    It is copied and truncated as necessary to
    the destination buffer and \a srclen is always returned.  */
unsigned utf8tomb(const char* src, unsigned srclen,
		  char* dst, unsigned dstlen)
{
  if (!utf8locale()) {
#ifdef _WIN32
    unsigned short lbuf[1024];
    wchar_t* buf = lbuf;
    unsigned length = utf8towc(src, srclen, buf, 1024);
    unsigned ret;
    if (length >= 1024) {
      buf = (wchar_t*)(malloc((length+1)*sizeof(wchar_t)));
      utf8towc(src, srclen, buf, length+1);
    }
    if (dstlen) {
      // apparently this does not null-terminate, even though msdn
      // documentation claims it does:
      ret =
        WideCharToMultiByte(GetACP(), 0, buf, length, dst, dstlen, 0, 0);
      dst[ret] = 0;
    }
    // if it overflows or measuring length, get the actual length:
    if (dstlen==0 || ret >= dstlen-1)
      ret =
	WideCharToMultiByte(GetACP(), 0, buf, length, 0, 0, 0, 0);
    if (buf != lbuf) free((void*)buf);
    return ret;
#else
    wchar_t lbuf[1024];
    wchar_t* buf = lbuf;
    unsigned length = utf8towc(src, srclen, buf, 1024);
    int ret;
    if (length >= 1024) {
      buf = (wchar_t*)(malloc((length+1)*sizeof(wchar_t)));
      utf8towc(src, srclen, buf, length+1);
    }
    if (dstlen) {
      ret = wcstombs(dst, buf, dstlen);
      if (ret >= dstlen-1) ret = wcstombs(0,buf,0);
    } else {
      ret = wcstombs(0,buf,0);
    }
    if (buf != lbuf) free((void*)buf);
    if (ret >= 0) return (unsigned)ret;
    // on any errors we return the UTF-8 as raw text...
#endif
  }
  // identity transform:
  if (srclen <= dstlen) {
    memcpy(dst, src, srclen);
    dst[srclen] = 0;
  } else {
    memcpy(dst, src, dstlen-1);
    dst[dstlen] = 0;
  }

  return srclen;
}
  
/*! Convert a filename from the locale-specific multibyte encoding
    used by Windows to UTF-8 as used by FLTK.

    Up to \a dstlen bytes are written to \a dst, including a null
    terminator. The return value is the number of bytes that would be
    written, not counting the null terminator. If greater or equal to
    \a dstlen then if you malloc a new array of size n+1 you will have
    the space needed for the entire string. If \a dstlen is zero then
    nothing is written and this call just measures the storage space
    needed.

    On Unix or on Windows when a UTF-8 locale is in effect, this
    does not change the data. It is copied and truncated as necessary to
    the destination buffer and \a srclen is always returned.
    You may also want to check if utf8test() returns non-zero, so that
    the filesystem can store filenames in UTF-8 encoding regardless of
    the locale.
*/
unsigned utf8frommb(char* dst, unsigned dstlen,
		    const char* src, unsigned srclen)
{
  if (!utf8locale()) {
#ifdef _WIN32
    wchar_t lbuf[1024];
    wchar_t* buf = lbuf;
    unsigned length;
    unsigned ret;
    length =
      MultiByteToWideChar(GetACP(), 0, src, srclen, buf, 1024);
    if (length >= 1024) {
      length = MultiByteToWideChar(GetACP(), 0, src, srclen, 0, 0);
      buf = (unsigned short*)(malloc(length*sizeof(unsigned short)));
      MultiByteToWideChar(GetACP(), 0, src, srclen, buf, length);
    }
    ret = utf8fromwc(dst, dstlen, buf, length);
    if (buf != lbuf) free((void*)buf);
    return ret;
#else
    wchar_t lbuf[1024];
    wchar_t* buf = lbuf;
    int length;
    unsigned ret;
    length = mbstowcs(buf, src, 1024);
    if (length >= 1024) {
      length = mbstowcs(0, src, 0)+1;
      buf = (wchar_t*)(malloc(length*sizeof(unsigned short)));
      mbstowcs(buf, src, length);
    }
    if (length >= 0) {
      ret = utf8fromwc(dst, dstlen, buf, length);
      if (buf != lbuf) free((void*)buf);
      return ret;
    }
    // errors in conversion return the UTF-8 unchanged
#endif
  }

  // identity transform:
  if (srclen <= dstlen) {
    memcpy(dst, src, srclen);
    dst[srclen] = 0;
  } else {
    memcpy(dst, src, dstlen-1);
    dst[dstlen] = 0;
  }
  return srclen;
}

/*! Examines the first \a srclen bytes in \a src and return a verdict
    on whether it is UTF-8 or not.

    - Returns 0 if there is any illegal UTF-8 sequences, using the
      same rules as utf8decode(). Note that some UCS values considered
      illegal by RFC 3629, such as 0xffff, are considered legal by this.

    - Returns 1 if there are only single-byte characters (ie no bytes
      have the high bit set). This is legal UTF-8, but also indicates
      plain ASCII. It also returns 1 if \a srclen is zero.

    - Returns 2 if there are only characters less than 0x800.

    - Returns 3 if there are only characters less than 0x10000.

    - Returns 4 if there are characters in the 0x10000 to 0x10ffff range.

    Because there are many illegal sequences in UTF-8, it is almost
    impossible for a string in another encoding to be confused with
    UTF-8. This is very useful for transitioning Unix to UTF-8
    filenames, you can simply test each filename with this to decide
    if it is UTF-8 or in the locale encoding. My hope is that if
    this is done we will be able to cleanly transition to a locale-less
    encoding.
*/
int utf8test(const char* src, unsigned srclen) {
  int ret = 1;
  const char* p = src;
  const char* e = src+srclen;
  while (p < e) {
    if (*p & 0x80) {
      int len; utf8decode(p,e,&len);
      if (len < 2) return 0;
      if (len > ret) ret = len;
      p += len;
    } else {
      p++;
    }
  }
  return ret;
}

#ifdef __cplusplus
}
#endif
