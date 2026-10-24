# Pre-token facade refusal

During independent review the reviewer raised whether profile `bind_input`
refusal during `retain_file(slot)`, before native bound `add_input` entry, must
also mark a destination native session terminal. The coordinator confirmed:

> Before token acquisition/native add_input entry, refusal terminally poisons
> profile/original admission only; no bound session registration has occurred.
> Session marker requirement starts on entry to bound native add_input, including
> failures before publication there. No new poison-session API is needed for
> facade pre-token refusal.

This records the bounded interpretation of the existing failed unpublished native
registration amendment. It adds no product mutation or new session API. The review
retains actual native direct-binding failure and allocation tests on empty and
populated destinations.
