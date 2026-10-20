# Keyboard_Gesture_Recognition_Prototype

Build and open the live recognition window on macOS:

```sh
cmake -S . -B build
cmake --build build
./build/LCS_Gesture_Recognition LCS_gesture_data_5.txt
```

Type while the window is focused. After 0.3 seconds without another character,
the window shows the completed input and ranked gesture scores. The next character
starts a new gesture. Spaces and punctuation count as input; navigation keys and
Command/Control shortcuts do not.

To recognize a single input in the terminal, supply it as a second argument:

```sh
./build/LCS_Gesture_Recognition LCS_gesture_data_5.txt "qew32asdzxcvfgty67guyhjbl;kop[0-"
```
