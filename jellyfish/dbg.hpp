/*  This file is part of Jellyfish.

    Jellyfish is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    Jellyfish is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with Jellyfish.  If not, see <http://www.gnu.org/licenses/>.
*/

#ifndef __DBG_HPP__
#define __DBG_HPP__

#include <iostream>
#include <iomanip>
#include <sstream>
#include <exception>
#include <stdexcept>
#include <sys/types.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <ctype.h>

namespace dbg {
  pid_t gettid();

  class stringbuf : public std::stringbuf {
  public:
    stringbuf() : std::stringbuf(std::ios_base::out) { }
    stringbuf(const std::string &str) : 
      std::stringbuf(str, std::ios_base::out) { }

    bool end_is_space() {
      if(pptr() == pbase())
        return true;
      return isspace(*(pptr() - 1));
    }
    friend class print_t;
  };

  class str {
    const char  *_s;
    const size_t _l;
  public:
    str(const char *s, size_t len) : _s(s), _l(len) {}
    friend class print_t;
  };

  class xspace { };

  class print_t {
    static pthread_mutex_t _lock;
    stringbuf              _strbuf;
    std::ostream           _buf;
  public:
    print_t(const char *file, const char *function, int line) :
      _buf(&_strbuf)
    {
      _buf << pthread_self() << "/" << gettid() << ":"
           << basename(file) << ":" << function << ":" << line << ": ";
    }

    ~print_t() {
      pthread_mutex_lock(&_lock);
      std::cerr.write(_strbuf.pbase(), _strbuf.pptr() - _strbuf.pbase());
      std::cerr << std::endl;
      pthread_mutex_unlock(&_lock);
    }

    print_t & operator<<(const char *a[]) {
      for(int i = 0; a[i]; i++)
        _buf << (i ? "\n" : "") << a[i];
      return *this;
    }
    print_t & operator<<(const std::exception &e) {
      _buf << e.what();
      return *this;
    }
    print_t & operator<<(const str &ss) {
      _buf.write(ss._s, ss._l);
      return *this;
    }
    print_t & operator<<(const xspace &xs) {
      if(!_strbuf.end_is_space())
        _buf << " ";
      return *this;
    }
    template<typename T>
    print_t & operator<<(const T &x) {
      _buf << x;
      return *this;
    }
  };

  class no_print_t {
  public:
    no_print_t() {}
    
    template<typename T>
    no_print_t & operator<<(const T &x) { return *this; }
  };
}

#ifdef DEBUG
#define DBG if(1) dbg::print_t(__FILE__, __FUNCTION__, __LINE__)
#define V(v) dbg::xspace() << #v ":" << v
#else
#define DBG if(1) dbg::no_print_t()
#define V(v) v
#endif

#endif /* __DBG_HPP__ */
