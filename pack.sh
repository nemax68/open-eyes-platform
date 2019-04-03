mv *.deb debian/binary

mv *.dsc debian/sources
mv *.tar.xz debian/sources

rm -f *.changes

cd debian

rm binary/Packages.gz
dpkg-scanpackages binary /dev/null | gzip -9c > binary/Packages.gz

rm sources/Sources.gz
dpkg-scansources sources /dev/null | gzip -9c > sources/Sources.gz
