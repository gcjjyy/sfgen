# sfgen: SpriteFont Generator

A Korean Sprite Font Generator for Construct 3.  
Using old-dos style Korean Combination 8x4x4 font(11520 bytes) and 256 english font(4096 bytes).  
The result png file contains 96 English characters and 2350 Korean characters(KSC5601) + 19 Korean Jamo characters.

  - PNG file is generated(Default filename: result.png)
  - Spacing Data file is generated(Filename: Spacing.txt)

# Prerequsites
Mac OS
```sh
$ brew install libpng
```

Ubuntu
```sh
$ sudo apt install libpng-dev
```

# Build
```sh
$ make
```

# Sample result image
![Preview](sfgen.png)

# License
Distributed under MIT license. See LICENSE file.
