# Known bugs

**Note: Some bugs in the preview are "fake". They don't exist while recording.**

Non-bugs or unrelated bugs (preview artifacts or TF2 bugs):
- After starting a new game/demo, some materials aren't being affected.
    - Cause: The game reloaded some materials, and sparkly-fx did not update them appropriately.
    - Hack: Toggle the preview on/off.
    - Fix: Update materials when new ones are added/modified, or update them every frame.
- HUD is disabled/enabled in all streams when using `cl_drawhud` for a single stream.
    - Cause: `cl_drawhud` only takes effect when `ClientModeShared::Update()` is called. Not during rendering.
    - Fix: Use the HUD setting in the Camera tweak.

Real bugs *(should get fixed)*:
- Sometimes in 64-bit, the depth buffer and other shaders appear black in the final video
    - Cause: Unknown
    - Hack: Re-arrange the order of your streams and play with your settings. The bug is not very random.
    - Fix: Unknown
- The recording indicator may appear in saved movie frames.
    - Cause: Screen is sometimes not cleared before next frame, or indicator draws too early.
    - Hack: Disable this in the "Recording" settings.
    - Fix: Unknown. Perhaps draw during `EVENT_IMGUI`, which always runs just before Present(). But if parts of the screen go un-cleared, it will still appear.
- Saving a config does not automatically append ".json" to the chosen name
- Flickering can appear when rendering with multiple streams
    - Cause: The HDR feature changes its exposure based on every frame that is rendered, even for mid-frame rendering.
    - Hack: Disable HDR with `mat_hdr_level 0`.
    - Fix: Unknown.
- Pixel shader texture coordinates are always 0
    - Cause: Unknown. Modifying/disabling texture transforms does nothing.
    - Hack: Pass texture coordinates using another vertex property like color, normal, etc.
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
    - Hack: Disable particles in the `Commands` tweak
    - Fix: Find a way to change particle colors.