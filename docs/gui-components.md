Switch 2 GUI pieces/components:

- SFX:
  - NavigateToMenuOption
  - SelectRadioOption
  - SelectSameRadioOption
  - OpenPopupMenu
  - BoxChecked
  - BoxUnchecked
  - GoBack

Round icon button, clicking brings up popup menu with options
    - Icon is white at rest, blue when popup is open


Popup Menu:
- No open animation
- No close animation
- Plays OpenPopupMenu SFX when opened
- Plays GoBack SFX when closed
  - UNLESS an action is selected, then plays the appropriate SFX for that action instead
- Navigating through options plays NavigateToMenuOption SFX
  - If option is disabled, plays NavigateToMenuOption SFX with lower volume
  - If option is disabled, background is darker
- Radio Group with options
  - Blue text when active, blue circle indicator
  - Selecting an option plays SelectRadioOption SFX
    - Closes the popup menu after selection
  - Selecting the same option again plays SelectSameRadioOption SFX
    - Also closes the popup menu after selection
  - Selecting an option flashes an opaque blue highlight briefly over the option to indicate selection
- Multi-select group
  - Blue text when active, blue checkmark indicator
  - Empty box indicator when inactive
  - Selecting an option flashes an opaque blue highlight briefly over the option to indicate selection
  - Box indicator fills in blue with white checkmark with animation, plays BoxChecked SFX
  - Deselecting an option flashes an opaque blue highlight briefly over the option to indicate deselection
  - Box indicator empties with animation, plays BoxUnchecked SFX
- Very brief pause between closing the popup menu and the focus cursor reappearing
- Can have menu items that just open another modal - popup menu closes with no SFX, modal opens with its own SFX