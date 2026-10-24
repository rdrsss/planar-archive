Python interface: src.core.quota.Quota(balance: int).
A balance and amount must be nonnegative integers; bool is invalid.
Invalid values raise ValueError without changing state.
Quota.remaining returns the current integer balance.
Quota.try_take(amount: int) -> bool atomically subtracts when affordable;
otherwise it returns False and preserves the balance. Taking zero succeeds.
The command accepts exactly BALANCE AMOUNT as decimal nonnegative integers.
It prints remaining=<integer> followed by LF on success or exhaustion.
Exit statuses: 0 taken, 2 exhausted, 64 invalid arguments (stderr only).
