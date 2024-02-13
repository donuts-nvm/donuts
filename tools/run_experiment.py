#!/usr/bin/env python3
# Script created by Kleber Kruger

import os
import sys
import shutil
import argparse
import env_setup
import benchapps

ALL_BENCHMARKS = benchapps.get_benchmarks()
DEFAULT_BENCHMARK = 'cpu2006'
DEFAULT_INSTRUCTIONS = 1000000000
DEFAULT_RESULTS_ROOT = os.path.join(env_setup.sniper_root(), 'results')
DEFAULT_SLOTS = 4
TASK_SPOOLER = 'tsp'


def parse_args():
    parser = argparse.ArgumentParser()
    parser.add_argument('config', type=str, nargs='+',
                        help='sniper configurations to the experiment')
    parser.add_argument('-q', '-S', '--slots', type=int, default=DEFAULT_SLOTS,
                        help='number of task-spooler slots')
    parser.add_argument('-p', '--benchmark', type=str, choices=ALL_BENCHMARKS,
                        help='benchmark selected')
    parser.add_argument('-a', '--apps', type=str, nargs='+',
                        help='applications to run')
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
    parser.add_argument('-m', '--more', nargs='+',
                        help='run n more-intensive <attribute> applications. Example: p8 m8 b8>')
    parser.add_argument('-f', '--force', action='store_true',
                        help='force test (override old results)')
    parser.add_argument('--debug', action='store_true',
                        help='debug mode (only shows the commands)')
    return parser.parse_args()


def check_task_spooler(debug) -> None:
    if not shutil.which(TASK_SPOOLER) and not debug:
        raise EnvironmentError(
            f"task-spooler ({TASK_SPOOLER}) is not installed. Execute in debug mode (--debug) or install it!\n"
            f"Example: sudo apt-get install task-spooler")


def get_default_input(benchmark: str) -> str:
    return benchapps.get_inputs(benchmark, 'default')


def get_default_ncores(benchmark: str) -> int:
    return benchapps.get_min_cores(benchmark)


def detect_benchmark(apps):
    benchmark = benchapps.get_benchmarks(apps[0])[0]
    if len(apps) > 1:
        for app in apps[1:]:
            if benchapps.get_benchmarks(app) != benchmark:
                raise Exception('Applications from different benchmarks')
    return benchmark


def check_apps(benchmark_apps: list, apps: list) -> list:
    return list(set(apps) - set(benchmark_apps))


def check_input(benchmark_inputs: list, input_size: str) -> bool:
    return input_size is benchmark_inputs


def check_configs(configs: list) -> list:
    errors = []
    for config in configs:
        config_file = config if config.endswith('.cfg') else f'{config}.cfg'
        if not os.path.exists(os.path.join(env_setup.sniper_root(), 'config', config_file)):
            errors.append(config)
    return errors


def check_ncores(benchmark: str, apps: list, ncores: int) -> list:
    pass


def assign_and_valid(args) -> None:
    if not args.benchmark:
        args.benchmark = DEFAULT_BENCHMARK if not args.apps else detect_benchmark(args.apps)

    benchmark_apps = benchapps.get_apps(args.benchmark)
    if not args.apps:
        args.apps = benchmark_apps
    else:
        rejected = check_apps(benchmark_apps, args.apps)
        if rejected:
            raise ValueError(f"ERROR: Application{'s' if len(rejected) > 1 else ''} not found in the benchmark '"
                             f"{args.benchmark}': {rejected}")

    rejected = check_configs(args.config)
    if rejected:
        raise ValueError(f"ERROR: Config file{'s' if len(rejected) > 1 else ''} not found: {rejected}")

    if not args.input:
        args.input = get_default_input(args.benchmark)
    elif not check_input(benchapps.get_inputs(args.benchmark), args.input):
        raise ValueError(f"ERROR: Input not found to the benchmark '{args.benchmark}': {args.input}")

    if not args.ncores:
        args.ncores = get_default_ncores(args.benchmark)
    elif not check_ncores(args.benchmark, args.apps, args.ncores):
        # validate minimum cores required to each application
        pass

    if not args.test:
        args.test = create_test_name(args.root)


