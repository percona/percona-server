make clean;
make all;

gcc -shared -fPIC -o libs2.so \
  base/objs/strtoint.o \
  base/objs/int128.o \
  base/objs/stringprintf.o \
  base/objs/logging.o \
  util/coding/objs/coder.o \
  util/coding/objs/varint.o \
  util/math/objs/mathlimits.o \
  util/math/objs/exactfloat.o \
  util/math/objs/mathutil.o \
  strings/objs/split.o \
  strings/objs/ascii_ctype.o \
  strings/objs/strutil.o \
  strings/objs/stringprintf2.o \
  objs/s1angle.o \
  objs/s2.o \
  objs/s2cellid.o \
  objs/s2latlng.o \
  objs/s1interval.o \
  objs/s2cap.o \
  objs/s2cell.o \
  objs/s2cellunion.o \
  objs/s2edgeindex.o \
  objs/s2edgeutil.o \
  objs/s2latlngrect.o \
  objs/s2loop.o \
  objs/s2pointregion.o \
  objs/s2polygon.o \
  objs/s2polygonbuilder.o \
  objs/s2polyline.o \
  objs/s2r2rect.o \
  objs/s2region.o \
  objs/s2regioncoverer.o \
  objs/s2regionintersection.o \
  objs/s2regionunion.o
