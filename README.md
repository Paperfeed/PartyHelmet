## PartyHelmet
Arduino Sketch for a party helmet

![Helmet](/helmet.jpg)

Has multiple modes:
- NAZI - Randomly switch between all modes
- DIST - Distance mode - LEDs are colored based on the distance read by the ultrasound sensor (from red if someone is standing too close, to green if you have enough space around you)
- RAIN - Rainbow mode, LEDs cycle through all colors in a gradient
- GLIT - Glitter mode, same as above, but LED's will randomly 'sparkle'
- CONF - Confetti mode, 
- SINE - Sine mode, a 'ping-pong' or 'Knight Rider's KITT' effect
- BPM - Beats Per Minute mode, LEDs flash according to the most common BPM used in dance music (128bpm)
- JGGL - Juggle mode, sort of like SINE but with multiple 'balls' going both directions simultaneously

Make sure you connect everything correctly. You can see/change which pins connect where in the sketch.

### Requires:
- FastLED library
- 4Digit display + SevenSegment Library
- Ultrasound sensor
- Touch sensor (or other button)
- LED Strip
- Battery pack
- Helmet to mount it all on

