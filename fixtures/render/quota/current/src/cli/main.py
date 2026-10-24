"""Small quota CLI; import the implementation only after argument validation."""
import sys


def parse_args(args):
    if len(args) != 2 or any(not s or any(c not in "0123456789" for c in s) for s in args):
        raise ValueError("invalid arguments")
    try:
        return tuple(int(s) for s in args)
    except ValueError as error:
        raise ValueError("invalid arguments") from error


def run(args):
    try:
        balance, amount = parse_args(args)
    except ValueError:
        print("invalid arguments", file=sys.stderr)
        return 64
    from src.core.quota import Quota
    quota = Quota(balance)
    taken = quota.try_take(amount)
    print(f"remaining={quota.remaining}")
    return 0 if taken else 2


if __name__ == "__main__":
    sys.exit(run(sys.argv[1:]))
