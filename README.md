# JSON Prettify && Colorify
A simple c program without dependencies to indent and color the json you throw at it via an unix terminal emulator.
# Usage example:
```bash
./jsonpc "$(< example.json )" -c --indent=4 | less -R
./jsonpc "$(< example.json )" --colors -i4 --labels-color='#ff0000'
./jsonpc -h

```

# Building:
On a system with the cc binary and a working /bin/sh shell just run:
`./build.sh`
to get the jsonpc binary that you can copy to anywhere you want.
