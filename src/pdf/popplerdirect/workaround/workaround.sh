#/bin/bash

# Build a static version of libpoppler
# to be included in Xournal++
# 
# The library and the header files are symlinked
# for later use

poppler_version="0.24.1"
scriptname=`basename $0`

abs_top_srcdir="/home/andreas/dev/xournalpp"
abs_top_builddir="/home/andreas/dev/xournalpp"
enable_libopenjpeg="@enable_libopenjpeg@"

poppler_srcdir="${abs_top_srcdir}/src/pdf/popplerdirect/poppler-${poppler_version}"
poppler_dir="${abs_top_builddir}/src/pdf/popplerdirect"
poppler_builddir="${poppler_dir}/poppler-${poppler_version}_build"

create_link()
{
  sourcedir=$1
  targetdir=$2
  filename=$3

  if [ ! -e "${sourcedir}/${filename}" ]; then
    echo "Error in ${scriptname}: Can't create symbolic link:"
    echo "${sourcedir}/${filename} does not exist"
    echo
    exit 1
  fi

  if [ ! -e "${targetdir}/${filename}" ]; then
    rm -f "${targetdir}/${filename}"
    ln -s "${sourcedir}/${filename}" "${targetdir}/${filename}"
  fi
}

build()
{
  cd "${poppler_dir}"

  if [ -e "${poppler_dir}/libpoppler.a" ] && \
     [ -e "${poppler_dir}/libpoppler-glib.a" ] && \
     [ -e "${poppler_dir}/poppler-config.h" ]
  then
    echo "No need to rebuild libpoppler, exiting..."
    exit 0
  else
    echo "Rebuilding libpoppler..."
  fi

  autoreconf -i "${poppler_srcdir}"

  mkdir -p "${poppler_builddir}"

  cd "${poppler_builddir}"

  "${poppler_srcdir}/configure" --enable-cms --enable-libopenjpeg="${enable_libopenjpeg}" --enable-libjpeg

  make

  create_link "${poppler_builddir}/poppler/.libs" "${poppler_dir}" "libpoppler.a"
  create_link "${poppler_builddir}/glib/.libs" "${poppler_dir}" "libpoppler-glib.a"
  create_link "${poppler_builddir}/poppler" "${poppler_dir}" "poppler-config.h"
}

clean()
{
  echo "Cleaning libpoppler build..."
  
  for file in libpoppler.a libpoppler-glib.a poppler-config.h; do
    fullname="${poppler_dir}/${file}"
    [ -f "${fullname}" ] && rm "${fullname}"
  done
  
  
  [ -d "${poppler_builddir}" ] && rm -r "${poppler_builddir}"
}

case "$1" in

  "build")
    build
    ;;
  
  "clean")
    clean
    ;;
  
  *)
    echo "Unknown argument: " "$1"
    exit 1
  
esac
