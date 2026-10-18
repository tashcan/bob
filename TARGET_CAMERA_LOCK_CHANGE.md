# Disable camera follow for selected targets

The `[graphics] disable_target_camera_lock = true` setting clears
`NavigationPan._trackingPOI` when the `NavigationPan.LateUpdate` hook runs.
The pointer is deliberately **not restored** at the end of the frame: restoring
it would re-enable camera-follow state on the next update and make the setting
appear ineffective. This field is the camera-follow state used by
`NavigationPan`; target selection and its UI marker are handled separately by
the game.

Set the option to `false` to retain the game's default target-follow behavior.

This is a source change only. It has not been compiled or tested in-game in this
environment; confirm target selection/marker behavior and manual mouse/touch/
keyboard panning in the game after building the DLL.
