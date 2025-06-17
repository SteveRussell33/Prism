# Prism change log
2.4.0

Rainbow:
~33% to ~50% Rack CPU load improvement - thanks [@danngreen](https://github.com/danngreen)
* Populate state only when needed
* Added faster brown noise gen
* Use std::clamp
* Some type changes

Audio: Optimized output summing

Small fixes for Spread and Morph

CI builds: Added ``#include <algorithm>`` conditionally to Rainbow.hpp to fix CI builds (C++11)
* Local dev builds will use std::clamp (C++17)

2.3.4
* Rainbow: Fix intermittent wayward output levels in audio processor (due to Audio module init bug)
* Rainbow: Fix CPU mode menu
* Rainbow: Fix filter Linux instantiation crash
* Rainbow & Spectrum: Initialize some variables for intermittent crash fixes

2.3.3
* Dark mode LED fix - seems to also help with intermittent crashes on patch load
* Rainbow: Fix channel locks and channel Qs

2.3.1 - 2.3.2
* Added hardware tags
* Misc plugin.json updates

2.3.0
* Updated for Rack2 API
* Rainbow: new panel and LED palette by Pyer
* Spectrum: updated panel
* Droplet: new panel by Pyer
* Droplet: only process if there's an input

## Rainbow

1.2.0
* Performance improvements with a 25%-45% reduction in CPU load
* New scale structure for 48/96 mode
* Add lights to show input mode for Poly In
* Fix BP scale
* Fix V/Oct in for 246 mode
* Fix overflow im motion_rotate
* Fix scale repeats for non-octave scales
* Tweaks to UI 

1.1.0
* Complete UI overhaul by Pyer
* Polyphonic audio input and output, with 1,2 and 6 channels in and out
* Added Spectrum - user scale creator for Rainbow
* Per-channel mono input for Level and Q, and output for V/Oct and Envelope
* V/Oct tracking for BpRe mode - just the basic filter tuning, not frequency nudge or V/Oct input
* The final calculated input levels are echoed on Poly Env out channels 7-12
* Refactoring scale data with additional bank and scale information
* Recalculate Mesopotamian scale
* Restore unused scales from SMR
* Update trigger calculation
* Revise tuning LED colour scheme
* Merge clip and level LEDs
* Freq CV now plus/minus 5V, 1V/Oct
* Nudge CV now +-1 semitone
* Transpose now +- 1 octave in semitone steps
* A polyphonic input to the FREQ CV1 port addresses all channels 1,3 and 5, and on the CV6 port to channels 2, 4 and 6. These inputs to do not pass through a LPF
* Revised Level and Q calculations again

1.0.2
* Split Level and Q CV inputs into separate global and channel inputs
* Add indicator for channel level
* Revise Level calculation
* Revise Q calculation
* Change Level LEDs to Q LEDs
* Resolve clipping problem
* Update UI text

1.0.1 
* Fix downsampling problem
* Remove compressor
* Update UI text

1.0.0
* First version

## Spectrum

1.2.0
* New scale structure for 48/96 mode
* Scales now have note information

1.1.0
* First version
