#!/usr/bin/env python3
# Script added by Kleber Kruger

# Usage:
# ./get_speccmds.py <*path>
# ./get_speccmds.py <*app (id or id.name or name)> <*size> <index>

import sys, os, errno, argparse
import env_setup

def parse_args():
    parser = argparse.ArgumentParser()
    parser.add_argument('app', type=str, help='application path or id/name (<id>, <name> or <id.name>)')
    parser.add_argument('input', type=str, nargs='?', choices=['test', 'train', 'ref'], help='input size')
    parser.add_argument('index', type=int, nargs='?', help='index of input')
    parser.add_argument('-r', '--root', type=str, default=f"{env_setup.benchmarks_root()}/cpu2017", help='<root-path> from cpu2017 directory')
    parser.add_argument('-t', '--tune', type=str, choices=['base', 'peak'], default='base', help='tune')
    parser.add_argument('-o', '--out', '--output', type=str, help='output file')
    return parser.parse_args()


def extract_cmd(line):
    cmd = line[line.find("../run"):].split(' ', 1)
    exe, args = os.path.basename(cmd[0]), cmd[1][:cmd[1].find('>')]
    return f"./{exe} {args}".strip()


def get_spec_cmds(path):
    speccmds_path = os.path.join(path, 'speccmds.cmd') if os.path.isdir(path) else path
    if not os.path.exists(speccmds_path):
        raise FileNotFoundError(errno.ENOENT, os.strerror(errno.ENOENT), speccmds_path)
    cmds = []
    with open(speccmds_path, 'r') as file:
        for line in file:
            if (line.startswith('-i') or line.startswith('-o') or line.startswith('-e')):
                cmds.append(extract_cmd(line))
    return cmds


def get_app_spec_cmds(app_path):
    run_path = f"{app_path}/run"
    runs = list(filter(lambda f: os.path.isdir(f"{run_path}/{f}"), os.listdir(run_path)))
    data = {
        'base':
            {
                'test': [],
                'train': [],
                'ref': [],
            },
        'peak':
            {
                'test': [],
                'train': [],
                'ref': [],
            },
    }
    for run in runs:
        _, tune, size, _ = run.split('_', 3)
        if size.startswith('ref'):
            size = 'ref'
        data[tune][size] = get_spec_cmds(f"{run_path}/{run}")
    return data


def get_app_spec_cmd(app_name, size, index, tune='base', cpu2017_path=f"{os.environ['BENCHMARKS_ROOT']}/cpu2017"):
    run_dir = f"run_{tune}_{size}_donuts-m64.0000"
    path = os.path.join(cpu2017_path, app_name, 'run', run_dir)
    return get_spec_cmds(path)[index]


def main():
    cmds = get_spec_cmds(sys.argv[1])
    for cmd in cmds:
        print(cmd)


if __name__ == '__main__':
    # main()
    # cmd = get_app_spec_cmd('502.gcc_r', 'test', 0)
    # print(cmd)
    args = parse_args()
    print(args)
