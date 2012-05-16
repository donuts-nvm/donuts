#!/usr/bin/env python

import re

rx = re.compile('#define __NR_(.*?)\s+(.*)')
ry = re.compile('\(__NR_(.*?)\+(\d+)\)')

print '''\
// Automatically generated by %s

#include "syscall_strings.h"

const char * syscall_string(int syscall_number)
{
   switch(syscall_number)
   {
''' % __file__

def make(bits):
  seen = set()
  names = {}
  for line in open('/usr/include/asm/unistd_%d.h' % bits, 'r').readlines():
    m = rx.match(line)
    if m:
      name, number = m.group(1), m.group(2)

      mm = ry.match(number)
      if mm:
        number = names[mm.group(1)] + int(mm.group(2))
      else:
        try:
          number = int(number.split()[0])
        except ValueError:
          continue

      if number not in seen:
        seen.add(number)
        names[name] = number
        print '      case %s: return "%s";' % (number, name)

print '#ifdef TARGET_IA32'
make(32)
print '#else'
make(64)
print '#endif'

print '''\
      default: return "(unknown)";
   }
}
'''
