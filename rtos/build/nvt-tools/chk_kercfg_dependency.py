#!/usr/bin/env python3
# -*- coding: utf-8 -*-

import sys
import os
import re
import subprocess
import requests
import argparse
import colorama
import logging
import getpass
from datetime import datetime

_test_mode = 0

ignore_symbols = [
    "strncpy",
    "strcpy",
    "strncat",
    "strncmp",
    "strcmp",
    "strnstr",
    "strlen",
    "strchr",
    "strsep",
    "sprintf",
    "kstrtoul",
    "kstrtoint",
    "kstrtouint",
    "kstrtoull",
    "vsnprintf",
    "snprintf",
    "sscanf",
    "sscanf_s",
    "va_list",
    "va_start",
    "va_end",
    "memcpy",
    "memset",
    "memmove",
    "memcmp",
    "__memset_io",
    "__stack_chk_guard",
    "__stack_chk_fail",
    "__aeabi_idiv",
    "__aeabi_uidiv",
    "arm_heavy_mb",
    "printk",
    "__warn_printk",
    "__do_div64",
    "__aeabi_uidivmod",
    "__aeabi_llsl",
    "__aeabi_idivmod",
    "kthread_should_stop",
    "__const_udelay",
    "vos_task_enter",
    "vos_flag_set",
    "vos_flag_wait",
    "vos_flag_clr",
    "vos_task_create",
    "vos_task_set_priority",
    "vos_task_resume",
    "vos_task_destroy",
    "vos_cpu_get_phy_addr",
    "vos_file_read",
    "vos_file_open",
    "vos_file_close",
    "vos_file_write",
    "vos_sem_create",
    "vos_sem_destroy",
    "vos_flag_create",
    "vos_flag_destroy",
    "vos_perf_mark",
    "vos_sem_sig",
    "vos_sem_wait",
    "vos_sem_wait_interruptible",
    "vos_flag_chk",
    "vos_util_msec_to_tick",
    "vos_util_delay_ms",
    "vos_sem_wait_timeout",
    "vos_task_delay_ms",
    "vk_spin_unlock_irqrestore",
    "vos_flag_wait_interruptible",
    "_vk_raw_spin_lock_irqsave",
    "vos_unregister_chrdev_region",
    "vos_perf_duration",
    "vos_cpu_dcache_sync",
    "vos_alloc_chrdev_region",
    "vos_util_delay_us_polling",
    "vos_util_delay_us",
    "fdt_path_offset",
    "fdt_getprop",
    "wait_for_completion",
    "wait_for_completion_timeout",
    "seq_printf",
    "__ll_sc_atomic_add",
    "__ll_sc_atomic_sub",
    "__ll_sc_atomic64_fetch_or",
    "__ll_sc_atomic64_or",
    "panic",
    "schedule_timeout",
    "__ll_sc___cmpxchg_case_acq_4",

]

tracer_ignore_symbols = [
    "__gnu__mcount_nc",
    "_mcount",
]

