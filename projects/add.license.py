#! /usr/bin/env python
# -*- coding: utf-8 -*-
#
# =============================================
#      Author   : andyhuzhill
#    E-mail     : andyhuzhill@gmail.com
#
#  Description  :
#  Revision     :
#
# =============================================

import os
import re
import tempfile

license = ''

def add_license(path):
    if os.path.exists(path):
        old_file = open(path, 'r')

        lines = old_file.read()

        old_file.seek(0)
        old_file.write(license)
        old_file.write(lines)        
  
        old_file.close()


def list_dir_files(rootDir):
    for lists in os.listdir(rootDir):
        path = os.path.join(rootDir, lists)
        if os.path.isfile(path):
            if re.search('.cpp', path.lower()) or re.search('.h', path.lower()) or re.search('.c', path.lower()):
                add_license(path)
        if os.path.isdir(path):
            list_dir_files(path)


if __name__ == "__main__":
    import sys
    if len(sys.argv) != 2:
        print "Usage: ./add_license.py [dir]"
        sys.exit(-1)
    list_dir_files(sys.argv[1])