########################################################################################################################
# Filename    # adacore/base.inc
# Purpose     # AdaCore base packages, GPRBuild, XMLAda, GNATColl, AWS.
# Description #
# Copyright   # Copyright (C) 2011-2017 Luke A. Guest, David Rees.
#             # All Rights Reserved.
########################################################################################################################

# $1 - Target triple
function gpr_bootstrap()
{
	local TASK_COUNT_TOTAL=1
 	VER="$build_type/$1"
	DIRS="$GPRBUILD_DIR-strap"
	LOGPRE=$LOG/$VER
	OBD=$BLD/$VER

    echo "  >> Creating Directories (if needed)..."

    cd $BLD
    for d in $DIRS; do
        if [ ! -d $VER/$d ]; then
            mkdir -p $VER/$d
        fi
    done

    cd $OBD/$GPRBUILD_DIR-strap

    if [ ! -f .gprbuild_strap ]; then
        echo "  >> [1/$TASK_COUNT_TOTAL] Building and installing GPRBuild bootstrap ($1)..."
        
        $SRC/$GPRBUILD_DIR/bootstrap.sh --srcdir=$SRC/$GPRBUILD_DIR --with-xmlada=$SRC/$XMLADA_DIR --prefix=$INSTALL_DIR &> $LOGPRE/$GPRBUILD_DIR-strap.txt

        check_error .gprbuild_strap
    fi

    echo "  >> GPRBuild bootstrap ($1) Installed"
}

# $1 - Host triple
# $2 - Build triple
# $3 - Target triple
function xmlada()
{
	local TASK_COUNT_TOTAL=4
 	VER="$build_type/$3"
	#DIRS="$XMLADA_DIR"
	LOGPRE=$LOG/$VER
	OBD=$BLD/$VER

    echo "  >> Creating Directories (if needed)..."

    cd $BLD
    if [ ! -d $VER/$XMLADA_DIR ]; then
        git clone $SRC/$XMLADA_DIR $VER/$XMLADA_DIR
    fi

    cd $OBD/$XMLADA_DIR

    if [ ! -f .config ]; then
        echo "  >> [1/$TASK_COUNT_TOTAL] Configuring XMLAda ($3)..."
        # Hack around the prefix as xmlada doesn't support DESTDIR.
        ./configure \
            --prefix=$STAGE_BASE_DIR$INSTALL_DIR \
            --target=$3 \
            --build=$2\
            --host=$1\
            &> $LOGPRE/$XMLADA_DIR-config.txt

        check_error .config
    fi

    if [ ! -f .make ]; then
        echo "  >> [2/$TASK_COUNT_TOTAL] Building XMLAda ($3)..."
        
        make all $JOBS &> $LOGPRE/$XMLADA_DIR-make.txt

        check_error .make
    fi

    if [ ! -f .make-pkg-stage ]; then
        echo "  >> [3/$TASK_COUNT_TOTAL] Packaging XMLAda ($3)..."
        
        make install &> $LOGPRE/$XMLADA_DIR-pkg.txt

        check_error .make-pkg-stage

        if [ ! -f .make-pkg ]; then
            cd $STAGE_DIR

            tar -cjpf $PKG/$PROJECT-$1_$2_$3-$XMLADA_DIR.tbz2 .

            check_error $OBD/$XMLADA_DIR/.make-pkg

            cd $OBD/$XMLADA_DIR
            rm -rf /tmp/opt
        fi
    fi

    if [ ! -f .make-install ]; then
        echo "  >> [4/$TASK_COUNT_TOTAL] Installing XMLAda (Native)..."

        tar -xjpf $PKG/$PROJECT-$1_$2_$3-$XMLADA_DIR.tbz2 -C $INSTALL_BASE_DIR

        check_error .make-install
    fi

    echo "  >> XMLAda (Native) Installed"
}