def create_test_name(results_dir: str):
    index = 1
    while True:
        test_name = f'test_{index}'
        if not os.path.exists(os.path.join(results_dir, test_name)):
            break
        index += 1
    return test_name


def get_output_path(results_dir: str, test_name: str, benchmark: str, app_name: str, num_cores: int, config: str) -> str:
    output = results_dir
    if test_name:
        output = os.path.join(results_dir, test_name)
    return os.path.join(output, f"{num_cores}-core{'s' if num_cores > 1 else ''}", benchmark, app_name, config)


def get_sniper_cmd(benchmark: str, app: str, config: str, input_size: str, num_cores: int, num_instr: int, output: str,
                   tsp_usage: bool = True):
    tsp = f'{TASK_SPOOLER} ' if tsp_usage else ''
    return f"{tsp}{env_setup.benchmarks_root()}/run-sniper -p {benchmark}-{app} -n {num_cores} -i {input_size} -c {config} -d {output} -s stop-by-icount:{num_instr}"


def execute(benchmark: str,
            app_name: str,
            config: str,
            input_size: str = None,
            num_cores: int = 0,
            num_instr: int = DEFAULT_INSTRUCTIONS,
            results_dir: str = DEFAULT_RESULTS_ROOT,
            force: bool = False,
            test_name: str = None,
            debug_mode: bool = False) -> None:

    if not input_size:
        input_size = get_default_input(benchmark)
    if num_cores == 0:
        num_cores = get_default_ncores(benchmark)

    output = get_output_path(results_dir, test_name, benchmark, app_name, num_cores, config)
    if os.path.exists(os.path.join(output, 'sim.out')) and not force:
        raise FileExistsError(f"File already exists: {os.path.join(output, 'sim.out')}")

    cmd = get_sniper_cmd(benchmark, app_name, config, input_size, num_cores, num_instr, output, True)
    print(cmd)
    if not debug_mode:
        os.system(f'mkdir -p {output} && {cmd}')


def execute_all(configs: list,
                benchmark: str = DEFAULT_BENCHMARK,
                apps: list = None,
                input_size: str = None,
                ncores: list = None,
                num_instr: int = DEFAULT_INSTRUCTIONS,
                results_dir: str = DEFAULT_RESULTS_ROOT,
                force: bool = False,
                test_name: str = None,
                num_slots: int = DEFAULT_SLOTS,
                debug_mode: bool = False) -> None:
    if not apps:
        apps = benchapps.get_apps(benchmark)
    if not input_size:
        input_size = get_default_input(benchmark)
    if ncores is None:
        ncores = [get_default_ncores(benchmark)]

    os.system(f'{TASK_SPOOLER} -S {num_slots}')

    for num_cores in ncores:
        for app in apps:
            for config in configs:
                try:
                    execute(benchmark=benchmark, app_name=app, config=config, input_size=input_size, num_cores=num_cores,
                            num_instr=num_instr, results_dir=results_dir, force=force, test_name=test_name, debug_mode=debug_mode)
                except Exception as ex:
                    print(ex, file=sys.stderr)


def main() -> None:
    args = parse_args()
    try:
        check_task_spooler(args.debug)
        assign_and_valid(args)
    except EnvironmentError or ValueError as ex:
        print(ex, file=sys.stderr)
        exit(1)

    execute_all(configs=args.config, benchmark=args.benchmark, apps=args.apps, input_size=args.input,
                ncores=args.ncores, num_instr=args.instr, results_dir=args.root, force=args.force,
                test_name=args.test, num_slots=args.slots, debug_mode=args.debug)


if __name__ == "__main__":
    main()
