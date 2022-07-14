##
# Maps true, True, TRUE, on, On, ON to 1
# and false, False, FALSE, off, Off, OFF to 0
#
# $input the value of a boolean type
# $output the value of int type
##
--let $int_value=$value
if ($value==on)
{
  --let $int_value=1
}

if ($value==On)
{
  --let $int_value=1
}

if ($value==ON)
{
  --let $int_value=1
}

if ($value==off)
{
  --let $int_value=0
}

if ($value==Off)
{
  --let $int_value=0
}

if ($value==OFF)
{
  --let $int_value=0
}

# MySQL allows 'true' and 'false' on bool values
if ($value==true)
{
  --let $int_value=1
}

if ($value==True)
{
  --let $int_value=1
}

if ($value==TRUE)
{
  --let $int_value=1
}

if ($value==false)
{
  --let $int_value=0
}

if ($value==False)
{
  --let $int_value=0
}

if ($value==FALSE)
{
  --let $int_value=0
}
