#!/usr/bin/env python3
# -*- coding: utf-8 -*-

import sys
import os
import re
import subprocess
import argparse
import logging
from datetime import datetime

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
    parser = argparse.ArgumentParser(description='')
    parser.add_argument('-b', '--block_size', required=True,
                        dest='block_size', action='store', choices={"0x20000", "0x40000"},
                        help='nand block size by byte')
    parser.add_argument('-o', '--output_dir',
                        dest='output_dir', action='store',
                        help='output dir where are firmware existing')
    args = parser.parse_args()
    return args


class ConsoleFormatter(logging.Formatter):
    #MSG_FMT = "%(class)s%(asctime)s: %(message)s\033[0m"
    MSG_FMT = "%(class)s%(message)s\033[0m"
    CSS_CLASSES = {'WARNING': '\033[1;93m',
                   'INFO': '\033[0m',
                   'DEBUG': '\033[0m',
                   'CRITICAL': '\033[1;91m',
                   'ERROR': '\033[1;91merror: '}

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


class ConsoleFilter(logging.Filter):
    def filter(self, rec):
        return rec.levelno in (logging.DEBUG, logging.INFO)


def initiate():
    # set log file
    rootLogger = logging.getLogger()
    consoleHandler = logging.StreamHandler(sys.stdout)
    consoleHandler.setFormatter(ConsoleFormatter())
    consoleHandler.addFilter(ConsoleFilter())
    consoleHandler2 = logging.StreamHandler()
    consoleHandler2.setFormatter(ConsoleFormatter())
    consoleHandler2.setLevel(logging.WARNING)
    rootLogger.addHandler(consoleHandler)
    rootLogger.addHandler(consoleHandler2)
    rootLogger.setLevel(logging.DEBUG)
    return 0


def get_raw_bins(dir):
    raw_bins = []
    for file in os.listdir(dir):
        main_name, ext_name = os.path.splitext(file)
        if ext_name != ".raw":
            continue
        raw_bins.append(os.path.join(dir, file))
    return raw_bins


def get_bins(dir):
    bins = []
    for file in os.listdir(dir):
        if os.path.splitext(file)[1] != ".bin":
            continue
        elif any(x in file for x in ["rootfs", "appfs"]):
            continue
        bins.append(os.path.join(dir, file))
    return bins


def do_bch(dir, page_size, sources):
    if not os.path.exists(dir):
        system_call("mkdir -p {}".format(dir))
    for input_file in sources:
        target_file = os.path.join(dir, os.path.basename(input_file))
        cmd = "$BUILD_DIR/nvt-tools/bch_8bit --input {} --output {} --page-size {} --spare-align 1".format(
            input_file,
            target_file,
            page_size
        )
        logging.info(cmd)
        er, _ = system_call(cmd, False)
        if er != 0:
            return er
    return 0

def main(argv):
    if 0:
        argv.append(r"-b")
        argv.append(r"0x20000")
        argv.append(r"-o")
        argv.append(r"~/ws/na51102_dual_sdk/na51102_linux_sdk/output")

    initiate()
    args = parse_args(argv)

    # check if folder existing
    output_dir = os.path.expanduser(args.output_dir)
    if not os.path.isdir(output_dir):
        logging.error("{} is not a folder.".format(output_dir))

    # get bin list with raw (rootfs), we will skip the bin of rootfs (with cksm header)
    raw_bins = get_raw_bins(os.path.join(output_dir, "raw"))
    # get bin list in output except rootfs, appfs
    bins = get_bins(output_dir)
    # calc page size
    re_hex = re.compile(r"0x(\w*)")
    block_size = int(re.search(re_hex, args.block_size).group(1), 16)
    page_size = int(block_size / 64)
    # do bch
    sources = bins + raw_bins
    er = do_bch(os.path.join(output_dir, "bch"), page_size, sources)

    return er


if __name__ == '__main__':
    try:
        er = main(sys.argv)
    except Exception as exp:
        er = -1
        print(exp, file=sys.stderr)
        raise exp
    sys.exit(er)
