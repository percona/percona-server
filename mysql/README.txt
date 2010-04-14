If you use without builtin InnoDB,
use 'series_for_xtradb' instead of 'series'.

example to patching
> cd mysql-x.x.x
> (cd [thisdir]; cat `cat series`) | patch -p1

