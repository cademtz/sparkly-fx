# Known bugs

Real bugs *(should get fixed)*:
- Material tweak often causes shadows to disappear until game/server restart.
    - Cause: The shadow material uses a non-white coloring. MaterialTweak forces all coloring to white when disabled.
    - Fix: Store and restore every material's original coloring.
- Recorded audio file is sometimes missing.
    - Cause: The engine only outputs audio in the game directory. An attempt to move the file will fail because the engine is still using it.
    - Fix: Wait and retry, or find the right event to move it.

Limitations *(may be fixed)*:
- Material tweak does not affect props, entities, or other models
    - Cause: Some lower-level engine rendering functions will override material colors.
    - Fix: Find a better place to intercept material color override.