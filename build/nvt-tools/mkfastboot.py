#!/usr/bin/env python3
# -*- coding: utf-8 -*-

import sys
import os
import re
import subprocess
import argparse
import logging
from datetime import datetime


def system_call(command, get_stdout=True):
    if get_stdout:
        process = subprocess.Popen(command, stdout=subprocess.PIPE, shell=True)
        stdout = process.communicate()[0].decode("ascii", errors="ignore").splitlines()
        er = process.returncode
    else:
        er = os.system(command)
        stdout = ""
    if er != 0:
        logging.error("failed to {}, er={}".format(command, er))
    return er, stdout


def parse_args(argv):
    parser = argparse.ArgumentParser(description="")
    parser.add_argument(
        "-i",
        "--input",
        metavar=".dts.tmp",
        required=True,
        dest="input",
        action="store",
        help="nvt-fastboot.dts.tmp (cpp processed dts)",
    )
    parser.add_argument("-o", "--output", required=True, dest="output", action="store", help="output folder")
    parser.add_argument(
        "-d",
        "--dtr",
        required=True,
        dest="dtr",
        action="store",
        help="EMBMEM_DTR_NONE, EMBMEM_DTR_4_BIT, EMBMEM_DTR_8_BIT",
    )
    args = parser.parse_args()
    return args


class ConsoleFormatter(logging.Formatter):
    # MSG_FMT = "%(class)s%(asctime)s: %(message)s\033[0m"
    MSG_FMT = "%(class)s%(message)s\033[0m"
    CSS_CLASSES = {
        "WARNING": "\033[1;93m",
        "INFO": "\033[0m",
        "DEBUG": "\033[0m",
        "CRITICAL": "\033[1;91m",
        "ERROR": "\033[1;91merror: ",
    }

    def __init__(self):
        super().__init__()

    def format(self, record):
        try:
            class_name = self.CSS_CLASSES[record.levelname]
        except KeyError:
            class_name = "info"
        tm_now = datetime.now()
        # asctime = "{:02d}:{:02d}".format(tm_now.hour, tm_now.minute)
        asctime = "{:02d}/{:02d} {:02d}:{:02d}".format(tm_now.month, tm_now.day, tm_now.hour, tm_now.minute)
        message = record.msg
        return self.MSG_FMT % {"class": class_name, "message": message, "asctime": asctime}


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


def parse_dts(dts_path):
    desc = {
        "text": "",
        "includes": [],
    }

    re_critical_begin = re.compile('^# 1 "(.*)" 1')
    re_critical_end = re.compile('^# \d* "(.*)" 2')

    with open(dts_path, "r", encoding="utf-8") as file:
        while 1:
            line = file.readline()
            if not line:
                break
            match = re.search(re_critical_begin, line)
            if match:
                desc["includes"].append(match.group(1))
                # loop to end of critical
                while 1:
                    line = file.readline()
                    if re.search(re_critical_end, line):
                        break
            elif len(line.strip()):
                desc["text"] += line
    return desc


def make_build_sh(desc, dtr_pattern, output_dir):
    logging.info("generate build.sh")
    head = "#!/bin/sh"
    text = head + "\n" + "cat nvt-fastboot-etc.dts \\\n"
    for include in desc["includes"]:
        text += "{} \\\n".format(os.path.basename(include))

    if dtr_pattern is None:
        text += (
            "> nvt-fastboot.dts.tmp \n"
            "dtc -@ -O dtb -b 0 -W no-unit_address_format -W no-unit_address_vs_reg -o nvt-fastboot.dtb nvt-fastboot.dts.tmp \n"
        )
    else:
        text += (
            "> nvt-fastboot.dts.tmp \n"
            "dtc -@ -O dtb -b 0 -W no-unit_address_format -W no-unit_address_vs_reg -o nvt-fastboot.dtb.tmp nvt-fastboot.dts.tmp \n"
            f"cat {dtr_pattern} nvt-fastboot.dtb.tmp > nvt-fastboot.dtb"
        )

    target = os.path.join(output_dir, "build.sh")
    with open(target, "w", encoding="utf-8") as file:
        file.write(text)

    os.chmod(target, 0o755)
    return 0


def copy_necessary(desc, dtr_pattern, output_dir, input_dir):
    for include in desc["includes"]:
        logging.info("copying {}".format(include))
        if include[0] == "/":
            target_file = os.path.join(output_dir, os.path.basename(include))
            symbolic_src = include
        else:
            target_file = os.path.join(output_dir, include)
            symbolic_src = os.path.join("../{}".format(include))

        if os.path.exists(target_file):
            cmd = "rm -f {}".format(target_file)
            er, _ = system_call(cmd, False)
            if er:
                return er

        cmd = "ln -s {} {}".format(symbolic_src, target_file)
        er, _ = system_call(cmd, False)
        if er:
            return er

    if dtr_pattern:
        cmd = "cp {} {} -f".format(os.path.join(input_dir, dtr_pattern), output_dir)
        er, _ = system_call(cmd, False)
        if er:
            return er

    return 0


def gen_dts(desc, output_dir):
    logging.info("generate nvt-fastboot-etc.dts")
    target = os.path.join(output_dir, "nvt-fastboot-etc.dts")
    with open(target, "w", encoding="utf-8") as file:
        file.write(desc["text"])
    return 0


def main(argv):
    if 0:
        argv.append(r"-i")
        argv.append(r"~/tmp/fastboot/nvt-fastboot.dts.tmp")
        argv.append(r"-o")
        argv.append(r"~/tmp/etc/fastboot")

    initiate()

    args = parse_args(argv)

    dts_path = os.path.expanduser(args.input)
    output_dir = os.path.expanduser(args.output)

    dtr_pattern = None
    if args.dtr == "EMBMEM_DTR_4_BIT":
        dtr_pattern = "flash_tunning_pattern_4bit.bin"
    elif args.dtr == "EMBMEM_DTR_8_BIT":
        dtr_pattern = "flash_tunning_pattern_8bit.bin"

    if not os.path.isfile(dts_path):
        logging.error("unable to open {}".format(dts_path))
        return -1

    if not os.path.isdir(output_dir):
        cmd = "mkdir -p {}".format(output_dir)
        er, _ = system_call(cmd, False)
        if er:
            logging.error("unable to create {}".format(output_dir))
            return er

    desc = parse_dts(dts_path)
    if desc is None:
        return -1

    er = copy_necessary(desc, dtr_pattern, output_dir, os.path.dirname(dts_path))
    if er:
        return er

    er = gen_dts(desc, output_dir)
    if er:
        return er

    er = make_build_sh(desc, dtr_pattern, output_dir)
    if er:
        return er

    return 0


if __name__ == "__main__":
    try:
        er = main(sys.argv)
    except Exception as exp:
        er = -1
        print(exp, file=sys.stderr)
        raise exp
    sys.exit(er)
