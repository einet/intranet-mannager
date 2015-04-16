#include <stdlib.h>
#include <boost/regex.hpp>
#include <string>
#include <iostream>

using namespace boost;
std::string sendcmdexp;

bool process_sendcmd(const std::string& msg)
{
   cmatch what;
   //"^(ls|yum|top|wget|cat|find|tail|date|hostname|free|df|du)( )*.*|^sysctl( )*vm\\.drop_caches=1|^killall( )*(find|tail|cat|Waiter|du)( )*|^set( )*.*(host( )*.*)|^\\.\\/Waiter( )*.*$"

   regex expression(sendcmdexp);
   if(regex_match(msg.c_str(), what, expression))
   {
     return true;
   }
   return false;
}

