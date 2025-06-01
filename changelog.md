## Version v1.10.0
- Added script to create a version.
- Added description of game mechanics to README.
- Game starts paused.
- Improve cell storage, converting the 2d array into a 1d array, for performance. Apparently it helps with allocation and caché locality.
- Buffer for displaying information was too short.
- Render only visible creatures.
- Camera does not move further than world limits.
- Prevent from zooming out too much.
- The usage of CREATURES_SIZE was messing up proportions and zoom.
- Changed calculation of coordinates for selection.
- Move camera by dragging with mouse.
- Left and right were inverted.
- Added paramterer for probability of mutation.
- The movement of the camera is proportional to the zoom.
## Version v1.11.0
- Compact the array of creatures after updating.
- Check for integer overflow before increasing energy.
- Add selection highlighting.
- Limit frame-rate to 60fps.
- Remove unused buffer.
- Error checking for when we can't find an unocuppied cell.
- Decouple redisplay from updating. More smoothness in animation.
- Limit the number of attempts to find an unoccupied cell.
- Mouse callbacks didn't handle coordinate transformation consistently.
- Remove redundant NULL check.
- Fixed typo in REPRODUCTION_ENERGY_THRESHOLD.
- Calculation of maxPopulation could overflow.
- Free memory when exiting.
- Check for memory allocation.
- reproduce() created selected as an array of 0; corruption!
- move() was indexing cells wrong.
- Remove duplicated log in changelog.
- Added script for creating versions.

## Version v1.12.0
- Added control of initial creatures with density proportional to world size.
- Zoom in and out positioning camera in mouse location.

