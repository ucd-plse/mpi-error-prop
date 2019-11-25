rm $1.ps
rm $1.pdf
rm all-$1.ps
rm all-$1.pdf
dot -Tps $1 > $1.ps
dot -Tps all-$1 > all-$1.ps
ps2pdf14 $1.ps
ps2pdf14 all-$1.ps
acroread $1.pdf all-$1.pdf
