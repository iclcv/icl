I1=
I2=
INPUT_STATE=0

USE_ARGS=

function usage() {
    echo "usage:"
    echo "   icl-create-video [INPUT] [OUTPUT] <-s output-size> <-force> <-w output-width=1280> <-c crop-rect|interactive> <-f format=DIV3> <-fps fps=24> <ARGS ...>"
    echo "   INPUT is either a file pattern (in single tics to avoid evaluation) or a directory name"
    echo "   OUTPUT is an output video file name, e.g. video.avi"
    echo "   -s gives defined output-size (like 640x480 or VGA)"
    echo "   -force forces overwriting the output file"
    echo "   -w defines output width only (estimates the optimal height to keep the aspect ratio)"
    echo "   -f gives output video format (DIVX is mpeg 4, MP42 is mpeg 4.2, DIV3 "
    echo "      is mpeg 4.3 (default), and U263 is H236 codec, H264 (not always supported), LAGS lagarith lossless codec (?))" 
    
    exit -1;
}

for A in $@ ; do 
    if [ "$A" = "--help" ] || [ "$A" = "-help" ] || [ "$A" = "-h" ] ; then
        usage ;
    fi
done

INPUT=$1
OUTPUT=$2

if [ "$INPUT" = "" ] ; then
    echo "Error no input given!" ;
    usage ;
fi

if [ "$OUTPUT" = "" ] ; then
    echo "Error: no output given!" ;
    usage ;
fi

if [ -e $OUTPUT ] ; then
    if [ -d $OUTPUT ] ; then
        echo "Error: output is a directory (this will not even be overwritten if you use -force" ;
        usage ;
    fi
    HAVE_FORCE=FALSE
    for A in $@ ; do 
        if [ "$A" = "-force" ] ; then
            HAVE_FORCE=TRUE ;
        fi
    done
    if [ "$HAVE_FORCE" = "TRUE" ] ; then
        rm $OUTPUT ;
    fi
   
fi
NEXT_S=FALSE
NEXT_W=FALSE
NEXT_F=FALSE
NEXT_FPS=FALSE
S=
W=1280
F=DIV3
FPS=24
CROP=

for A in $@ ; do 
    if [ "$A" = "-s" ] ; then
        NEXT_S=TRUE
    elif [ "$NEXT_S" = "TRUE" ] ; then
        S=$A ;
        NEXT_S=FALSE ;
    fi

    if [ "$A" = "-w" ] ; then
        NEXT_W=TRUE
    elif [ "$NEXT_W" = "TRUE" ] ; then
        W=$A ;
        NEXT_W=FALSE ;
    fi

    if [ "$A" = "-f" ] ; then
        NEXT_F=TRUE
    elif [ "$NEXT_F" = "TRUE" ] ; then
        F=$A ;
        NEXT_F=FALSE ;
    fi

    if [ "$A" = "-fps" ] ; then
        NEXT_FPS=TRUE
    elif [ "$NEXT_FPS" = "TRUE" ] ; then
        FPS=$A ;
        NEXT_FPS=FALSE ;
    fi
done


for A in $@ ; do 
    if [ "$A" = "-ic" ] || [ "$A" = "-interactive-crop" ] ; then
        CROP="-clip $(icl-crop -input file $INPUT -c)"
        FIX=" -compute-optimal-scaling-size-input-size $(echo $CROP | cut -d ')' -f 2)"
    fi
done


if [ "$S" = "" ] ; then
    S="$(icl-crop -input file $INPUT -compute-optimal-scaling-size $W $FIX)"
fi


CMD="icl-pipe -size $S -input file $INPUT@loop=0 $CROP -output video "$OUTPUT,$F,$S,$FPS" -fps 1000"
echo "COMMAND: $CMD"
$CMD


