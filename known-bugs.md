# Known bugs

Real bugs *(should get fixed)*:
- None

Limitations *(may be fixed)*:
- Material tweak does not affect props, entities, or other models
    - Cause: Some lower-level engine rendering functions will override material colors.
    - Fix: Find a better place to intercept material color override.