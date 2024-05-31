
SCRIPT=$(realpath "$0")
SCRIPTPATH=$(dirname "$SCRIPT")

rm -f xboard.debug

xboard -debug -debugfile xboard.debug -cp -fcp "$SCRIPTPATH/zoe" -fd "$SCRIPTPATH"

exit 0

xboard -cp -fcp "belofte-0.9.10" -fd "/home/roland/Applications/moteurs/belofte" -scp "belofte-0.9.12" -sd "/home/roland/Applications/moteurs/belofte" /mg 2 /tc 5 /inc 3
xboard -cp -fcp "gnuchess-5.50-64" -fd "/home/roland/Applications/moteurs/gnuchess" -scp "crafty-236-64-ja" -sd "/home/roland/Applications/moteurs/crafty/236-ja/Linux" /mg 2 /tc 5 /inc 3
xboard -cp -fcp "/home/roland/Applications/moteurs/gnuchess/gnuchess-5.50-64" -fd "" -scp "/home/roland/Applications/moteurs/crafty/236-ja/Linux/crafty-236-64-ja" -sd "" /mg 2 /tc 5 /inc 3
xboard -cp -fcp "/home/roland/Applications/moteurs/belofte/belofte-0.9.10" -fd "" -scp "/home/roland/Applications/moteurs/belofte/belofte-0.9.12" -sd "" /mg 2 /tc 1 /inc 1
xboard -cp -fcp "/home/roland/Documents/echecs/sources/zoe/zoe" -fd "/home/roland/Documents/echecs/sources/zoe" -firstProtocolVersion 1
