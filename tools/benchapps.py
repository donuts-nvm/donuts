#!/usr/bin/env python
# Script created by Kleber Kruger

import env_setup
import json
import os
import subprocess
import sys

sys.path.append(env_setup.benchmarks_root())
suites = __import__('suites')

MODULES = sorted(suites.modules)
IS_PYTHON_2 = sys.version_info.major == 2

INPUT_MAP = {
    'cpu2006_pinballs': [None, None, None, 'ref'],
    'cpu2006': ['test', None, 'large', 'ref'],
    'splash2': ['test', 'small', 'large', 'large'],
    'npb': ['S', 'W', 'A', 'B'],
    'parsec': ['test', 'simsmall', 'simlarge', 'simlarge'],
    'jikes': [None, None, None, None],
    'local': ['test', 'small', 'large', 'large']
}

PARSEC_MIN_CORES = {
    'blackscholes': 2,
    'bodytrack': 3,
    'facesim': 1,
    'ferret': 6,
    'fluidanimate': 2,
    'freqmine': 1,
    'raytrace': 2,
    'swaptions': 2,
    'vips': 3,
    'x264': 1,
    'canneal': 2,
    'dedup': 4,
    'streamcluster': 2,
}


def _exec_in_python2(func, arg=None):
    commands = [os.path.abspath(__file__), func]
    if arg:
        commands.append(arg)
    return json.loads(subprocess.check_output(commands, universal_newlines=True))


def _load_module(module):
    if not IS_PYTHON_2:
        version = '{}.{}.{}'.format(sys.version_info.major, sys.version_info.minor, sys.version_info.micro)
        raise Exception("The module '{}' are incompatible with Python {}".format(module, version))
    try:
        return __import__(module)
    except ImportError as ex:
        raise ImportError("INFO: '{}' not downloaded yet, run make to download its components".format(module)
                          if module in MODULES else str(ex))


def _get_benchmark_inputs(benchmark):
    try:
        return _load_module(benchmark).allinputs()
    except Exception as ex:
        raise ex


def _get_benchmark_apps(benchmark):
    try:
        return _load_module(benchmark).allbenchmarks()
    except Exception as ex:
        raise ex


def _get_all_benchmark_apps():
    return {benchmark: _get_benchmark_apps(benchmark) for benchmark in MODULES}


def get_benchmarks(app=None):
    if IS_PYTHON_2:
        return MODULES if not app else [bm for bm, apps in _get_all_benchmark_apps().items() if app in apps]
    else:
        return _exec_in_python2('get_benchmarks', app)


def get_apps(benchmark=None):
    if IS_PYTHON_2:
        return _get_benchmark_apps(benchmark) if benchmark else {b: _get_benchmark_apps(b) for b in MODULES}
    else:
        return _exec_in_python2('get_apps', benchmark)


def get_inputs(benchmark=None, size=None):
    if size:
        return INPUT_MAP[benchmark][['test', 'small', 'large', 'default'].index(size)]
    elif IS_PYTHON_2:
        return _get_benchmark_inputs(benchmark) if benchmark else {b: _get_benchmark_inputs(b) for b in MODULES}
    else:
        return _exec_in_python2('get_inputs', benchmark)


def get_min_cores(benchmark=None, app=None):
    return PARSEC_MIN_CORES[app] if benchmark == 'parsec' and app else 1


def main():
    if len(sys.argv) < 2:
        sys.stderr.write("Usage: {} <function> [argument]\n".format(sys.argv[0]))
        sys.exit(1)

    func = sys.argv[1]
    arg = sys.argv[2] if len(sys.argv) > 2 else None

    if func == 'get_apps':
        print(json.dumps(get_apps(arg)))
    elif func == 'get_benchmarks':
        print(json.dumps(get_benchmarks(arg)))
    elif func == 'get_inputs':
        size = sys.argv[3] if len(sys.argv) > 3 else None
        print(json.dumps(get_inputs(arg, size)))
    else:
        sys.stderr.write("Invalid function: {}\n".format(func))
        sys.exit(1)


if __name__ == "__main__":
    main()