# $1 - Host triple
# $2 - Build triple
# $3 - Target triple
function gprbuild()
{
	local TASK_COUNT_TOTAL=4
 	VER="$build_type/$3"
	DIRS="$GPRBUILD_DIR"
	LOGPRE=$LOG/$VER
	OBD=$BLD/$VER

    echo "  >> Creating Directories (if needed)..."

    cd $BLD
    for d in $DIRS; do
        if [ ! -d $VER/$d ]; then
            mkdir -p $VER/$d
        fi
    done

    cd $OBD/$GPRBUILD_DIR

    MAKEFILE=$SRC/$GPRBUILD_DIR/Makefile
    
    if [ ! -f .config ]; then
        echo "  >> [1/$TASK_COUNT_TOTAL] Configuring GPRBuild ($3)..."

        make \
            prefix=$STAGE_BASE_DIR$INSTALL_DIR \
            SOURCE_DIR=$SRC/$GPRBUILD_DIR \
            PROCESSORS=$JOBS_NUM \
            LIBRARY_TYPE=relocatable \
            TARGET=$3 \
            -f $MAKEFILE \
            setup &> $LOGPRE/$GPRBUILD_DIR-config.txt

        check_error .config
    fi

    if [ ! -f .make ]; then
        echo "  >> [2/$TASK_COUNT_TOTAL] Building GPRBuild ($3)..."
        
        make LIBRARY_TYPE=relocatable -f $MAKEFILE all libgpr.build &> $LOGPRE/$GPRBUILD_DIR-make.txt

        check_error .make
    fi

    if [ ! -f .make-pkg-stage ]; then
        echo "  >> [3/$TASK_COUNT_TOTAL] Packaging GPRBuild ($3)..."
        
        make -f $MAKEFILE install libgpr.install &> $LOGPRE/$GPRBUILD_DIR-pkg.txt
        
        rm $STAGE_BASE_DIR$INSTALL_DIR/doinstall

        check_error .make-pkg-stage

        if [ ! -f .make-pkg ]; then
            cd $STAGE_DIR

            tar -cjpf $PKG/$PROJECT-$1_$2_$3-$GPRBUILD_DIR.tbz2 .

            check_error $OBD/$GPRBUILD_DIR/.make-pkg

            cd $OBD/$GPRBUILD_DIR
            rm -rf /tmp/opt
        fi
    fi

    if [ ! -f .make-install ]; then
        echo "  >> [4/$TASK_COUNT_TOTAL] Installing GPRBuild ($3)..."
        
        tar -xjpf $PKG/$PROJECT-$1_$2_$3-$GPRBUILD_DIR.tbz2 -C $INSTALL_BASE_DIR
        
        check_error .make-install
    fi

    echo "  >> GPRBuild bootstrap ($3) Installed"
}

# $1 - Host triple
# $2 - Build triple
# $3 - Target triple
function gnatcoll()
{
	local TASK_COUNT_TOTAL=5
 	VER="$build_type/$3"
	DIRS="$GNATCOLL_DIR"
	LOGPRE=$LOG/$VER
	OBD=$BLD/$VER

    # TODO: Another broken configure script requires cloning this into the build dir.
    
    echo "  >> Creating Directories (if needed)..."

    cd $BLD
    for d in $DIRS; do
        if [ ! -d $VER/$d ]; then
            mkdir -p $VER/$d
        fi
    done

    cd $OBD
    
#    if [ ! -f .gnatcoll-cloned ]; then
#        echo "  >> [1/$TASK_COUNT_TOTAL] Cloning GNATColl due to broken configure script ($3)..."
#
#        git clone $SRC/$GNATCOLL_DIR
#
#        check_error .gnatcoll-cloned 
#    fi
    
    if [ ! -f .gnatcoll-copied ]; then
        echo "  >> [1/$TASK_COUNT_TOTAL] Copying GNATColl due to broken configure script ($3)..."

        cp -Ra $SRC/$GNATCOLL_DIR .

        check_error .gnatcoll-copied
    fi

    cd $OBD/$GNATCOLL_DIR

    if [ ! -f .config ]; then
        echo "  >> [2/$TASK_COUNT_TOTAL] Configuring GNATColl ($3)..."

        ./configure \
            --prefix=$STAGE_BASE_DIR$INSTALL_DIR \
            --with-python=$INSTALL_DIR/bin --with-python-exec=python2.7 \
            &> $LOGPRE/$GNATCOLL_DIR-config.txt

        check_error .config
    fi

    if [ ! -f .make ]; then
        echo "  >> [3/$TASK_COUNT_TOTAL] Building GNATColl ($3)..."

        make $JOBS &> $LOGPRE/$GNATCOLL_DIR-make.txt

        check_error .make
    fi

    if [ ! -f .make-pkg-stage ]; then
        echo "  >> [4/$TASK_COUNT_TOTAL] Packaging GNATColl ($3)..."

        make  install &> $LOGPRE/$GNATCOLL_DIR-pkg.txt

        check_error .make-pkg-stage

        if [ ! -f .make-pkg ]; then
            cd $STAGE_DIR

            tar -cjpf $PKG/$PROJECT-$1_$2_$3-$GNATCOLL_DIR.tbz2 .

            check_error $OBD/$GNATCOLL_DIR/.make-pkg

            cd $OBD/$GNATCOLL_DIR
            rm -rf /tmp/opt
        fi
    fi

    if [ ! -f .make-install ]; then
        echo "  >> [5/$TASK_COUNT_TOTAL] Installing GNATColl ($3)..."

        tar -xjpf $PKG/$PROJECT-$1_$2_$3-$GNATCOLL_DIR.tbz2 -C $INSTALL_BASE_DIR

        check_error .make-install
    fi

    echo "  >> GNATColl ($3) Installed"
}
