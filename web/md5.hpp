

#pragma once

#include "md5.h"
#include <sstream>
using namespace std;
namespace zmd5 {

    typedef unsigned char md5digest[16];

    inline void md5(const void *buf, int nbytes, md5digest digest) {
        md5_state_t st;
        md5_init(&st);
        md5_append(&st, (const md5_byte_t *) buf, nbytes);
        md5_finish(&st, digest);
    }

    inline void md5(const char *str, md5digest digest) {
        md5(str, strlen(str), digest);
    }
    
    inline std::string digestToString( md5digest digest ){
        static const char * letters = "0123456789abcdef";
        stringstream ss;
        for ( int i=0; i<16; i++){
            unsigned char c = digest[i];
            ss << letters[ ( c >> 4 ) & 0xf ] << letters[ c & 0xf ];
        }
        return ss.str().substr(0, 32);
    }

    inline std::string md5simpledigest( const void* buf, int nbytes){
        md5digest d;
        md5( buf, nbytes , d );
        return digestToString( d );
    }

    inline std::string md5simpledigest( const std::string& s ){
        return md5simpledigest(s.data(), s.size());
    }


} // namespace zmd5
