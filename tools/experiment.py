#!/usr/bin/env python3

# Script created by Kleber Kruger

import env_setup
import benchapps
import os
import argparse

DEFAULT_RESULTS_ROOT = os.path.join(env_setup.sniper_root(), 'results')


def generate_args():
    parser = argparse.ArgumentParser()

    parser.add_argument('-p', '--benchmark', type=str, choices=benchapps.get_benchmarks(), help='benchmark selected')
    parser.add_argument('-p', '--benchmark', type=str, default='cpu2006', help='benchmark selected')

    parser.add_argument('-a', '--app', '--apps', type=str, nargs='+', help='applications to run')
    parser.add_argument('-a', '--app', '--apps', type=str, nargs='+', default=[],
                        help='applications to extract results')

    parser.add_argument('config', type=str, nargs='+', help='sniper configurations to the experiment')
    parser.add_argument('-c', '--config', '--configs', type=str, nargs='+', default=[],
                        help='configurations to extract results')

    parser.add_argument('-d', '--dir', '--root', type=str, default=DEFAULT_RESULTS_ROOT,
                        help='results root directory path')
    parser.add_argument('-r', '--root', type=str, default=DEFAULT_RESULTS_ROOT,
                        help='<root-path> from results directory')

    parser.add_argument('-t', '--test', '--name', type=str, help='test name')


class Experiment:
    def __init__(self, benchmark: str, apps: list, configs: list, results_root: str, test_name: str):
        self.benchmark: str = benchmark
        self.apps: list = apps
        self.configs: list = configs
        self.result_root: str = results_root
        self.test_name: str = test_name

    def execute(self, input_size: str, num_cores: int, num_instr, num_slots: int,
                force: bool, debug: bool):
        pass

    def read(self, baseline: str, num_cores: list, infos: list, out_file: str, err_file: str):
        pass
