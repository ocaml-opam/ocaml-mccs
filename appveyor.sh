#!/bin/bash

TERM=st

# Increment whenever the OCaml version or a package is updated to invalidate the caches
SERIAL=2

ROOT=C:/OCaml
ROOT_CYG=$(echo $ROOT| cygpath -f -)
APPVEYOR_BUILD_FOLDER=$(echo $APPVEYOR_BUILD_FOLDER| cygpath -f -)

ERRORS_ALLOWED=0
function quietly_log {
  if ! script --quiet --return --append --command "$1" $LOG_FILE > /dev/null 2>&1 ; then
    cat $LOG_FILE
    if ((ERRORS_ALLOWED)) ; then
      return 1
    else
      exit 1
    fi
  fi
}

function msvs_promote_path {
  if [[ ${1%64} = "msvc" ]] ; then
    eval $($ROOT_CYG/msvs-promote-path)
  fi
}

if ! cat $APPVEYOR_BUILD_FOLDER/appveyor.yml | tr -d '\015' | sed -e '1,/^cache:/d' -e '/^$/,$d' | grep -q "^ \+- \+C:\\\\OCaml$" ; then
  echo "$(tput setf 4)ERROR$(tput sgr0) C:\\OCaml doesn't appear to be cached in appveyor.yml"
  exit 1
fi

