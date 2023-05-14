# Ligtweight json and json-schema library for C

## Compile and install
```
make
sudo make install
```

## Test examples
```
cd test/simple
CFLAGS="-std=c11 -Wpedantic -Wall -Wextra -O2" LDLIBS="-ljson" make demo && ./demo
```