limited_symbols = [
    "__kmalloc",
    "kmalloc_caches",
    "kmem_cache_alloc",
    "kfree",
    "of_find_node_opts_by_path",
    "of_property_read_variable_u8_array",
    "__arch_copy_from_user",
    "__arch_copy_to_user",
    "device_unregister",
    "class_destroy",
    "__iounmap",
    "__this_module",
    "vmalloc",
    "vfree",
    "__ioremap",
    "free_irq",
    "fmem_lookup_pa",
    "pdrv_info_data",
    "clk_get_parent",
    "clk_set_parent",
    "clk_put",
    "clk_prepare",
    "clk_enable",
    "clk_unprepare",
    "clk_get_rate",
    "clk_disable",
    "__tasklet_schedule",
    "ioremap_cache",
    "proc_mkdir",
    "proc_create",
    "proc_remove",
    "filp_open",
    "filp_close",
    "msleep",
    "tasklet_init",
    "queued_spin_lock_slowpath",
    "__wake_up",
    "__init_waitqueue_head",
    "schedule",
    "init_wait_entry",
    "prepare_to_wait_event",
    "__release_region",
    "platform_driver_unregister",
    "single_open",
    "seq_lseek",
    "seq_read",
    "single_release",
    "cdev_init",
    "cdev_del",
    "cdev_add",
    "unregister_chrdev_region",
    "iomem_resource",
    "of_match_device",
    "platform_get_resource",
    "__request_region",
    "platform_get_irq",
    "__class_create",
    "device_create",
    "no_llseek",
    "proc_remove",
    "clk_get",
    "param_ops_int",
    "kernel_read",
    "kernel_write",
    "finish_wait",
    "of_property_read_variable_u32_array",
    "__platform_driver_register",
    "request_threaded_irq",
    "of_find_property",
    "alloc_chrdev_region",
    "set_user_nice",
    "schedule_timeout",
    "init_timer_key",
    "add_timer",
    "hrtimer_init",
    "hrtimer_cancel",
    "kthread_create_on_node",
    "wake_up_process",
    "del_timer_sync",
    "kthread_stop",
    "hrtimer_start_range_ns",
    "mod_timer",
    "devm_clk_get",
    "complete",
    "clk_set_rate",
    "mutex_lock",
    "mutex_unlock",
    "up",
    "down",
    "kmalloc_order",
    "__UNIQUE_ID_version206"
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
    parser = argparse.ArgumentParser(description='')
    parser.add_argument('-t', '--target', required=True,
                        dest='target', action='store',
                        help='target file or folder')
    parser.add_argument('-c', '--compiler_type',
                        dest='cpu', action='store',  choices={"32", "64"},
                        help='check 32 or 64 bit compiler_type')
    parser.add_argument('-l', '--level',
                        dest='level', action='store', type=int, default=1,  choices={1, 2, 3},
                        help='msg level 1: only noticed(default), 2: ignore white list, 3: list all link symbol')
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

def password_input(username):
    password = None
    while password==None or len(password) == 0:
        password = getpass.getpass("{}'s password:".format(username))
    return password

def objs_to_symbols(filepath):
    symbols = []
    re_objdump = re.compile(r"([^\s]*)\s(.{8})([^\s]*)\t([^\s]*)\s(.*)")
    cmd = "$OBJDUMP -t {}".format(filepath)
    er, lines = system_call(cmd)
    for line in lines:
        match = re.search(re_objdump, line)
        if match:
            symbol = {
                "offset": int(match.group(1), 16),
                "flags": match.group(2),
                "section": match.group(3),
                "size": int(match.group(4), 16),
                "name": match.group(5),
            }
            symbols.append(symbol)
    return symbols

def get_symbol_categories(filepath):
    symbols = objs_to_symbols(filepath)
    symbol_errors = []
    symbol_ignores = []
    symbol_others = []
    for symbol in symbols:
        if symbol["section"] == ".modinfo":
            symbol_errors.append(symbol["name"])
        if symbol["section"] != "*UND*":
            continue
        if symbol["name"] in ignore_symbols:
            symbol_ignores.append(symbol["name"])
            continue
        if "no_tracer.o" not in filepath and symbol["name"] in tracer_ignore_symbols:
            symbol_ignores.append(symbol["name"])
            continue
        if symbol["name"] in limited_symbols:
            symbol_errors.append(symbol["name"])
        else:
            symbol_others.append(symbol["name"])
    return symbol_errors, symbol_ignores, symbol_others

def get_author(filepath):
    cwd = os.getcwd()
    os.chdir(os.path.dirname(filepath))
    c_file = os.path.basename(filepath).replace("_no_tracer","").replace(".o",".c")
    if os.path.exists(c_file):
        cmd = "git log -10 --pretty=format:%ae {}".format(c_file)
    else:
        cmd = "git log -10 --pretty=format:%ae {}".format(os.path.basename(filepath))
    er, lines = system_call(cmd)
    os.chdir(cwd)
    if er != 0:
        return "unknown"
    if len(lines) == 0:
        return "no_git"
    authors = dict()
    for line in lines:
        if line not in authors:
            authors[line] = 0
        authors[line] += 1

    sorted_authors = {k: v for k, v in sorted(authors.items(), reverse = True, key=lambda item: item[1])}
    perhaps_author = list(sorted_authors.items())[0][0]
    return perhaps_author

def generate_description(filepath, symbol_errors):
    header = \
        r'<p>以下是系統偵測到閣下在,</p>' + \
        '<p>{}</p>'.format(filepath) + \
        r'<p>此對應的 source file 未被列在 publish.json, 且有 kernel config 的依賴性產生, 您可以選擇以下,</p>' + \
        r'<p>&emsp;1. 修正它, 把相依的部份放在有 source open 的 .c 中</p>' + \
        r'<p>&emsp;2. 開放它, 如果沒有重要的 code, 是可以把這個 .c 加入到對應的 publish.json 中</p>' + \
        r'<p>&emsp;3. 轉給別人解, 該 .c 是由機器人判斷 author, 有可能會出錯, 若你知道請誰修, 麻煩轉 assign 給他, 若不知道轉給我也可以</p>' + \
        r'<p>請 4/15 前完成</p>' + \
        r'<p>要 check 有沒有修掉, 只要 make all 後, 如下鍵入 (那個 $ 要跟著打) </p>' + \
        r'<p>$BUILD_DIR/nvt-tools/chk_kercfg_dependency.py -t [你的 .o 的路徑]</p>' + \
        r'<p>e.g: $BUILD_DIR/nvt-tools/chk_kercfg_dependency.py -t code/hdal/drivers/k_driver/source/comm/drv_sys/nvt_jiffies_no_tracer.o</p>' + \
        r'<p>偵側到的相依 symbol 如下,</p>'

    text =  \
        r'<pre class="jeCodeBlock">' + \
        r'<code class="language-bash">'
    for symbol_error in symbol_errors:
        text += "{}\n".format(symbol_error)

    text += r'</code></pre>'
    return header + text

def chk_symbol_jira(filepath, shortpath):
    symbol_errors, symbol_ignores, symbol_others = get_symbol_categories(filepath)
    if len(symbol_errors) == 0:
        return None, None
    author = get_author(filepath)
    jira_description = generate_description(shortpath, symbol_errors)
    return author, jira_description

def chk_symbol(filepath, msg_level):
    er = 0
    symbol_errors, symbol_ignores, symbol_others = get_symbol_categories(filepath)

    if msg_level == 1:
        if len(symbol_errors) == 0:
            return er
        er = -1
        logging.error("[{}]".format(filepath))
        for symbol_error in symbol_errors:
             logging.error("{}".format(symbol_error))
        return er

    if msg_level == 2:
        logging.info("[{}]".format(filepath))
        if len(symbol_errors):
            er = -1
            for symbol_error in symbol_errors:
                logging.error("* {}".format(symbol_error))
        for symbol_other in symbol_others:
            logging.info(symbol_other)
        return er

    logging.info("[{}]".format(filepath))
    if len(symbol_errors):
        er = -1
        for symbol_error in symbol_errors:
            logging.error("* {}".format(symbol_error))
    for symbol_other in symbol_others:
        logging.info(symbol_other)
    for symbol_ignore in symbol_ignores:
        logging.info(symbol_ignore)

    return er

def chk_fileformat(filepath, criteria):
    # criteria: elf64-littleaarch64, elf32-littlearm
    re_objdump = re.compile(r"file format (.*)")
    cmd = "$OBJDUMP -t {}".format(filepath)
    er, lines = system_call(cmd)
    for line in lines:
        match = re.search(re_objdump, line)
        if match:
            fileformat = match.group(1)
            if fileformat == criteria:
                return 0
            else:
                logging.info("{}: {}".format(fileformat, filepath))
                return -1
    return -1

"""
import libmynvt
import libjira
def main2():
    jira_key = "NA51102"
    parent_jira = "NA51102-572"
    compiler_type = "elf64-littleaarch64" # elf32-littlearm, elf64-littleaarch64
    target_dir = "~/firmware/na51102_dual_sdk/na51102_linux_sdk/base/hdal"

    initiate()
    target_dir = os.path.expanduser(target_dir)
    if not os.path.exists(target_dir):
        logging.error("failed to open {}".format(target_dir))
        return -1

    if "USER" in os.environ:
        username = os.environ['USER'] #ubuntu
    else:
        username = os.environ['USERNAME'].lower() #windows
    password = password_input(username)

    jira = libjira.Jira(username=username,password=password)
    mynvt = libmynvt.MyNvt(username=username, password=password)

    # get parent's summary to be subtask summary
    parent_summary = jira.query("key={}".format(parent_jira))[0]["summary"]

    for root, dirs, files in os.walk(target_dir):
        for file in files:
            main_name, ext_name = os.path.splitext(file)
            if ext_name != ".o":
                continue
            fullpath = os.path.join(root, file)
            author, description = chk_symbol_jira(fullpath, fullpath.replace(target_dir,"."))
            if author is None and description is None:
                continue
            if author == "no_git":
                logging.warning("no git log: {}".format(fullpath))
                continue
            if _test_mode:
                author = "niven_cho@novatek.com.tw"
            assignee = mynvt.email_to_account(author)
            if assignee is None:
                assignee = username
            summary = parent_summary + " - " + os.path.basename(fullpath)
            logging.warning("if you really want to create jira, plz un-mark below.")
            #er = jira.create_subtask(jira_key, assignee, parent_jira, summary, description)
            #if er != 0:
            #    logging.error("failed to assign {}".format(author))
            #else:
            #    jira.add_watcher(parent_jira, assignee)
    return 0
"""

def main(argv):
    #return main2()
    if 0:
        argv.append(r"-t")
        argv.append(r"~/ws/ns02301_dual_sdk/ns02301_linux_sdk/base/hdal/drivers/k_driver/source/kdrv_gfx2d/kdrv_grph_if")
        argv.append(r"-c")
        argv.append(r"32")

    if os.getenv("CONFIG_DIR") is None:
        logging.error("Please source build/envsetup.sh in BSP firstly to have auto copyso function")
        return -1

    msg_level = 1
    initiate()
    args = parse_args(argv)

    compiler_type = None
    if args.cpu == "32":
        compiler_type = "elf32-littlearm"
    elif args.cpu == "64":
        compiler_type = "elf64-littleaarch64"

    err_cnt = 0
    target_dir = os.path.expanduser(args.target)
    if not os.path.exists(target_dir):
        logging.error("failed to open {}".format(target_dir))
        return -1
    if os.path.isdir(target_dir):
        if compiler_type is not None:
            for root, dirs, files in os.walk(args.target):
                for file in files:
                    main_name, ext_name = os.path.splitext(file)
                    if ext_name != ".o":
                        continue
                    fullpath = os.path.join(root, file)
                    chk_fileformat(fullpath, compiler_type)
        for root, dirs, files in os.walk(args.target):
            for file in files:
                main_name, ext_name = os.path.splitext(file)
                if ext_name != ".o":
                    continue
                fullpath = os.path.join(root, file)
                if chk_symbol(fullpath, args.level) != 0:
                    err_cnt += 1
    elif os.path.isfile(args.target):
        if compiler_type is not None:
            chk_fileformat(target_dir, compiler_type)
        if chk_symbol(target_dir, args.level) != 0:
            err_cnt += 1

    if err_cnt == 0:
        logging.info("congratulations! not found any kernel config dependency.")
        return 0

    return -1



if __name__ == '__main__':
    try:
        er = main(sys.argv)
    except Exception as exp:
        er = -1
        print(exp, file=sys.stderr)
        raise exp
    sys.exit(er)