if [[ ! -e $ROOT_CYG/$OCAML_VERSION/$PORT/bin/ocamlopt.exe || ! -e $ROOT_CYG/$OCAML_VERSION/version || $(cat $ROOT_CYG/$OCAML_VERSION/version) != "$OCAML_VERSION-$SERIAL" ]] ; then
  if [[ -e $ROOT_CYG/$OCAML_VERSION/version && $(cat $ROOT_CYG/$OCAML_VERSION/version) != "$OCAML_VERSION-$SERIAL" ]] ; then
    echo "Build cache for $OCAML_VERSION has serial $(cat $ROOT_CYG/$OCAML_VERSION/version); should be $OCAML_VERSION-$SERIAL -- clearing"
    rm -rf $ROOT_CYG/$OCAML_VERSION
  elif [[ ! -e $ROOT_CYG/$OCAML_VERSION/version ]] ; then
    rm -rf $ROOT_CYG/$OCAML_VERSION
  fi

  PREFIX=$ROOT_CYG/$OCAML_VERSION/$PORT
  ROOT=$ROOT/$OCAML_VERSION/$PORT
  OCAML_BRANCH=${OCAML_VERSION%.*}
  OCAML_BRANCH=${OCAML_BRANCH/.}

  if [[ ! -d $APPVEYOR_BUILD_FOLDER/../src ]] ; then
    mkdir -p $APPVEYOR_BUILD_FOLDER/../src
    cd $APPVEYOR_BUILD_FOLDER/../src
    git clone https://github.com/ocaml/ocaml.git
    cd ocaml
    mkdir -p $PREFIX
    cp tools/msvs-promote-path $ROOT_CYG/
    cd ..
    FLEXDLL_VER=0.37
    CPPO_VER=1.6.6
    FINDLIB_VER=1.8.0
    DUNE_VER=1.10.0
    OCAMLBUILD_VER=0.14.0
    # NB CUDF URL will also need updating
    CUDF_VER=0.9
    EXTLIB_VER=1.7.6
    CBC_VER=2.9.9
    appveyor DownloadFile "https://bintray.com/coin-or/download/download_file?file_path=Cbc-$CBC_VER-win32-msvc14.zip" -FileName Cbc-$CBC_VER-win32-msvc14.zip
    appveyor DownloadFile "https://github.com/alainfrisch/flexdll/releases/download/$FLEXDLL_VER/flexdll-bin-$FLEXDLL_VER.zip" -FileName flexdll-bin-$FLEXDLL_VER.zip
    appveyor DownloadFile "https://github.com/mjambon/cppo/archive/v$CPPO_VER.tar.gz" -FileName cppo-$CPPO_VER.tar.gz
    appveyor DownloadFile "http://download.camlcity.org/download/findlib-$FINDLIB_VER.tar.gz" -FileName findlib-$FINDLIB_VER.tar.gz
    appveyor DownloadFile "https://github.com/ocaml/dune/archive/$DUNE_VER.tar.gz" -FileName dune-$DUNE_VER.tar.gz
    appveyor DownloadFile "https://github.com/ocaml/ocamlbuild/archive/$OCAMLBUILD_VER.tar.gz" -FileName ocamlbuild-$OCAMLBUILD_VER.tar.gz
    appveyor DownloadFile "https://gforge.inria.fr/frs/download.php/file/36602/cudf-0.9.tar.gz" -FileName cudf-$CUDF_VER.tar.gz
    appveyor DownloadFile "https://github.com/ygrek/ocaml-extlib/releases/download/$EXTLIB_VER/extlib-$EXTLIB_VER.tar.gz" -FileName extlib-$EXTLIB_VER.tar.gz
    cp $APPVEYOR_BUILD_FOLDER/appveyor/*.patch $APPVEYOR_BUILD_FOLDER/../src/
    [[ -e $PREFIX/../version ]] || echo $OCAML_VERSION-$SERIAL> $PREFIX/../version
  fi

  export PATH=$PREFIX/bin:$PATH

  cd $APPVEYOR_BUILD_FOLDER/../src/ocaml
  git checkout $OCAML_VERSION
  git worktree add ../$OCAML_VERSION/$PORT/ocaml -b build-$OCAML_VERSION-$PORT
  if [[ $OCAML_BRANCH -ge 403 ]] ; then
    pushd ../$OCAML_VERSION/$PORT/ocaml > /dev/null
    git submodule update --init
    # PR#48 (not yet merged)
    cd flexdll
    git remote add -f dra27 https://github.com/dra27/flexdll.git
    git checkout linking-c++
    cd ..
    popd > /dev/null
  fi
  cd ../$OCAML_VERSION/$PORT/ocaml
  if [[ $OCAML_BRANCH -ge 406 ]] ; then
    cp config/s-nt.h byterun/caml/s.h
    cp config/m-nt.h byterun/caml/m.h
  else
    cp config/s-nt.h config/s.h
    cp config/m-nt.h config/m.h
  fi
  if [[ $OCAML_BRANCH -ge 405 ]] ; then
    POST_WORLD=flexlink.opt
  else
    POST_WORLD=
  fi
  if [[ $OCAML_BRANCH -lt 403 ]] ; then
    mkdir -p $PREFIX/bin
    pushd $PREFIX/bin > /dev/null
    case $PORT in
      msvc)
        MANIFEST=default.manifest;;
      msvc64)
        MANIFEST=default_amd64.manifest;;
      *)
        MANIFEST=;;
    esac
    unzip $APPVEYOR_BUILD_FOLDER/../src/flexdll-bin-$FLEXDLL_VER.zip flexdll_*$PORT.* flexdll.h flexlink.exe $MANIFEST
    popd > /dev/null
    PRE_WORLD=
  else
    PRE_WORLD=flexdll
  fi
  sed -e "s|PREFIX=[^\r]*|PREFIX=$ROOT|" config/Makefile.$PORT > config/Makefile
  msvs_promote_path $PORT
  cd ..
  tar -xzf $APPVEYOR_BUILD_FOLDER/../src/findlib-$FINDLIB_VER.tar.gz
  cd findlib-$FINDLIB_VER
  # Not yet upstreamed
  sed -i -e 's/\.a/$(LIB_SUFFIX)/g' src/findlib/Makefile
  cd ..
  tar -xzf $APPVEYOR_BUILD_FOLDER/../src/dune-$DUNE_VER.tar.gz
  if [[ $OCAML_BRANCH -ge 403 ]] ; then
    tar -xzf $APPVEYOR_BUILD_FOLDER/../src/ocamlbuild-$OCAMLBUILD_VER.tar.gz
  fi
  tar -xzf $APPVEYOR_BUILD_FOLDER/../src/cppo-$CPPO_VER.tar.gz
  tar -xzf $APPVEYOR_BUILD_FOLDER/../src/cudf-$CUDF_VER.tar.gz
  cd cudf-$CUDF_VER
  # Upstreamed; not merged
  patch -p1 -i ../../../cudf-0.9.patch
  cd ..
  tar -xzf $APPVEYOR_BUILD_FOLDER/../src/extlib-$EXTLIB_VER.tar.gz
  cd ocaml

  LOG_FILE=OCaml-$OCAML_VERSION-$PORT.log
  echo "Building OCaml $OCAML_VERSION for $PORT" | tee $LOG_FILE
  echo "Please see $LOG_FILE for further information"
  LOG_FILE="$APPVEYOR_BUILD_FOLDER/$LOG_FILE"
  quietly_log "make -f Makefile.nt $PRE_WORLD world.opt $POST_WORLD install"
  # Remove unnecessary executables to keep the build cache size down
  # These are removed here to ensure findlib doesn't configure itself
  # to use .opt commands
  if [[ $OCAML_BRANCH -ge 404 ]] ; then
    rm $PREFIX/bin/*.byte.exe  $PREFIX/bin/*.opt.exe
  else
    for i in $PREFIX/bin/*.opt.exe ; do
      rm ${i%.opt.exe}.exe
      mv $i ${i%.opt.exe}.exe
    done
  fi
  cd ../findlib-$FINDLIB_VER
  quietly_log "./configure && make all opt && make install"
  cd ../dune-$DUNE_VER
  quietly_log "ocaml bootstrap.ml && ./boot.exe --release && cp _boot/install/default/bin/dune.exe $PREFIX/bin/dune.exe"
  if [[ $OCAML_BRANCH -ge 403 ]] ; then
    cd ../ocamlbuild-$OCAMLBUILD_VER
    quietly_log "make -f configure.make all OCAMLBUILD_PREFIX=$ROOT OCAMLBUILD_BINDIR=$ROOT/bin OCAMLBUILD_LIBDIR=$(ocamlfind printconf path | cygpath -f - -m) OCAML_NATIVE=true OCAML_NATIVE_TOOLS=false && make all findlib-install"
    rm $PREFIX/bin/ocamlbuild.{byte,native}.exe
  fi
  cd ../cppo-$CPPO_VER
  quietly_log "dune build @install -p cppo && cp _build/install/default/bin/cppo.exe $PREFIX/bin/"
  cd ../extlib-$EXTLIB_VER
  quietly_log "make minimal=1 build install"
  cd ../cudf-$CUDF_VER
  quietly_log "make DOC= BINDIR=$PREFIX/bin all opt install"
  pushd $PREFIX > /dev/null
  unzip $APPVEYOR_BUILD_FOLDER/../src/Cbc-$CBC_VER-win32-msvc14.zip bin/cbc.exe
  chmod +x bin/cbc.exe
  popd > /dev/null
  # Remove unnecessary commands to keep the build cache size down
  rm $PREFIX/bin/{ocamlcp,ocamldebug,ocamldoc,ocamlmktop,ocamlobjinfo,ocamloptp,ocamlprof}.exe $PREFIX/lib/{expunge,extract_crc,objinfo_helper}.exe
  # Remove unnecessary files
  if [[ $OCAML_BRANCH -lt 405 && $OCAML_BRANCH -gt 402 ]] ; then
    rm $PREFIX/*.txt
  fi
  find $PREFIX -name \*.cmt\* | xargs rm
  find $PREFIX -name \*.ml\* | xargs rm
  rm -f $PREFIX/lib/compiler-libs/*.cmx* $PREFIX/lib/compiler-libs/*.{lib,a} $PREFIX/lib/compiler-libs/ocamloptcomp.cma
  echo "Complete"
  appveyor PushArtifact $(echo $LOG_FILE| cygpath -m -f -)
fi
