#dnf install pulseaudio-libs-devel argtable-devel libconfig-devel hidapi-devel libserialport-devel lua-devel libuv-devel libxdg-basedir-devel
Summary: A device manager for racing sims
Name: monocoque
Version: 0.0.5
Release: 1
License: GPL
Group: Applications/Sound
Source: https://github.com/monocoque
URL: https://spacefreak18.github.io/simapi
Distribution: Fedora Linux
Vendor: spacefreak18
Packager: Paul Jones <paul@spacefreak18.xyz>
Requires: pulseaudio-libs argtable libconfig hidapi libserialport libuv libxdg-basedir lua-libs

%description
A device manager for Racing sims

%prep
rm -rf $RPM_BUILD_DIR/monocoque
rm -rf $RPM_SOURCE_DIR/monocoque
cd $RPM_SOURCE_DIR
git clone https://github.com/spacefreak18/monocoque
cd monocoque
git submodule update --init --recursive
cd ..
cp -r $RPM_SOURCE_DIR/monocoque $RPM_BUILD_DIR/

%build
cd $RPM_BUILD_DIR/monocoque
cmake -B build
cd build
make

%install
mkdir -p $RPM_BUILD_ROOT/usr/bin
cp $RPM_BUILD_DIR/monocoque/build/monocoque $RPM_BUILD_ROOT/usr/bin/monocoque

%files
/usr/bin/monocoque
