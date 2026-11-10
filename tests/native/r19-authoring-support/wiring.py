#!/usr/bin/env python3
"""Guard the retained-VM lifetime where same-bundle process results cannot distinguish VMs."""
import argparse
from pathlib import Path
import re


FAMILIES = ('activate', 'status', 'preview', 'claim', 'takeover', 'refresh',
            'checkpoint', 'release', 'recover', 'check', 'check_recover', 'verdict')


def audit(path, composition=None):
    text = path.read_text()
    profile_start = text.index('response invoke_with_profile(')
    embedded_start = text.index('response invoke(std::span')
    profile = text[profile_start:embedded_start]
    embedded = text[embedded_start:]
    scope = '::planar::cmd::' + (composition + '::' if composition else '') + 'handlers::'
    profile_scope = '::planar::cmd::handlers::' if composition == 'authoring_stack' else scope
    for family in FAMILIES:
        expression = (r'return\s+' + re.escape(scope + family + '_command') +
                      r'\(\s*families\.' + family + r'\s*,\s*&\*vm\s*\)\s*;')
        assert len(re.findall(expression, embedded)) == 1, (
            f'{path}: {family} must forward the admitted VM exactly once')
        module = 'planar.cmd.' + (composition + '.execution.' if composition else 'execution.') + family
        assert len(re.findall(r'import\s+' + re.escape(module) + r'\s*;', text)) == 1, (
            f'{path}: {family} must import its typed composition handler')
        expression = (r'return\s+' + re.escape(profile_scope + family + '_with_profile') +
                      r'\(\s*families\.' + family + r'\s*,\s*prefix\.selector\s*\)\s*;')
        assert len(re.findall(expression, profile)) == 1, (
            f'{path}: {family} must preserve its closed profile owner')
        if composition == 'authoring_stack':
            assert len(re.findall(r'import\s+planar\.cmd\.execution\.' + family + r'\s*;', text)) == 1, (
                f'{path}: {family} must import the production profile handler')
    if composition:
        assert '::planar::cmd::handlers::' not in embedded, f'{path}: original embedded-handler fallback'
        if composition != 'authoring_stack':
            assert 'import planar.cmd.execution.' not in text, f'{path}: original module import'


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--source', type=Path,
                        default=Path(__file__).resolve().parents[3] / 'src/cmd/planar/bootstrap.cpp')
    parser.add_argument('--build', type=Path)
    args = parser.parse_args()
    audit(args.source)
    count = 1
    if args.build:
        command = args.build / 'src/cmd/planar'
        copies = [(command / 'authoring-stack-generated', 'authoring_stack'),
                  (command / 'r14-d7-b-generated', 'load_cli_probe'),
                  (args.build / 'x08-generated/0', 'x08_0')]
        copies += [(command / f'v5-02-b2-generated/{i}', f'cli_{i}') for i in range(6)]
        copies += [(command / f'v5-02-x07-generated/{i}', f'x07_{i}') for i in range(5)]
        for folder, composition in copies:
            audit(folder / 'bootstrap.cpp', composition)
            count += 1
    print(f'PASS {count} bootstrap sources; {count * len(FAMILIES)} retained-VM family bindings and profile routes')


if __name__ == '__main__':
    main()
