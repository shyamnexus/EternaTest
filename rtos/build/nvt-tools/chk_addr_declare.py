#!/usr/bin/env python3
# -*- coding: utf-8 -*-

import sys
import os
import re
import subprocess
import argparse
import colorama
import logging
from datetime import datetime

ban_list = [
    r"hd_videocapture.h",
    r"shm_info.h",
    r"rtos_",
    r"bridge.c",
    r"vdoenc_builtin.c",
    r"nvt_vdocdc/",
    r"iq_builtin.c",
    r"sensor_builtin.h",
    r"isp_builtin.c",
    r"isp_builtin.h",
    r"sensor_builtin.c",
    r"kdrv_ai",
    r"ipp/",
    r"dsi_int.c",
    r"display_int.h",
    r"disp2.c",
    r"disp1.c",
    r"tse.c",
]

def system_call(command, get_stdout=True, show_er=True):
    if get_stdout:
        process = subprocess.Popen(command, stdout=subprocess.PIPE, shell=True)
        stdout = process.communicate()[0].decode(
            "ascii", errors="ignore").splitlines()
        er = process.returncode
    else:
        er = os.system(command)
        stdout = ""
    if er != 0:
        logging.error("failed to {}, er={}".format(command, er))
    return er, stdout

def parse_args(argv):
    parser = argparse.ArgumentParser(description='configs redirect symbolic')
    parser.add_argument('-f', '--folder', required=True,
                        dest='folder', action='store',
                        help='folder to hdal or include dir')
    args = parser.parse_args()
    return args

class ConsoleFormatter(logging.Formatter):
    # MSG_FMT = "%(class)s%(asctime)s: %(message)s\033[0m"
    MSG_FMT = "%(class)s%(message)s\033[0m"
    CSS_CLASSES = {'WARNING': '\033[1;93m',
                   'INFO': '\033[0m',
                   'DEBUG': '\033[0m',
                   'CRITICAL': '\033[1;91m',
                   'ERROR': '\033[1;91m'}

    def __init__(self):
        super().__init__()

    def format(self, record):
        try:
            class_name = self.CSS_CLASSES[record.levelname]
        except KeyError:
            class_name = "info"
        tm_now = datetime.now()
        asctime = "{:02d}:{:02d}".format(
            tm_now.hour,
            tm_now.minute)
        message = record.msg
        return self.MSG_FMT % {
            "class": class_name,
            "message": message,
            "asctime": asctime
        }


def initiate():
    colorama.init()
    # set log file
    rootLogger = logging.getLogger()
    consoleHandler = logging.StreamHandler()
    consoleHandler.setFormatter(ConsoleFormatter())
    rootLogger.addHandler(consoleHandler)
    rootLogger.setLevel(logging.DEBUG)
    return 0

def is_ban(filepath):
    for file in ban_list:
        if file in filepath:
            return 1
    return 0

def chk(filepath):
    re_addr = re.compile(r'(\w*)\s*(\w*addr\w*|\w*Addr\w*|\w*ADDR\w*|\w*ptr\w*|\w*buf\w*)')
    #re_addr = re.compile(r'(\w*)\s*(buf\w*)')
    re_exclude_buf = re.compile(r'(size|Size|len|Len|method|id|height|ms|width|lineoffset|ofs|_w|_h|mode|opt|num|_index|_ctrl|ctl|cnt|count|Fmt|fmt|attr|sz|info|check)')
    re_type = re.compile(r'(^int$|^UINT32$|^u32$)')

    invalids = []
    line_num = 0
    with open(filepath, 'rt',encoding="Big5", errors='ignore') as fin:
        line = fin.readline()
        line_num += 1
        while(line):
            match = re.search(re_addr, line)
            if not match:
                line = fin.readline()
                line_num += 1
                continue
            declare_type = match.group(1)
            name = match.group(2)

            match_exclude_buf = re.search(re_exclude_buf, name)
            if match_exclude_buf:
                line = fin.readline()
                line_num += 1
                continue

            match_type = re.search(re_type, declare_type)
            if match_type:
                invalids.append("{:04d}: {}".format(line_num, line))
                #logging.info("{}, {}".format(declare_type, line))
            line = fin.readline()
            line_num += 1
    if len(invalids):
        print(filepath)
        for invalid in invalids:
            print(invalid, end="")
        print("\n\n")

    return 0

def main(argv):
    if 0:
        argv.append(r"-f")
        argv.append(r"/home/ubuntu/firmware/na51102_dual_sdk/na51102_linux_sdk/base/hdal")

    initiate()
    args = parse_args(argv)

    if os.path.isdir(args.folder):
        for root, dirs, files in os.walk(args.folder):
            for file in files:
                main_name, ext_name = os.path.splitext(file)
                if ext_name != ".c" and ext_name != ".h":
                    continue
                fullpath = os.path.join(root, file)
                if not is_ban(fullpath):
                    chk(fullpath)
    else:
        filepath = args.folder
        if not is_ban(filepath):
            chk(filepath)
    return 0



if __name__ == '__main__':
    try:
        er = main(sys.argv)
    except Exception as exp:
        er = -1
        print(exp, file=sys.stderr)
        raise exp
    sys.exit(er)
