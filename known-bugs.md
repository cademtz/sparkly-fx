# Known bugs

Real bugs *(should get fixed)*:
- Rarely, the game crashes after ending a recording.
    - Cause: Unknown. The stack trace does not point to any sparkly-fx code. I also deleted my crash log and can't replicate it...
    - Fix: Unknown.

Limitations *(may be fixed)*:
- The "Fog depth" preset renders some materials without the depth effect (like water).
    - Cause: There is currently no access to the depth buffer. It is faked, and some models' materials can't be overriden yet.
    - Fix: Gain access to the depth buffer contents, or cover more edge cases.
- In TF2, the "Fog depth" preset makes the control point hologram solid.
    - Cause: The hologram is a `CBaseAnimating` entity. Entity materials are replaced with solid colors to override any undesired effects like reflections.
    - Hack: Add a new entity tweak that makes `CBaseAnimating` invisible. This will also affect health and ammo pickups.
    - Fix: Find a way to render semi-transparent materials separately, including the hologram.
- The "Player matte" preset does not hide particle colors
    - Cause: Particle colors can't be affected yet.
    - Hack: Disable particles in the `Miscellaneous` tweak
    - Fix: Find a way to change particle colors.