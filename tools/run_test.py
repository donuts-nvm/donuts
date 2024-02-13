#!/usr/bin/env python3
# Script created by Kleber Kruger

import os
import argparse
import env_setup
import benchapps

ALL_BENCHMARKS = benchapps.get_benchmarks()
DEFAULT_RESULTS_ROOT = os.path.join(env_setup.sniper_root(), 'results')
DEFAULT_INSTRUCTIONS = 1000000000


def parse_args():
    parser = argparse.ArgumentParser()
    parser.add_argument('-p', '--benchmark', type=str, choices=ALL_BENCHMARKS,
                        help='benchmark selected')
    parser.add_argument('-a', '--apps', type=str, nargs='+',
                        help='applications to run')
    parser.add_argument('config', type=str, nargs='+',
                        help='sniper configurations to the experiment')
    parser.add_argument('-i', '--in', '--input', dest='input', type=str,
                        help='input type')
    parser.add_argument('-r', '--root', type=str, default=DEFAULT_RESULTS_ROOT,
                        help='results root directory path')
    parser.add_argument('-t', '--test', type=str,
                        help='test name')
    parser.add_argument('-n', '--ncores', type=int, nargs='+', default=[0],
                        help='number of cores to run')
    parser.add_argument('-s', '--instr', type=int, default=DEFAULT_INSTRUCTIONS,
                        help='number of instructions')
    parser.add_argument('-f', '--force', action='store_true',
                        help='force test (override old results)')
    parser.add_argument('--debug', action='store_true',
                        help='debug mode (only shows the commands)')


def main() -> None:
    pass


if __name__ == "__main__":
    main()
