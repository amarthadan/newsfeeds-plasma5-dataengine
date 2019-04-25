# NewsFeeds

Plasma 5 DataEngine for fetching RSS, Atom and other news feeds.

## Installation

### Arch Linux
Package will be available soon...

### Source

#### Requirements
* [CMake 2.8.12+](https://cmake.org/)
* [Extra CMake Modules](https://api.kde.org/ecm/)
* [Qt 5.5+](https://www.qt.io/)
* [KDE Frameworks 5.21+](https://api.kde.org/frameworks/)

#### Compile and install
```bash
git clone https://github.com/Misenko/newsfeeds-plasma5-dataengine.git
cd newsnow-plasma5-applet
mkdir build
cd build
cmake .. \
    -DCMAKE_INSTALL_PREFIX=/usr \
    -DCMAKE_BUILD_TYPE=Release \
    -DKDE_INSTALL_LIBDIR=lib \
    -DKDE_INSTALL_USE_QT_SYS_PATHS=ON
make
sudo make install
```

## Contributing
1. Fork it ( https://github.com/Misenko/newsfeeds-plasma5-dataengine/fork )
2. Create your feature branch (`git checkout -b my-new-feature`)
3. Commit your changes (`git commit -am 'Add some feature'`)
4. Push to the branch (`git push origin my-new-feature`)
5. Create a new Pull Request
