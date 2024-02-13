#!/usr/bin/env python3
# Script updated to Python 3 by Kleber Kruger

import errno
import os
import sys


def local_sniper_root():
    return os.path.dirname(os.path.dirname(os.path.realpath(__file__)))


def sniper_root():
    # Return an existing *_ROOT if it is valid
    for rootname in ('SNIPER_ROOT', 'GRAPHITE_ROOT'):
        root = os.getenv(rootname)
        if root:
            if not os.path.isfile(os.path.join(root, 'run-sniper')):
                raise EnvironmentError((errno.EINVAL, 'Invalid %s directory [%s]' % (rootname, root)))
            elif os.path.realpath(root) != local_sniper_root():
                sys.stderr.write('Warning: %s is different from current script directory [%s]!=[%s]\n' % (
                    rootname, os.path.realpath(root), local_sniper_root()))
            return root
    # Use the root corresponding to this file when nothing has been set
    return local_sniper_root()


def sim_root():
    return sniper_root()


def benchmarks_root():
    # Return an existing BENCHMARKS_ROOT if it is valid
    if os.getenv('BENCHMARKS_ROOT'):
        if os.path.isfile(os.path.join(os.getenv('BENCHMARKS_ROOT'), 'run-sniper')):
            return os.getenv('BENCHMARKS_ROOT')
        else:
            raise EnvironmentError(
                (errno.EINVAL, 'Invalid BENCHMARKS_ROOT directory [%s]' % os.getenv('BENCHMARKS_ROOT')))

    # Try to determine what the BENCHMARKS_ROOT should be if it is not set
    sniper_path = sniper_root()
    possible_paths = [
        os.path.realpath(os.path.join(sniper_path, 'benchmarks', 'run-sniper')),
        os.path.realpath(os.path.join(sniper_path, '..', 'run-sniper')),
        os.path.realpath(os.path.join(sniper_path, '..', 'benchmarks', 'run-sniper')),
        os.path.realpath(os.path.join(sniper_path, '..', 'sniper-benchmarks', 'run-sniper'))  # Added by Kleber Kruger
    ]

    for bt in possible_paths:
        if os.path.isfile(bt):
            return os.path.dirname(bt)

    raise EnvironmentError((errno.EINVAL, 'Unable to determine the BENCHMARKS_ROOT directory'))


def main():
    sniper_path = sniper_root()
    benchmarks_path = None
    try:
        benchmarks_path = benchmarks_root()
    except EnvironmentError as e:
        pass
    roots = {
        'SNIPER_ROOT': sniper_path,
        'GRAPHITE_ROOT': sniper_path,
        'BENCHMARKS_ROOT': benchmarks_path
    }
    print(roots)


if __name__ == "__main__":
    main()
