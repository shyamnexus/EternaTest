#!/usr/bin/env python3
# -*- coding: utf-8 -*-

import sys
import os
import shutil
import argparse
import colorama
import logging

from datetime import datetime

def create_symlink_directory(src_dir, dest_dir):
    """
    Create a directory with symbolic links to the content of src_dir in dest_dir,
    using relative paths for files and creating new directories for subdirectories.

    :param src_dir: The source directory containing the original files.
    :param dest_dir: The destination directory to create with symbolic links.
    """
    # Check if source directory exists
    if not os.path.exists(src_dir):
        raise FileNotFoundError(f"The source directory '{src_dir}' does not exist.")

    # Check if destination directory exists, if not, create it
    if not os.path.exists(dest_dir):
        os.makedirs(dest_dir)

    # Get the absolute paths of the source and destination directories
    abs_src_dir = os.path.abspath(src_dir)
    abs_dest_dir = os.path.abspath(dest_dir)

    # Iterate through all the files and directories in the source directory
    for item in os.listdir(abs_src_dir):
        src_path = os.path.join(abs_src_dir, item)
        dest_path = os.path.join(abs_dest_dir, item)

        if os.path.isdir(src_path):
            # If the item is a directory, create a new directory in the destination
            if not os.path.exists(dest_path):
                os.makedirs(dest_path)
            # Recursively create symlinks for the subdirectory
            create_symlink_directory(src_path, dest_path)
        else:
            if item in ['Makefile', 'publish.json']:
                shutil.copy(src_path, dest_path)
                logging.info(f"Copy: {dest_path} -> {dest_path}")
            else:
                # If the item is a file, calculate the relative path from the destination to the source
                rel_path = os.path.relpath(src_path, os.path.dirname(dest_path))
                # Create a symbolic link in the destination directory pointing to the source item
                os.symlink(rel_path, dest_path)
                logging.info(f"Created symlink: {dest_path} -> {rel_path}")

def parse_args(argv):
    parser = argparse.ArgumentParser(description='')
    parser.add_argument('-t', '--target', required=True,
                        dest='target', action='store',
                        help='target folder')
    parser.add_argument('-s', '--source', required=True,
                        dest='source', action='store',
                        help='source folder')
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
        # asctime = "{:02d}:{:02d}".format(tm_now.hour, tm_now.minute)
        asctime = "{:02d}/{:02d} {:02d}:{:02d}".format(tm_now.month, tm_now.day, tm_now.hour, tm_now.minute)
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
    colorama.init()
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

def main(argv):
    if 0:
        argv.append(r"-s")
        argv.append(r"ol1")
        argv.append(r"-t")
        argv.append(r"ol2")

    initiate()
    args = parse_args(argv)

    create_symlink_directory(args.source, args.target)

    return 0



if __name__ == '__main__':
    try:
        er = main(sys.argv)
    except Exception as exp:
        er = -1
        print(exp, file=sys.stderr)
        raise exp
    sys.exit(er)
