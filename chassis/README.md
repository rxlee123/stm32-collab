# Optional 3D-printable chassis

Not used on the current robot (it has its own chassis). Kept for reference or a rebuild.

- `robot_chassis.FCMacro`: FreeCAD macro (Macro → Macros → Execute). Builds the lower deck, top deck, claw cage, gate arm, spacers and caster spacer as solids, with dimension lines. Sizes at the top of the file.
- `build_chassis.py`: the same design in Python (manifold3d + trimesh), exports STL.
- `stl/`: print files, already laid flat. PLA, 0.2 mm layers, 25 % infill; supports only for the claw cage.

Sized for a 15 × 9 cm mainboard, a 9 × 7.3 cm sensor board, TT motors with 65 mm wheels and a CY-15A ball caster.
